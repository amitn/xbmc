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
#include "AddonCallbacksVOIP.h"
#include "settings/AdvancedSettings.h"
#include "utils/log.h"


#include "voip/addons/VOIPClient.h"
#include "voip/VOIPManager.h"

using namespace VOIP;

namespace ADDON
{

CAddonCallbacksVOIP::CAddonCallbacksVOIP(CAddon* addon)
{
  m_addon     = addon;
  m_callbacks = new CB_VOIPLib;

  m_callbacks->AddMenuHook                = VOIPAddMenuHook;
}

CAddonCallbacksVOIP::~CAddonCallbacksVOIP()
{
  /* delete the callback table */
  delete m_callbacks;
}

CVOIPClient *CAddonCallbacksVOIP::GetVOIPClient(void *addonData)
{
  CAddonCallbacks *addon = static_cast<CAddonCallbacks *>(addonData);
  if (!addon || !addon->GetHelperVOIP())
  {
    CLog::Log(LOGERROR, "VOIP - %s - called with a null pointer", __FUNCTION__);
    return NULL;
  }

  return dynamic_cast<CVOIPClient *>(addon->GetHelperVOIP()->m_addon);
}

void CAddonCallbacksVOIP::VOIPAddMenuHook(void *addonData, VOIP_MENUHOOK *hook)
{
  CVOIPClient *client = GetVOIPClient(addonData);
  if (!hook || !client)
  {
    CLog::Log(LOGERROR, "VOIP - %s - invalid handler data", __FUNCTION__);
    return;
  }

  VOIP_MENUHOOKS *hooks = client->GetMenuHooks();
  if (hooks)
  {
    VOIP_MENUHOOK hookInt;
    hookInt.iHookId            = hook->iHookId;
    hookInt.iLocalizedStringId = hook->iLocalizedStringId;
    hookInt.category           = hook->category;

    /* add this new hook */
    hooks->push_back(hookInt);
  }
}


}; /* namespace ADDON */
