/*
 *      Copyright (C) 2012 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "Application.h"
#include "ApplicationMessenger.h"
#include "GUIInfoManager.h"
#include "dialogs/GUIDialogOK.h"
#include "dialogs/GUIDialogNumeric.h"
#include "dialogs/GUIDialogProgress.h"
#include "dialogs/GUIDialogExtendedProgressBar.h"
#include "dialogs/GUIDialogKaiToast.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/LocalizeStrings.h"
#include "music/tags/MusicInfoTag.h"
#include "settings/AdvancedSettings.h"
#include "settings/GUISettings.h"
#include "settings/Settings.h"
#include "threads/SingleLock.h"
#include "windows/GUIWindowPVR.h"
#include "utils/log.h"
#include "utils/Stopwatch.h"
#include "utils/StringUtils.h"
#include "threads/Atomics.h"
#include "windows/GUIWindowPVRCommon.h"
#include "utils/JobManager.h"

#include "VOIPManager.h"
#include "addons/VOIPClient.h"

#include "interfaces/AnnouncementManager.h"
#include "addons/AddonInstaller.h"

using namespace std;
using namespace VOIP;

CVOIPManager::CVOIPManager(void) :
    CThread("VOIP manager"),
    m_triggerEvent(true),
    m_managerState(ManagerStateStopped)
{
  ResetProperties();
}

CVOIPManager::~CVOIPManager(void)
{
  Stop();
  CLog::Log(LOGDEBUG,"VOIPManager - destroyed");
}

CVOIPManager &CVOIPManager::Get(void)
{
  static CVOIPManager voipManagerInstance;
  return voipManagerInstance;
}

bool CVOIPManager::InstallAddonAllowed(const std::string& strAddonId) const
{
  return !IsStarted();
}

void CVOIPManager::MarkAsOutdated(const std::string& strAddonId, const std::string& strReferer)
{
  if (IsStarted() && g_settings.m_bAddonAutoUpdate)
  {
    CSingleLock lock(m_critSection);
    m_outdatedAddons.insert(make_pair<string, string>(strAddonId, strReferer));
  }
}

bool CVOIPManager::UpgradeOutdatedAddons(void)
{
  CSingleLock lock(m_critSection);
  if (m_outdatedAddons.empty())
    return true;

  // there's add-ons that couldn't be updated
  for (map<string, string>::iterator it = m_outdatedAddons.begin(); it != m_outdatedAddons.end(); it++)
  {
    if (!InstallAddonAllowed(it->first))
    {
      // we can't upgrade right now
      return true;
    }
  }

  // all outdated add-ons can be upgraded now
  CLog::Log(LOGINFO, "VOIP - upgrading outdated add-ons");

  map<string, string> outdatedAddons = m_outdatedAddons;
  // stop threads and unload
  SetState(ManagerStateInterrupted);
  Cleanup();

  // upgrade all add-ons
  for (map<string, string>::iterator it = outdatedAddons.begin(); it != outdatedAddons.end(); it++)
  {
    CLog::Log(LOGINFO, "VOIP - updating add-on '%s'", it->first.c_str());
    CAddonInstaller::Get().Install(it->first, true, it->second, false);
  }

  // reload
  CLog::Log(LOGINFO, "VOIPManager - %s - restarting the VOIP manager", __FUNCTION__);
  SetState(ManagerStateStarting);
  ResetProperties();

  while (!Load() && GetState() == ManagerStateStarting)
  {
    CLog::Log(LOGERROR, "VOIPManager - %s - failed to load VOIP data, retrying", __FUNCTION__);
    Cleanup();
    Sleep(1000);
  }

  if (GetState() == ManagerStateStarting)
  {
    SetState(ManagerStateStarted);

    CLog::Log(LOGDEBUG, "VOIPManager - %s - restarted", __FUNCTION__);
    return true;
  }

  return false;
}

void CVOIPManager::Cleanup(void)
{
  CSingleLock lock(m_critSection);

  m_triggerEvent.Set();


  for (unsigned int iJobPtr = 0; iJobPtr < m_pendingUpdates.size(); iJobPtr++)
    delete m_pendingUpdates.at(iJobPtr);
  m_pendingUpdates.clear();

  if (m_clientsManager)
  {
	  m_clientsManager->Stop();
	  delete m_clientsManager;
  }

  m_initialisedEvent.Reset();
  SetState(ManagerStateStopped);
}

void CVOIPManager::ResetProperties(void)
{
  CSingleLock lock(m_critSection);
  Cleanup();
}

void CVOIPManager::Start(bool bAsync /* = false */, bool bOpenVOIPWindow /* = false */)
{
  CLog::Log(LOGERROR, "VOIP - %s ", __FUNCTION__);
  CSingleLock lock(m_critSection);

  /* first stop and remove any clients */
  Stop();

  /* don't start if Settings->Video->TV->Enable isn't checked */

  ResetProperties();
  SetState(ManagerStateStarting);

  m_clientsManager = new CVOIPClientsManager();
  m_clientsManager->Start();

}

void CVOIPManager::Stop(void)
{
  /* check whether the pvrmanager is loaded */
  if (GetState() == ManagerStateStopping ||
      GetState() == ManagerStateStopped)
    return;

  SetState(ManagerStateStopping);

  /* stop the EPG updater, since it might be using the pvr add-ons */
  m_initialisedEvent.Set();

  CLog::Log(LOGNOTICE, "VOIPManager - stopping");

  /* unload all data */
  Cleanup();
}

ManagerState CVOIPManager::GetState(void) const
{
  CSingleLock lock(m_managerStateMutex);
  return m_managerState;
}

void CVOIPManager::SetState(ManagerState state) 
{
  CSingleLock lock(m_managerStateMutex);
  m_managerState = state;
}

void CVOIPManager::Process(void)
{
  /* load the pvr data from the db and clients if it's not already loaded */
  while (!Load() && GetState() == ManagerStateStarting)
  {
    CLog::Log(LOGERROR, "VOIPManager - %s - failed to load VOIP data, retrying", __FUNCTION__);
    Cleanup();
    Sleep(1000);
  }

  if (GetState() == ManagerStateStarting)
    SetState(ManagerStateStarted);
  else
    return;

  /* main loop */
  CLog::Log(LOGDEBUG, "VOIPManager - %s - entering main loop", __FUNCTION__);
  m_initialisedEvent.Set();

  bool bRestart(false);
  
}

bool CVOIPManager::Load(void)
{
  
  CLog::Log(LOGDEBUG, "PVRManager - %s - active clients found. continue to start", __FUNCTION__);

  return true;
}

bool CVOIPManager::IsStarted(void) const
{
	return true;
}