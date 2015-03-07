#pragma once
/*
 *      Copyright (C) 2015 Sam Stenvall
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301  USA
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include <string>
#include "libXBMC_addon.h"
#include "libXBMC_pvr.h"
#include "libXBMC_gui.h"
#include "vbox/VBox.h"

#ifdef TARGET_WINDOWS
#define snprintf _snprintf
#endif

// Helpers
extern ADDON::CHelper_libXBMC_addon *XBMC;
extern CHelper_libXBMC_pvr          *PVR;

// Globals
extern ADDON_STATUS g_status;
extern vbox::VBox *g_vbox;

// Settings
extern std::string g_hostname;
extern int g_port;
extern int g_timeout;
