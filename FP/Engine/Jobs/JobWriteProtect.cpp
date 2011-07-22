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


#include "JobWriteProtect.h"

JobWriteProtect::JobWriteProtect(Globals &glb, iptr drive, bool &state) :
   Job(glb, drive),
   newstate(state)
{
}

void JobWriteProtect::execute()
{
   int32 newstat;
   g.Optical->DoMethodA(ARRAY(DRV_SetAttrs, Drive, DRA_Disc_WriteProtect, newstate, 0, 0));
   g.Optical->DoMethodA(ARRAY(DRV_GetAttrs, Drive, DRA_Disc_WriteProtect, (iptr)&newstat, 0, 0));
   newstate = newstat ? true : false;
}

uint32 JobWriteProtect::getProgress()
{
   return 0;
}

const char *JobWriteProtect::getActionName()
{
   return "Changing Write Protection Status";
}

bool JobWriteProtect::inhibitDOS()
{
   return false;
}

