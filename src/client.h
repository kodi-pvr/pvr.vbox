/*
 *  Copyright (C) 2015-2020 Team Kodi
 *  Copyright (C) 2015 Sam Stenvall
 *  https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "vbox/VBox.h"

#include <kodi/libKODI_guilib.h>
#include <kodi/libXBMC_addon.h>
#include <kodi/libXBMC_pvr.h>

#ifdef TARGET_WINDOWS
#define snprintf _snprintf
#endif

// Helpers
extern ADDON::CHelper_libXBMC_addon* XBMC;
extern CHelper_libXBMC_pvr* PVR;
extern CHelper_libKODI_guilib* GUI;

// Globals
extern ADDON_STATUS g_status;
extern vbox::VBox* g_vbox;
extern bool g_skippingInitialEpgLoad;
