#pragma once
/*
 *      Copyright (C) 2005-2010 Team XBMC
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

#include <string>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "xbmc_voip_types.h"
#include "../library.xbmc.addon/libXBMC_addon.h"

#ifdef _WIN32
#define VOIP_HELPER_DLL "\\library.xbmc.voip\\libXBMC_voip" ADDON_HELPER_EXT
#else
#define VOIP_HELPER_DLL_NAME "libXBMC_voip-" ADDON_HELPER_ARCH ADDON_HELPER_EXT
#define VOIP_HELPER_DLL "/library.xbmc.voip/" VOIP_HELPER_DLL_NAME
#endif

class CHelper_libXBMC_voip
{
public:
  CHelper_libXBMC_voip(void)
  {
    m_libXBMC_voip = NULL;
    m_Handle       = NULL;
  }

  ~CHelper_libXBMC_voip(void)
  {
    if (m_libXBMC_voip)
    {
      VOIP_unregister_me(m_Handle, m_Callbacks);
      dlclose(m_libXBMC_voip);
    }
  }

  /*!
   * @brief Resolve all callback methods
   * @param handle Pointer to the add-on
   * @return True when all methods were resolved, false otherwise.
   */
  bool RegisterMe(void* handle)
  {
    m_Handle = handle;

    std::string libBasePath;
    libBasePath  = ((cb_array*)m_Handle)->libPath;
    libBasePath += VOIP_HELPER_DLL;

#if defined(ANDROID)
      struct stat st;
      if(stat(libBasePath.c_str(),&st) != 0)
      {
        std::string tempbin = getenv("XBMC_ANDROID_LIBS");
        libBasePath = tempbin + "/" + VOIP_HELPER_DLL_NAME;
      }
#endif

    m_libXBMC_voip = dlopen(libBasePath.c_str(), RTLD_LAZY);
    if (m_libXBMC_voip == NULL)
    {
      fprintf(stderr, "Unable to load %s\n", dlerror());
      return false;
    }

    VOIP_register_me = (void* (*)(void *HANDLE))
      dlsym(m_libXBMC_voip, "VOIP_register_me");
    if (VOIP_register_me == NULL) { fprintf(stderr, "Unable to assign function %s\n", dlerror()); return false; }

    VOIP_unregister_me = (void (*)(void* HANDLE, void* CB))
      dlsym(m_libXBMC_voip, "VOIP_unregister_me");
    if (VOIP_unregister_me == NULL) { fprintf(stderr, "Unable to assign function %s\n", dlerror()); return false; }

    VOIP_add_menu_hook = (void (*)(void* HANDLE, void* CB, VOIP_MENUHOOK *hook))
      dlsym(m_libXBMC_voip, "VOIP_add_menu_hook");
    if (VOIP_add_menu_hook == NULL) { fprintf(stderr, "Unable to assign function %s\n", dlerror()); return false; }

	m_Callbacks = VOIP_register_me(m_Handle);
    return m_Callbacks != NULL;
  }

  void AddMenuHook(VOIP_MENUHOOK* hook)
  {
    return VOIP_add_menu_hook(m_Handle, m_Callbacks, hook);
  }


protected:
  void* (*VOIP_register_me)(void*);
  void (*VOIP_unregister_me)(void*, void*);
  void (*VOIP_add_menu_hook)(void*, void*, VOIP_MENUHOOK*);

private:
  void* m_libXBMC_voip;
  void* m_Handle;
  void* m_Callbacks;
  struct cb_array
  {
    const char* libPath;
  };
};
