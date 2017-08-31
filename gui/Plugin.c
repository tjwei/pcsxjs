/*  Pcsx - Pc Psx Emulator
 *  Copyright (C) 1999-2002  Pcsx Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1307 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <SDL/SDL.h>
#include <signal.h>

#include "Linux.h"

#include "../libpcsxcore/plugins.h"
#include "../libpcsxcore/spu.h"
#include "../libpcsxcore/cdriso.h"

#include "nopic.h"

#define MAX_SLOTS 5 /* ADB TODO Same as Gtk2Gui.c */

unsigned long gpuDisp;

int StatesC = 0;
int ShowPic = 0;

static short modctrl = 0, modalt = 0;

/* Handle keyboard keystrokes */
void PADhandleKey(int key)
{
    char Text[MAXPATHLEN];
    short rel = 0; //released key flag

    if (key == 0)
	return;

    if ((key >> 30) & 1) //specific to dfinput (padJoy)
	rel = 1;

    if (key == SDLK_ESCAPE)
    {
	ClosePlugins();
	exit(1);
    }
    //else GPU_keypressed(key);
}

void SignalExit(int sig)
{
    ClosePlugins();
}

#define PARSEPATH(dst, src)            \
    ptr = src + strlen(src);           \
    while (*ptr != '\\' && ptr != src) \
	ptr--;                         \
    if (ptr != src)                    \
    {                                  \
	strcpy(dst, ptr + 1);          \
    }

int _OpenPlugins()
{
    int ret;

    GPU_clearDynarec(clearDynarec);

    ret = CDR_open();
    if (ret < 0)
    {
	SysMessage(_("Error opening CD-ROM plugin!"));
	return -1;
    }

    ret = GPU_open(&gpuDisp, "PCSX", NULL);
    if (ret < 0)
    {
	SysMessage(_("Error opening GPU plugin!"));
	return -1;
    }

    ret = SPU_open();
    if (ret < 0)
    {
	SysMessage(_("Error opening SPU plugin!"));
	return -1;
    }
    SPU_registerCallback(SPUirq);

    ret = PAD1_open(&gpuDisp);
    if (ret < 0)
    {
	SysMessage(_("Error opening Controller 1 plugin!"));
	return -1;
    }
    ret = PAD2_open(&gpuDisp);
    if (ret < 0)
    {
	SysMessage(_("Error opening Controller 2 plugin!"));
	return -1;
    }

    return 0;
}

int OpenPlugins()
{
    int ret;

    while ((ret = _OpenPlugins()) == -2)
    {
	//ReleasePlugins();
	LoadMcds(Config.Mcd1, Config.Mcd2);
	if (LoadPlugins() == -1)
	    return -1;
    }
    return ret;
}

void ClosePlugins()
{
    int ret;

    ret = CDR_close();
    if (ret < 0)
    {
	SysMessage(_("Error closing CD-ROM plugin!"));
	return;
    }
    ret = SPU_close();
    if (ret < 0)
    {
	SysMessage(_("Error closing SPU plugin!"));
	return;
    }
    ret = PAD1_close();
    if (ret < 0)
    {
	SysMessage(_("Error closing Controller 1 Plugin!"));
	return;
    }
    ret = PAD2_close();
    if (ret < 0)
    {
	SysMessage(_("Error closing Controller 2 plugin!"));
	return;
    }
    ret = GPU_close();
    if (ret < 0)
    {
	SysMessage(_("Error closing GPU plugin!"));
	return;
    }
}
