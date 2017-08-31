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
static void CreateMemcard(char *filename, char *conf_mcd)
{
    struct stat buf;
    strcpy(conf_mcd, getenv("HOME"));
    strcat(conf_mcd, MEMCARD_DIR);
    if (stat(conf_mcd, &buf) == -1)
    {
        mkdir(conf_mcd, S_IRWXU);
    }
    strcat(conf_mcd, filename);
    /* Only create a memory card if an existing one does not exist */
    if (stat(conf_mcd, &buf) == -1)
    {
        printf(_("Creating memory card: %s\n"), conf_mcd);
        CreateMcd(conf_mcd);
    }
}
void execI();
#include <emscripten.h>
extern long updated_display;
int DoGPUUpdate = 0;

void GPUupdateLace0(void);
void GPUupdateLace1(void);
void one_iter()
{
    if (updated_display != -1)
    {
        GPUupdateLace1();

        EmuUpdate();
        updated_display = -1;
    }
    DoGPUUpdate = 0;    
    while (!DoGPUUpdate)
    {
        execI();
    }
    GPU_vBlank(0);
    //setIrq( 0x01 );
    psxHu32ref(0x1070) |= 1;
    GPUupdateLace0();
    EM_ASM_({

        setTimeout("pcsx_mainloop()", $0);
    },
            updated_display / 1000);
}

int pcsx_init(const char *isofilename)
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
    printf("end pcsx_init\n");
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

void ls(const char *dirname)
{
    DIR *dir;
    struct dirent *ent;
    printf("showdir %s\n", dirname);
    if ((dir = opendir(dirname)) != NULL)
    {
        /* print all the files and directories within directory */
        while ((ent = readdir(dir)) != NULL)
        {
            printf("%s\n", ent->d_name);
        }
        closedir(dir);
    }
    else
    {
        /* could not open directory */
        printf("cannot open %s\n", dirname);
        return;
    }
}
int main(int argc, char *argv[])
{
    EM_ASM({ 
        FS.mkdir('/home/web_user/.pcsx'); 
        clear_event_history();  
});
    emscripten_exit_with_live_runtime();
}