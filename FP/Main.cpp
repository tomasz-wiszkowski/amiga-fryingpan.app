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

#include <Generic/Generic.h>
#include <libclass/dos.h>
#include <libclass/intuition.h>
#include <libclass/exec.h>
#include <libclass/utility.h>
#include "Application.h"
#include "Globals.h"
#include <PlugLib/PlugLib.h>

using namespace GenNS;

extern const char* Version;

const char* Version =
"$VER: Frying Pan Main application 1.3.2 (C) 2004-2009 Tomasz Wiszkowski";

ExecIFace *Exec = 0;
DOSIFace *DOS = 0;
UtilityIFace *Utility = 0;
PlugLibIFace *Plug = 0;

int main()
{
    Exec = ExecIFace::GetInstance(SysBase);
    DOS = DOSIFace::GetInstance(0);
    Utility = UtilityIFace::GetInstance(0);
    Plug = PlugLibIFace::GetInstance(0);

    delete new Application();

    Plug->FreeInstance();
    Utility->FreeInstance();
    DOS->FreeInstance();
    Exec->FreeInstance();
}
