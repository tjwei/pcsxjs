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
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/time.h>
#include <regex.h>

#include "Linux.h"

#include "../libpcsxcore/plugins.h"
#include "../libpcsxcore/cheat.h"



void OnFile_RunCd() {

	SysReset();

	if (CheckCdrom() == -1) {
		/* Only check the CD if we are starting the console with a CD */
		ClosePlugins();
		SysErrorMessage (_("CD ROM failed"), _("The CD does not appear to be a valid Playstation CD"));
		SysRunGui();
		return;
	}

	// Read main executable directly from CDRom and start it
	if (LoadCdrom() == -1) {
		ClosePlugins();
		SysErrorMessage(_("Could not load CD-ROM!"), _("The CD-ROM could not be loaded"));
		SysRunGui();
	}

	psxCpu->Execute();
}


void SysErrorMessage(char *primary, char *secondary) {
	printf ("%s - %s\n", primary, secondary);
}

void SysInfoMessage(char *primary, char *secondary) {
		printf ("%s - %s\n", primary, secondary);
}
