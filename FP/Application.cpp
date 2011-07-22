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


#include "Application.h"
#include "Globals.h"
#include <Generic/Debug.h>
#include <LibC/LibC.h>
#include "GUI/GenericUI.h"
#include <PlugLib/PlugLib.h>

uint32 StartupFlags = 0;

static TagItem gui_tags[] =
{
    { PLO_NameSpace,      (iptr)"FryingPan"	},
    { PLO_PluginName,     (iptr)GUI_Name	},
    { PLO_MinVersion,     GUI_Version		},
    { PLO_MinRevision,    GUI_Revision		},
    { 0,		0			}
};

Application::Application() :
    UI(*((UIPlugin*)Plug->OpenPlugin(gui_tags)))
{
    _createDebug(true, "Application");

    if (UI.IsValid())
    {
	_d(Lvl_Info, "%s: Starting the interface", (int)__PRETTY_FUNCTION__);
	UI->start();
	_d(Lvl_Info, "%s: Stoping the interface", (int)__PRETTY_FUNCTION__);
	UI->stop();
	_d(Lvl_Info, "%s: Disposing the interface", (int)__PRETTY_FUNCTION__);
    }
}

Application::~Application()
{
    UI.Dispose();
    _d(Lvl_Info, "%s: Shutting down", (int)__PRETTY_FUNCTION__);
    _destroyDebug();
}

void Application::setDebug(DbgHandler* d)
{
    dbg = d;
}

DbgHandler* Application::getDebug()
{
    return dbg;
}
