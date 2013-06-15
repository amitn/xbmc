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

#include <vector>
#include <cstdio>
#include "VOIPClient.h"
#include "settings/AdvancedSettings.h"
#include "utils/log.h"
#include "settings/GUISettings.h"
#include "Application.h"

using namespace std;
using namespace ADDON;
using namespace VOIP;

#define DEFAULT_INFO_STRING_VALUE "unknown"

CVOIPClient::CVOIPClient(const AddonProps& props) :
    CAddonDll<DllVOIPClient, VOIPClient, VOIP_PROPERTIES>(props),
    m_apiVersion("0.0.0")
{
  ResetProperties();
}

CVOIPClient::CVOIPClient(const cp_extension_t *ext) :
    CAddonDll<DllVOIPClient, VOIPClient, VOIP_PROPERTIES>(ext),
    m_apiVersion("0.0.0")
{
  ResetProperties();
}

CVOIPClient::~CVOIPClient(void)
{
  Destroy();
  SAFE_DELETE(m_pInfo);
}

void CVOIPClient::ResetProperties(int iClientId /* = VOIP_INVALID_CLIENT_ID */)
{
  /* initialise members */
  SAFE_DELETE(m_pInfo);
  m_pInfo				  = new VOIP_PROPERTIES;
  m_strUserPath           = CSpecialProtocol::TranslatePath(Profile());
  m_pInfo->strUserPath    = m_strUserPath.c_str();
  m_strClientPath         = CSpecialProtocol::TranslatePath(Path());
  m_pInfo->strClientPath  = m_strClientPath.c_str();
  m_menuhooks.clear();
  m_bReadyToUse           = false;
  m_iClientId             = iClientId;
  
  
  m_apiVersion = AddonVersion("0.0.0");
}

ADDON_STATUS CVOIPClient::Create(int iClientId)
{
  ADDON_STATUS status(ADDON_STATUS_UNKNOWN);
  if (iClientId <= VOIP_INVALID_CLIENT_ID || iClientId == VOIP_VIRTUAL_CLIENT_ID)
    return status;

  /* ensure that a previous instance is destroyed */
  Destroy();

  /* reset all properties to defaults */
  ResetProperties(iClientId);

  /* initialise the add-on */
  bool bReadyToUse(false);
  CLog::Log(LOGDEBUG, "VOIP - %s - creating VOIP add-on instance '%s'", __FUNCTION__, Name().c_str());
  try
  {
    if ((status = CAddonDll<DllVOIPClient, VOIPClient, VOIP_PROPERTIES>::Create()) == ADDON_STATUS_OK)
      bReadyToUse = GetAddonProperties();
  }
  catch (exception &e) { LogException(e, __FUNCTION__); }

  m_bReadyToUse = bReadyToUse;

  return status;
}

bool CVOIPClient::DllLoaded(void) const
{
  try { return CAddonDll<DllVOIPClient, VOIPClient, VOIP_PROPERTIES>::DllLoaded(); }
  catch (exception &e) { LogException(e, __FUNCTION__); }

  return false;
}

void CVOIPClient::Destroy(void)
{
  if (!m_bReadyToUse)
    return;
  m_bReadyToUse = false;

  /* reset 'ready to use' to false */
  CLog::Log(LOGDEBUG, "VOIP - %s - destroying VOIP add-on '%s'", __FUNCTION__, GetFriendlyName().c_str());

  /* destroy the add-on */
  try { CAddonDll<DllVOIPClient, VOIPClient, VOIP_PROPERTIES>::Destroy(); }
  catch (exception &e) { LogException(e, __FUNCTION__); }

  /* reset all properties to defaults */
  ResetProperties();
}

void CVOIPClient::ReCreate(void)
{
  int iClientID(m_iClientId);
  Destroy();

  /* recreate the instance */
  Create(iClientID);
}

bool CVOIPClient::ReadyToUse(void) const
{
  return m_bReadyToUse;
}

int CVOIPClient::GetID(void) const
{
  return m_iClientId;
}


bool CVOIPClient::IsCompatibleAPIVersion(const ADDON::AddonVersion &minVersion, const ADDON::AddonVersion &version)
{
  AddonVersion myMinVersion = AddonVersion(XBMC_VOIP_MIN_API_VERSION);
  AddonVersion myVersion = AddonVersion(XBMC_VOIP_API_VERSION);
  return (version >= myMinVersion && minVersion <= myVersion);
}

bool CVOIPClient::CheckAPIVersion(void)
{
  /* check the API version */
  AddonVersion minVersion = AddonVersion(XBMC_VOIP_MIN_API_VERSION);
  try { m_apiVersion = AddonVersion(m_pStruct->GetVOIPAPIVersion()); }
  catch (exception &e) { LogException(e, "GetVOIPAPIVersion()"); return false;  }

  if (!IsCompatibleAPIVersion(minVersion, m_apiVersion))
  {
    CLog::Log(LOGERROR, "VOIP - Add-on '%s' is using an incompatible API version. XBMC minimum API version = '%s', add-on API version '%s'", Name().c_str(), minVersion.c_str(), m_apiVersion.c_str());
    return false;
  }

  return true;
}

bool CVOIPClient::GetAddonProperties(void)
{
  CStdString strHostName, strBackendName, strConnectionString, strFriendlyName, strBackendVersion;
  VOIP_ADDON_CAPABILITIES addonCapabilities;

  /* get the capabilities */
  try
  {
    memset(&addonCapabilities, 0, sizeof(addonCapabilities));
    VOIP_ERROR retVal = m_pStruct->GetAddonCapabilities(&addonCapabilities);
    if (retVal != VOIP_ERROR_NO_ERROR)
    {
      CLog::Log(LOGERROR, "VOIP - couldn't get the capabilities for add-on '%s'. Please contact the developer of this add-on: %s", GetFriendlyName().c_str(), Author().c_str());
      return false;
    }
  }
  catch (exception &e) { LogException(e, "GetAddonCapabilities()"); return false; }

  /* get the name of the backend */
  try { strBackendName = m_pStruct->GetBackendName(); }
  catch (exception &e) { LogException(e, "GetBackendName()"); return false;  }

  /* display name = backend name:connection string */
  strFriendlyName.Format("%s:%s", strBackendName.c_str(), strConnectionString.c_str());

  /* backend version number */
  try { strBackendVersion = m_pStruct->GetBackendVersion(); }
  catch (exception &e) { LogException(e, "GetBackendVersion()"); return false;  }

  /* update the members */
  m_strBackendName      = strBackendName;
  m_strConnectionString = strConnectionString;
  m_strFriendlyName     = strFriendlyName;
  m_strBackendVersion   = strBackendVersion;
  m_addonCapabilities   = addonCapabilities;

  return true;
}

VOIP_ADDON_CAPABILITIES CVOIPClient::GetAddonCapabilities(void) const
{
  VOIP_ADDON_CAPABILITIES addonCapabilities(m_addonCapabilities);
  return addonCapabilities;
}

CStdString CVOIPClient::GetBackendName(void) const
{
  CStdString strReturn(m_strBackendName);
  return strReturn;
}

CStdString CVOIPClient::GetBackendVersion(void) const
{
  CStdString strReturn(m_strBackendVersion);
  return strReturn;
}


CStdString CVOIPClient::GetFriendlyName(void) const
{
  CStdString strReturn(m_strFriendlyName);
  return strReturn;
}

void CVOIPClient::CallMenuHook(const VOIP_MENUHOOK &hook)
{
  if (!m_bReadyToUse)
    return;
  try { m_pStruct->MenuHook(hook); }
  catch (exception &e) { LogException(e, __FUNCTION__); }
}

int CVOIPClient::GetNumberOfContacts(void)
{
  int numberOfContacts = -1;
  try{ numberOfContacts = m_pStruct->GetNumberOfContacts(); } 
  catch (exception &e) { LogException(e, __FUNCTION__); }
  return numberOfContacts;
}

void CVOIPClient::GetContactById(int id)
{
  VOIP_CONTACT * pContact = NULL;
  try{ pContact = m_pStruct->GetContactById(1);
    CLog::Log(LOGERROR, "VOIP - Contact name %s" , pContact->strContactNew);
  } 
  catch (exception &e) { LogException(e, __FUNCTION__); }
  
}

VOIP_MENUHOOKS *CVOIPClient::GetMenuHooks(void)
{
  return &m_menuhooks;
}

const char *CVOIPClient::ToString(const VOIP_ERROR error)
{
  switch (error)
  {
  case VOIP_ERROR_NO_ERROR:
    return "no error";
  case VOIP_ERROR_NOT_IMPLEMENTED:
    return "not implemented";
  case VOIP_ERROR_SERVER_ERROR:
    return "server error";
  case VOIP_ERROR_SERVER_TIMEOUT:
    return "server timeout";
  case VOIP_ERROR_REJECTED:
    return "rejected by the backend";
  case VOIP_ERROR_FAILED:
    return "the command failed";
  case VOIP_ERROR_UNKNOWN:
  default:
    return "unknown error";
  }
}

bool CVOIPClient::LogError(const VOIP_ERROR error, const char *strMethod) const
{
  if (error != VOIP_ERROR_NO_ERROR)
  {
    CLog::Log(LOGERROR, "VOIP - %s - addon '%s' returned an error: %s",
        strMethod, GetFriendlyName().c_str(), ToString(error));
    return false;
  }
  return true;
}

void CVOIPClient::LogException(const exception &e, const char *strFunctionName) const
{
  CLog::Log(LOGERROR, "VOIP - exception '%s' caught while trying to call '%s' on add-on '%s'. Please contact the developer of this add-on: %s", e.what(), strFunctionName, GetFriendlyName().c_str(), Author().c_str());
}