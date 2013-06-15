/*
 *      Copyright (C) 2005-2012 Team XBMC
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

#ifndef __XBMC_VOIP_H__
#define __XBMC_VOIP_H__

#include "xbmc_addon_dll.h"
#include "xbmc_voip_types.h"


extern "C"
{
  /*! @name PVR add-on methods */
  //@{
  /*!
   * Get the XBMC_PVR_API_VERSION that was used to compile this add-on.
   * Used to check if this add-on is compatible with XBMC.
   * @return The XBMC_PVR_API_VERSION that was used to compile this add-on.
   * @remarks Valid implementation required.
   */
  const char* GetVOIPAPIVersion(void);

  /*!
   * Get the XBMC_PVR_MIN_API_VERSION that was used to compile this add-on.
   * Used to check if this add-on is compatible with XBMC.
   * @return The XBMC_PVR_MIN_API_VERSION that was used to compile this add-on.
   * @remarks Valid implementation required.
   */
  const char* GetMininumVOIPAPIVersion(void);

  /*!
   * Get the list of features that this add-on provides.
   * Called by XBMC to query the add-on's capabilities.
   * Used to check which options should be presented in the UI, which methods to call, etc.
   * All capabilities that the add-on supports should be set to true.
   * @param pCapabilities The add-on's capabilities.
   * @return PVR_ERROR_NO_ERROR if the properties were fetched successfully.
   * @remarks Valid implementation required.
   */
  VOIP_ERROR GetAddonCapabilities(VOIP_ADDON_CAPABILITIES *pCapabilities);

  /*!
   * @return The name reported by the backend that will be displayed in the UI.
   * @remarks Valid implementation required.
   */
  const char* GetBackendName(void);

  /*!
   * @return The version string reported by the backend that will be displayed in the UI.
   * @remarks Valid implementation required.
   */
  const char* GetBackendVersion(void);

  /*!
   * Call one of the menu hooks (if supported).
   * Supported PVR_MENUHOOK instances have to be added in ADDON_Create(), by calling AddMenuHook() on the callback.
   * @param menuhook The hook to call.
   * @return PVR_ERROR_NO_ERROR if the hook was called successfully.
   * @remarks Optional. Return PVR_ERROR_NOT_IMPLEMENTED if this add-on won't provide this function.
   */
  VOIP_ERROR CallMenuHook(const VOIP_MENUHOOK& menuhook);
  //@}

  int GetNumberOfContacts(void);

  VOIP_CONTACT *GetContactById(int);

  VOIP_ERROR GetContact(VOIP_CONTACT&, int);
  /*!
   * Called by XBMC to assign the function pointers of this add-on to pClient.
   * @param pClient The struct to assign the function pointers to.
   */
  void __declspec(dllexport) get_addon(struct VOIPClient* pClient)
  {
    pClient->GetVOIPAPIVersion              = GetVOIPAPIVersion;
    pClient->GetMininumVOIPAPIVersion       = GetMininumVOIPAPIVersion;
    pClient->GetAddonCapabilities           = GetAddonCapabilities;

    pClient->GetBackendName                 = GetBackendName;
    pClient->GetBackendVersion              = GetBackendVersion;

	pClient->GetNumberOfContacts            = GetNumberOfContacts;
	pClient->GetContact                     = GetContact;
	pClient->GetContactById                 = GetContactById;
    pClient->MenuHook                       = CallMenuHook;

  };
};

#endif
