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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include "../libpcsxcore/sio.h"

#include "Linux.h"
int isMute=0;
static void CreateMemcard(char *filename, char *conf_mcd)
{
    struct stat buf;
    strcpy(conf_mcd, getenv("HOME"));
    strcat(conf_mcd, MEMCARD_DIR);
    strcat(conf_mcd, filename);
    /* Only create a memory card if an existing one does not exist */
    if (stat(conf_mcd, &buf) == -1)
    {
	printf(_("Creating memory card: %s\n"), conf_mcd);
	CreateMcd(conf_mcd);
    }
}
void execI();
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
extern int updated_display;
void one_iter()
{
    updated_display = 0;
    while (!updated_display)
    {
	execI();
    }
}
void run() __attribute__((used));
int mainloop(const char *isofilename);
void run()
{
    mainloop("_.bin");
}
#endif

int mainloop(const char *isofilename)
{
    char file[MAXPATHLEN] = "";
    char path[MAXPATHLEN];
    int loadst = 0;
    int i;

    SetIsoFile(isofilename);
    memset(&Config, 0, sizeof(PcsxConfig));
    strcpy(Config.Net, "Disabled");
    Config.PsxAuto = 1;
    strcpy(Config.Gpu, "./libDFXVideo.so");
    strcpy(Config.Spu, "./libspuPeteNull.so.1.0.1");
    strcpy(Config.Cdr, "Disabled");
    strcpy(Config.Pad1, "./libDFInput.so");
    strcpy(Config.Pad2, "./libDFInput.so");
    strcpy(Config.Bios, "HLE");
    Config.HLE = TRUE;

    // create & load default memcards if they don't exist
    CreateMemcard("card1.mcd", Config.Mcd1);
    CreateMemcard("card2.mcd", Config.Mcd2);

    LoadMcds(Config.Mcd1, Config.Mcd2);
    // switch to plugin dotdir
    // this lets plugins work without modification!
    //chdir(plugin_default_dir);

    if (SysInit() == -1)
	return 1;
    // the following only occurs if the gui isn't started
    if (LoadPlugins() == -1)
    {
	printf("Error Failed loading plugins!");
	return 1;
    }

    if (OpenPlugins() == -1)
    {
	return 1;
    }

    SysReset();
    CheckCdrom();
    if (LoadCdrom() == -1)
    {
	ClosePlugins();
	printf("Could not load CD-ROM!\n");
	return -1;
    }

#ifdef __EMSCRIPTEN__
    // void emscripten_set_main_loop(em_callback_func func, int fps, int simulate_infinite_loop);
    emscripten_set_main_loop(one_iter, 0, 0);
#else
    while (1)
    {
	execI();
    }
#endif

    return 0;
}

int SysInit()
{

    if (EmuInit() == -1)
    {
	printf(_("PSX emulator couldn't be initialized.\n"));
	return -1;
    }
    LoadMcds(Config.Mcd1, Config.Mcd2);
    return 0;
}

void SysReset()
{
    EmuReset();
}

void SysClose()
{
    EmuShutdown();
    //ReleasePlugins();
}

void SysUpdate()
{
    PADhandleKey(PAD1_keypressed());
    PADhandleKey(PAD2_keypressed());
}

int main(int argc, char *argv[])
{
    char file[MAXPATHLEN] = "";
    char path[MAXPATHLEN];
    int loadst = 0;
    int i;

#ifdef __EMSCRIPTEN__
    emscripten_exit_with_live_runtime();
#else
    return mainloop(argv[1]);
#endif
}