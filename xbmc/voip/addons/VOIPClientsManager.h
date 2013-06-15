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

#include "threads/CriticalSection.h"
#include "threads/Thread.h"
#include "utils/Observer.h"
#include "VOIPClient.h"

#include "addons/AddonDatabase.h"

#include <vector>
#include <deque>

namespace VOIP
{
  class CPVRGUIInfo;

  typedef std::map< int, boost::shared_ptr<CVOIPClient> >                 VOIP_CLIENTMAP;
  typedef std::map< int, boost::shared_ptr<CVOIPClient> >::iterator       VOIP_CLIENTMAP_ITR;
  typedef std::map< int, boost::shared_ptr<CVOIPClient> >::const_iterator VOIP_CLIENTMAP_CITR;

  typedef boost::shared_ptr<CVOIPClient> VOIP_CLIENT;

  class CVOIPClientsManager : public ADDON::IAddonMgrCallback,
							  public Observer,
							  private CThread
  {

  public:
    CVOIPClientsManager(void);
    virtual ~CVOIPClientsManager(void);

    /*!
     * @brief Start the backend info updater thread.
     */
    void Start(void);

    void Stop(void);

	void Unload(void);

	// CAddonMgr::Get().RegisterObserver(this);
    void Notify(const Observable &obs, const ObservableMessage msg);

	// CAddonMgr::Get().RegisterAddonMgrCallback(ADDON_VOIPDLL, this);
	bool RequestRestart(ADDON::AddonPtr addon, bool bDataChanged);
	bool RequestRemoval(ADDON::AddonPtr addon);


	bool StopClient(ADDON::AddonPtr client, bool bRestart);
	bool GetClient(const CStdString &strId, ADDON::AddonPtr &addon) const;
	bool IsInUse(const std::string& strAddonId) const;


  private:
    bool UpdateAddons(void);

    void Process(void);

	bool GetClient(int iClientId, boost::shared_ptr<CVOIPClient> &addon) const;
	bool GetConnectedClient(int iClientId, boost::shared_ptr<CVOIPClient> &addon) const;
	int GetConnectedClients(VOIP_CLIENTMAP &clients) const;
	int GetClientId(const ADDON::AddonPtr client) const;

    int RegisterClient(ADDON::AddonPtr client);
    
    ADDON::VECADDONS      m_addons;
    VOIP_CLIENTMAP        m_clientMap;
	CAddonDatabase        m_addonDb;

    CCriticalSection      m_critSection;
  };
}
