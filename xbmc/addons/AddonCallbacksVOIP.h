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

#include "AddonCallbacks.h"
#include "include/xbmc_voip_types.h"

namespace VOIP
{
  class CVOIPClient;
}

namespace ADDON
{

/*!
 * Callbacks for a PVR add-on to XBMC.
 *
 * Also translates the addon's C structures to XBMC's C++ structures.
 */
class CAddonCallbacksVOIP
{
public:
  CAddonCallbacksVOIP(CAddon* addon);
  ~CAddonCallbacksVOIP(void);

  /*!
   * @return The callback table.
   */
  CB_VOIPLib *GetCallbacks() { return m_callbacks; }

  static void VOIPAddMenuHook(void* addonData, VOIP_MENUHOOK* hook);

private:
  static VOIP::CVOIPClient* GetVOIPClient(void* addonData);

  CB_VOIPLib    *m_callbacks; /*!< callback addresses */
  CAddon        *m_addon;     /*!< the addon */
};

}; /* namespace ADDON */
