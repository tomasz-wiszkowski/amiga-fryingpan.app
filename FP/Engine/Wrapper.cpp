/*
 * FryingPan - Amiga CD/DVD Recording Software (User Intnerface and supporting Libraries only)
 * Copyright (C) 2001-2008 Tomasz Wiszkowski Tomasz.Wiszkowski at gmail.com
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#include <GenericLib/Library.h>
#include <libclass/dos.h>
#include <libclass/intuition.h>
#include <libclass/exec.h>
#include <libclass/utility.h>
#include <LibC/LibC.h>
#include <libclass/Optical.h>
#include <DTLib/Calls.h>
#include <exec/libraries.h>
#include <PlugLib/PlugLib.h>
#include <PlugLib/IPlugin.h>

#include "Engine.h"
#include "Globals.h"

// TARGET:
// both fryingpan and coffe work on common engine
// this CAN BE DONE!

LIBRARY("fryingpan.engine", "$VER: fryingpan.engine 1.2 Tomasz Wiszkowski", 1);
GATE1(IEngine*,         getEngine,     int,              d0);
GATE1(const ScanData*,  scanDevice,    const char*,      a0);
GATE1(void,             freeScanData,  const ScanData*,  a0);

    LIB_FT_Begin
    LIB_FT_Function(getEngine)
    LIB_FT_Function(scanDevice)
    LIB_FT_Function(freeScanData)
    LIB_FT_End

    uint32            StartupFlags = 0;

    PlugLibIFace     *plug = 0;
    ExecIFace        *Exec = 0;
    IntuitionIFace   *Intuition = 0;
    UtilityIFace     *Utility = 0;
    DOSIFace         *DOS = 0;


bool Lib_SetUp()
{
    __setup();

    OurBase->lib_Flags |= LIBF_DELEXP;

    Exec = ExecIFace::GetInstance(SysBase);
    DOS = DOSIFace::GetInstance(0);
    Intuition = IntuitionIFace::GetInstance(0);
    Utility = UtilityIFace::GetInstance(0);
    plug = PlugLibIFace::GetInstance(0);

    glob = new Globals(plug);

    //
    // we're done with initialization
    //
    return true;
}

void Lib_CleanUp()
{
    delete glob;
    plug->FreeInstance();
    Utility->FreeInstance();
    Intuition->FreeInstance();
    DOS->FreeInstance();
    Exec->FreeInstance();
    __cleanup();
}

bool Lib_Acquire()
{
    return true;
}

void Lib_Release()
{
    glob->Update();
}

IEngine *getEngine(int which)
{
    ASSERT((0 <= which) && (3 >= which));
    return glob->Engines[which];
}


// vim: ts=3 et
