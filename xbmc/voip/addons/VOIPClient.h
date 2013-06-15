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

#include "addons/Addon.h"
#include "addons/AddonDll.h"
#include "addons/DllVOIPClient.h"

namespace VOIP
{
  class CVOIPClient;

  typedef std::vector<VOIP_MENUHOOK> VOIP_MENUHOOKS;
  typedef boost::shared_ptr<CVOIPClient> VOIP_CLIENT;
  #define VOIP_INVALID_CLIENT_ID (-2)
  #define VOIP_VIRTUAL_CLIENT_ID (-1)

  /*!
   * Interface from XBMC to a VOIP add-on.
   *
   * Also translates XBMC's C++ structures to the addon's C structures.
   */
  class CVOIPClient : public ADDON::CAddonDll<DllVOIPClient, VOIPClient, VOIP_PROPERTIES>
  {
  public:
    CVOIPClient(const ADDON::AddonProps& props);
    CVOIPClient(const cp_extension_t *ext);
    ~CVOIPClient(void);

    ADDON_STATUS Create(int iClientId);

    bool DllLoaded(void) const;

    void Destroy(void);

    void ReCreate(void);

	bool ReadyToUse(void) const;

    int GetID(void) const;

	bool GetAddonProperties(void);

	VOIP_ADDON_CAPABILITIES GetAddonCapabilities(void) const;

    CStdString GetBackendName(void) const;
	CStdString GetBackendVersion(void) const;
	CStdString GetFriendlyName(void) const;

    VOIP_MENUHOOKS *GetMenuHooks(void);
    void CallMenuHook(const VOIP_MENUHOOK &hook);

	int GetNumberOfContacts(void);
	void GetContactById(int);
    static const char *ToString(const VOIP_ERROR error);

  private:

	static bool IsCompatibleAPIVersion(const ADDON::AddonVersion &minVersion, const ADDON::AddonVersion &version);
    bool CheckAPIVersion(void);
    void ResetProperties(int iClientId = VOIP_INVALID_CLIENT_ID);

	bool LogError(const VOIP_ERROR error, const char *strMethod) const;
    void LogException(const std::exception &e, const char *strFunctionName) const;

    bool                   m_bReadyToUse;          /*!< true if this add-on is connected to the backend, false otherwise */
    CStdString             m_strHostName;          /*!< the host name */
    VOIP_MENUHOOKS         m_menuhooks;            /*!< the menu hooks for this add-on */
    int                    m_iClientId;            /*!< database ID of the client */


    /* stored strings to make sure const char* members in PVR_PROPERTIES stay valid */
    std::string m_strUserPath;    /*!< @brief translated path to the user profile */
    std::string m_strClientPath;  /*!< @brief translated path to this add-on */

    CCriticalSection m_critSection;

	VOIP_ADDON_CAPABILITIES    m_addonCapabilities;
	CStdString                 m_strBackendName;
    CStdString                 m_strConnectionString;
    CStdString                 m_strFriendlyName;
    CStdString                 m_strBackendVersion;
    ADDON::AddonVersion        m_apiVersion;
  };
}
