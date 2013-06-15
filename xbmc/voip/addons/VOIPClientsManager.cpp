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

#include "VOIPClientsManager.h"

#include "Application.h"
#include "ApplicationMessenger.h"
#include "GUIUserMessages.h"
#include "settings/GUISettings.h"
#include "settings/Settings.h"

using namespace std;
using namespace ADDON;
using namespace VOIP;

CVOIPClientsManager::CVOIPClientsManager(void) :
    CThread("VOIP add-on updater")
{
}

CVOIPClientsManager::~CVOIPClientsManager(void)
{
  Unload();
}

bool CVOIPClientsManager::IsInUse(const std::string& strAddonId) const
{
  CSingleLock lock(m_critSection);

  for (VOIP_CLIENTMAP_CITR itr = m_clientMap.begin(); itr != m_clientMap.end(); itr++)
    if (itr->second->Enabled() && itr->second->ID().Equals(strAddonId.c_str()))
      return true;
  return false;
}

void CVOIPClientsManager::Start(void)
{
  CLog::Log(LOGERROR, "VOIP - %s ", __FUNCTION__);
  Stop();

  m_addonDb.Open();
  Create();
  SetPriority(-1);
}

void CVOIPClientsManager::Stop(void)
{
  StopThread();
  m_addonDb.Close();
}


int CVOIPClientsManager::GetClientId(const AddonPtr client) const
{
  CSingleLock lock(m_critSection);

  for (VOIP_CLIENTMAP_CITR itr = m_clientMap.begin(); itr != m_clientMap.end(); itr++)
    if (itr->second->ID() == client->ID())
      return itr->first;

  return -1;
}

bool CVOIPClientsManager::GetClient(int iClientId, VOIP_CLIENT &addon) const
{
  bool bReturn(false);
  if (iClientId <= VOIP_INVALID_CLIENT_ID || iClientId == VOIP_VIRTUAL_CLIENT_ID)
    return bReturn;

  CSingleLock lock(m_critSection);

  VOIP_CLIENTMAP_CITR itr = m_clientMap.find(iClientId);
  if (itr != m_clientMap.end())
  {
    addon = itr->second;
    bReturn = true;
  }

  return bReturn;
}


bool CVOIPClientsManager::RequestRestart(AddonPtr addon, bool bDataChanged)
{
  return StopClient(addon, true);
}

bool CVOIPClientsManager::RequestRemoval(AddonPtr addon)
{
  return StopClient(addon, false);
}

void CVOIPClientsManager::Unload(void)
{
  Stop();

  CSingleLock lock(m_critSection);

  /* destroy all clients */
  for (VOIP_CLIENTMAP_ITR itr = m_clientMap.begin(); itr != m_clientMap.end(); itr++)
    itr->second->Destroy();

  /* reset class properties */

  m_clientMap.clear();
}

bool CVOIPClientsManager::StopClient(AddonPtr client, bool bRestart)
{
  CSingleLock lock(m_critSection);  
  int iId = GetClientId(client);
  VOIP_CLIENT mappedClient;
  if (GetConnectedClient(iId, mappedClient))
  {
    if (bRestart)
      mappedClient->ReCreate();
    else
      mappedClient->Destroy();

    return true;
  }

  return false;
}

bool CVOIPClientsManager::GetConnectedClient(int iClientId, VOIP_CLIENT &addon) const
{
  if (GetClient(iClientId, addon))
    return addon->ReadyToUse();
  return false;
}

int CVOIPClientsManager::GetConnectedClients(VOIP_CLIENTMAP &clients) const
{
  int iReturn(0);
  CSingleLock lock(m_critSection);

  for (VOIP_CLIENTMAP_CITR itr = m_clientMap.begin(); itr != m_clientMap.end(); itr++)
  {
    if (itr->second->ReadyToUse())
    {
      clients.insert(std::make_pair(itr->second->GetID(), itr->second));
      ++iReturn;
    }
  }

  return iReturn;
}


int CVOIPClientsManager::RegisterClient(AddonPtr client)
{
  CLog::Log(LOGDEBUG, "%s - registering add-on '%s'", __FUNCTION__, client->Name().c_str());
  int iClientId(-1);
  if (!client->Enabled())
    return -1;

  CLog::Log(LOGDEBUG, "%s - registering add-on '%s'", __FUNCTION__, client->Name().c_str());

  VOIP_CLIENT addon;
  // load and initialise the client libraries
  {
    CSingleLock lock(m_critSection);
    VOIP_CLIENTMAP_ITR existingClient = m_clientMap.find(iClientId);
    if (existingClient != m_clientMap.end())
    {
      // return existing client
      addon = existingClient->second;
    }
    else
    {
      // create a new client instance
      addon = boost::dynamic_pointer_cast<CVOIPClient>(client);
      m_clientMap.insert(std::make_pair(iClientId, addon));
    }
  }

  if (iClientId < 0)
    CLog::Log(LOGERROR, "VOIP - %s - can't register add-on '%s'", __FUNCTION__, client->Name().c_str());

  return iClientId;
}

void CVOIPClientsManager::Process(void)
{
  CLog::Log(LOGERROR, "VOIP - %s ", __FUNCTION__);
  bool bCheckedEnabledClientsOnStartup(false);
  

  CAddonMgr::Get().RegisterAddonMgrCallback(ADDON_VOIPDLL, this);
  CAddonMgr::Get().RegisterObserver(this);

  UpdateAddons();

  while (!g_application.m_bStop && !m_bStop)
  {
    Sleep(1000);
  }

}

bool CVOIPClientsManager::UpdateAddons(void)
{
  CLog::Log(LOGERROR, "VOIP - %s ", __FUNCTION__);
  ADDON::VECADDONS addons;
  bool bReturn(CAddonMgr::Get().GetAddons(ADDON_VOIPDLL, addons, true));
  CLog::Log(LOGERROR, "VOIP - %s Number of Addons %d", __FUNCTION__, addons.size() );
  if (bReturn)
  {
    CSingleLock lock(m_critSection);
    m_addons = addons;
  }
  
  // handle "new" addons which aren't yet in the db - these have to be added first
  for (unsigned iClientPtr = 0; iClientPtr < m_addons.size(); iClientPtr++)
  {
    const AddonPtr clientAddon = m_addons.at(iClientPtr);
	CLog::Log(LOGERROR, "VOIP - %s Number of Addons %d name %s", __FUNCTION__, iClientPtr,  clientAddon->Name().c_str()  );
    if (!m_addonDb.HasAddon(clientAddon->ID()))
    {
      m_addonDb.AddAddon(clientAddon, -1);
    }

	if ( clientAddon->Enabled() ) {
		VOIP_CLIENT addon = boost::dynamic_pointer_cast<CVOIPClient>(clientAddon);
		addon->Create(iClientPtr+1);
		addon->DllLoaded();
		CLog::Log(LOGERROR, "VOIP - %s Number of Addons %d name %s Number of C: %d ", __FUNCTION__, iClientPtr,  clientAddon->Name().c_str(), addon->GetNumberOfContacts() );
		addon->GetContactById(1);
	}
  }

  return bReturn;
}

void CVOIPClientsManager::Notify(const Observable &obs, const ObservableMessage msg)
{
  if (msg == ObservableMessageAddons)
    UpdateAddons();
}

bool CVOIPClientsManager::GetClient(const CStdString &strId, ADDON::AddonPtr &addon) const
{
  CSingleLock lock(m_critSection);
  for (VOIP_CLIENTMAP_CITR itr = m_clientMap.begin(); itr != m_clientMap.end(); itr++)
  {
    if (itr->second->ID() == strId)
    {
      addon = itr->second;
      return true;
    }
  }
  return false;
}
