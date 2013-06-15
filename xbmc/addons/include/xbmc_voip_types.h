#pragma once
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

#ifndef __VOIPCLIENT_TYPES_H__
#define __VOIPCLIENT_TYPES_H__

#ifdef _WIN32
#include <windows.h>
#else
#ifndef __cdecl
#define __cdecl
#endif
#ifndef __declspec
#define __declspec(X)
#endif
#endif
#include <string.h>

#include "xbmc_addon_types.h"

/*! @note Define "USE_DEMUX" at compile time if demuxing in the PVR add-on is used.
 *        Also XBMC's "DVDDemuxPacket.h" file must be in the include path of the add-on,
 *        and the add-on should set bHandlesDemuxing to true.
 */

#undef ATTRIBUTE_PACKED
#undef PRAGMA_PACK_BEGIN
#undef PRAGMA_PACK_END

#if defined(__GNUC__)
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95)
#define ATTRIBUTE_PACKED __attribute__ ((packed))
#define PRAGMA_PACK 0
#endif
#endif

#if !defined(ATTRIBUTE_PACKED)
#define ATTRIBUTE_PACKED
#define PRAGMA_PACK 1
#endif

#define VOIP_ADDON_NAME_STRING_LENGTH         1024
#define VOIP_ADDON_URL_STRING_LENGTH          1024
#define VOIP_ADDON_DESC_STRING_LENGTH         1024
#define VOIP_ADDON_INPUT_FORMAT_STRING_LENGTH 32

/* using the default avformat's MAX_STREAMS value to be safe */
#define VOIP_STREAM_MAX_STREAMS 20

/* current VOIP API version */
#define XBMC_VOIP_API_VERSION "1.0.0"

/* min. PVR API version */
#define XBMC_VOIP_MIN_API_VERSION "1.0.0"

#ifdef __cplusplus
extern "C" {
#endif

  /*!
   * @brief VOIP add-on error codes
   */
  typedef enum
  {
    VOIP_ERROR_NO_ERROR           =  0,  /*!< @brief no error occurred */
    VOIP_ERROR_UNKNOWN            = -1, /*!< @brief an unknown error occurred */
    VOIP_ERROR_NOT_IMPLEMENTED    = -2, /*!< @brief the method that XBMC called is not implemented by the add-on */
    VOIP_ERROR_SERVER_ERROR       = -3, /*!< @brief the backend reported an error, or the add-on isn't connected */
    VOIP_ERROR_SERVER_TIMEOUT     = -4, /*!< @brief the command was sent to the backend, but the response timed out */
    VOIP_ERROR_REJECTED           = -5, /*!< @brief the command was rejected by the backend */
    VOIP_ERROR_RECORDING_RUNNING  = -6, /*!< @brief a recording is running, so the timer can't be deleted without doing a forced delete */
    VOIP_ERROR_FAILED             = -7, /*!< @brief the command failed */
  } VOIP_ERROR;

  
  /*!
   * @brief VOIP menu hook categories
   */
  typedef enum
  {
    VOIP_MENUHOOK_ALL             = 0, /*!< @brief all categories */
    VOIP_MENUHOOK_SETTING         = 1, /*!< @brief for settings */
  } VOIP_MENUHOOK_CAT;

  /*!
   * @brief Properties passed to the Create() method of an add-on.
   */
  typedef struct VOIP_PROPERTIES
  {
    const char* strUserPath;           /*!< @brief path to the user profile */
    const char* strClientPath;         /*!< @brief path to this add-on */
  } VOIP_PROPERTIES;

  /*!
   * @brief VOIP add-on capabilities. All capabilities are set to "false" as default.
   * If a capabilty is set to true, then the corresponding methods from xbmc_pvr_dll.h need to be implemented.
   */
  typedef struct VOIP_ADDON_CAPABILITIES
  {
    bool bSupportsVoice;
	bool bSupportsVideo;
  } ATTRIBUTE_PACKED VOIP_ADDON_CAPABILITIES;

  /*!
   * @brief PVR stream properties
   */
  typedef struct VOIP_STREAM_PROPERTIES
  {
    unsigned int iStreamCount;
    struct VOIP_STREAM
    {
      unsigned int iPhysicalId;        /*!< @brief (required) physical index */
      unsigned int iCodecType;         /*!< @brief (required) codec type id */
      unsigned int iCodecId;           /*!< @brief (required) codec id */
      char         strLanguage[4];     /*!< @brief (required) language id */
      int          iIdentifier;        /*!< @brief (required) stream id */
      int          iFPSScale;          /*!< @brief (required) scale of 1000 and a rate of 29970 will result in 29.97 fps */
      int          iFPSRate;           /*!< @brief (required) FPS rate */
      int          iHeight;            /*!< @brief (required) height of the stream reported by the demuxer */
      int          iWidth;             /*!< @brief (required) width of the stream reported by the demuxer */
      float        fAspect;            /*!< @brief (required) display aspect ratio of the stream */
      int          iChannels;          /*!< @brief (required) amount of channels */
      int          iSampleRate;        /*!< @brief (required) sample rate */
      int          iBlockAlign;        /*!< @brief (required) block alignment */
      int          iBitRate;           /*!< @brief (required) bit rate */
      int          iBitsPerSample;     /*!< @brief (required) bits per sample */
     } stream[VOIP_STREAM_MAX_STREAMS]; /*!< @brief (required) the streams */
   } ATTRIBUTE_PACKED VOIP_STREAM_PROPERTIES;



  /*!
   * @brief Menu hooks that are available in the context menus while playing a stream via this add-on.
   * And in the Live TV settings dialog
   */
  typedef struct VOIP_MENUHOOK
  {
    unsigned int     iHookId;              /*!< @brief (required) this hook's identifier */
    unsigned int     iLocalizedStringId;   /*!< @brief (required) the id of the label for this hook in g_localizeStrings */
    VOIP_MENUHOOK_CAT category;             /*!< @brief (required) category of menu hook */
  } ATTRIBUTE_PACKED VOIP_MENUHOOK;

  /*!
   * @brief Representation of a TV or radio channel.
   */
  typedef struct VOIP_CONTACT
  {
    unsigned int iUniqueId;                                             /*!< @brief (required) unique identifier for this channel */
    char         strContactlName[VOIP_ADDON_NAME_STRING_LENGTH];         /*!< @brief (optional) channel name given to this channel */
	char *       strContactNew;
  } ATTRIBUTE_PACKED VOIP_CONTACT;

  
  typedef struct VOIPClient
  {
    const char*  (__cdecl* GetVOIPAPIVersion)(void);
    const char*  (__cdecl* GetMininumVOIPAPIVersion)(void);
    VOIP_ERROR   (__cdecl* GetAddonCapabilities)(VOIP_ADDON_CAPABILITIES*);

    const char*  (__cdecl* GetBackendName)(void);
    const char*  (__cdecl* GetBackendVersion)(void);
	
	VOIP_CONTACT*(__cdecl* GetContactById)(int);

	int			 (__cdecl* GetNumberOfContacts)(void);
	VOIP_ERROR   (__cdecl* GetContact)(VOIP_CONTACT&, int);

    VOIP_ERROR   (__cdecl* MenuHook)(const VOIP_MENUHOOK&);
    

  } VOIPClient;

#ifdef __cplusplus
}
#endif

#endif //__VOIPCLIENT_TYPES_H__
