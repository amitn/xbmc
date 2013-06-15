#pragma once
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

#include "threads/Thread.h"
#include "utils/JobManager.h"
#include "threads/Event.h"
#include "addons/include/xbmc_voip_types.h"
#include "addons/VOIPClientsManager.h"
#include <map>


namespace VOIP
{
  class CVOIPClient;
  
  enum ManagerState
  {
    ManagerStateError = 0,
    ManagerStateStopped,
    ManagerStateStarting,
    ManagerStateStopping,
    ManagerStateInterrupted,
    ManagerStateStarted
  };

  #define g_VOIPManager       CVOIPManager::Get()

  class CVOIPManager : private CThread
  {
    friend class CVOIPClients;

  private:
    /*!
     * @brief Create a new CPVRManager instance, which handles all PVR related operations in XBMC.
     */
    CVOIPManager(void);

  public:
    /*!
     * @brief Stop the PVRManager and destroy all objects it created.
     */
    virtual ~CVOIPManager(void);

    /*!
     * @brief Get the instance of the PVRManager.
     * @return The PVRManager instance.
     */
    static CVOIPManager &Get(void);

    /*!
     * @brief Get the channel groups container.
     * @return The groups container.
     */
    
    void Start(bool bAsync = false, bool bOpenVOIPWindow = false);

    /*!
     * @brief Stop the PVRManager and destroy all objects it created.
     */
    void Stop(void);

    /*!
     * @brief Delete PVRManager's objects.
     */
    void Cleanup(void);

    /*!
     * @brief Check whether an add-on can be upgraded or installed without restarting the pvr manager, when the add-on is in use or the pvr window is active
     * @param strAddonId The add-on to check.
     * @return True when the add-on can be installed, false otherwise.
     */
    bool InstallAddonAllowed(const std::string& strAddonId) const;

    /*!
     * @brief Mark an add-on as outdated so it will be upgrade when it's possible again
     * @param strAddonId The add-on to mark as outdated
     * @param strReferer The referer to use when downloading
     */
    void MarkAsOutdated(const std::string& strAddonId, const std::string& strReferer);

    /*!
     * @return True when updated, false when the pvr manager failed to load after the attempt
     */
    bool UpgradeOutdatedAddons(void);

    bool IsStarted(void) const;

  protected:
    /*!
     * @brief PVR update and control thread.
     */
    virtual void Process(void);

  private:
    /*!
     * @brief Load at least one client and load all other PVR data after loading the client.
     * If some clients failed to load here, the pvrmanager will retry to load them every second.
     * @return If at least one client and all pvr data was loaded, false otherwise.
     */
    bool Load(void);
    void ResetProperties(void);
    ManagerState GetState(void) const;

	CVOIPClientsManager            *m_clientsManager;

    void SetState(ManagerState state);
    CCriticalSection                m_critSectionTriggers;         /*!< critical section for triggered updates */
    CEvent                          m_triggerEvent;                /*!< triggers an update */
    std::vector<CJob *>             m_pendingUpdates;              /*!< vector of pending pvr updates */
    CCriticalSection                m_critSection;                 /*!< critical section for all changes to this class, except for changes to triggers */

	CCriticalSection                m_managerStateMutex;
    ManagerState                    m_managerState;
    std::map<std::string, std::string> m_outdatedAddons;
    CEvent                             m_initialisedEvent;         /*!< triggered when the pvr manager initialised */
  };

}
