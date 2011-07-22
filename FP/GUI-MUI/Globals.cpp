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

#include "Globals.h"
#include <Generic/Locale.h>
#include <PlugLib/PlugLib.h>
#include "Plug.h"

using namespace GenNS;

class MUIMasterIFace *MUIMaster;
class DatatypesIFace *Datatypes;

static class Localization::LocaleSet LocaleSets[] =
{
   {  Globals::loc_Req,             "Request",        "REQUEST"      },
   {  Globals::loc_Info,            "Information",    "INFORMATION"  },
   {  Globals::loc_Warn,            "Warning",        "WARNING"      },
   {  Globals::loc_Error,           "Error",          "ERROR"        },

   {  Globals::loc_OK,              "OK",             "OK"           },
   {  Globals::loc_Proceed,         "Proceed",        "PROCEED"      },
   {  Globals::loc_Abort,           "Abort",          "ABORT"        },
   {  Globals::loc_YesNo,           "Yes|No",         "YESNO"        },
   {  Globals::loc_ContinueAbort,   "Continue|Abort", "CONTINUEABORT"},
   {  Localization::LocaleSet::Set_Last, 0, 0                        }
};

static const char* LocaleGroup = "GLOBAL";

static TagItem iso_tags[] =
{
    { PLO_NameSpace,      (iptr)"FryingPan"	},
    { PLO_PluginName,     (iptr)ISOBuilder_Name },
    { PLO_MinVersion,     ISOBuilder_Version	},
    { PLO_MinRevision,    ISOBuilder_Revision	},
    { 0,		0			}
};

static TagItem dt_tags[] =
{
    { PLO_NameSpace,      (iptr)"FryingPan"	},
    { PLO_PluginName,     (iptr)DTLib_Name	},
    { PLO_MinVersion,     DTLib_Version		},
    { PLO_MinRevision,    DTLib_Revision	},
    { 0,		0			}
};

Globals::Globals() :
    DT(*((DTLibPlugin*)plug->OpenPlugin(dt_tags))),
    ISO(*((ISOBuilderPlugin*)plug->OpenPlugin(iso_tags)))
 {
   Loc.AddGroup((Localization::LocaleSet*)LocaleSets, LocaleGroup);
}
   
void Globals::FormatSpeed(String&s, DiscSpeed& spd, bool mini)
{
    String   t;
	
    /*
    ** starting element always the same
    */
    s = Loc.FormatNumber(spd.begin_i, spd.begin_f * 100000);

    /*
    ** now, depending on speed type...
    */
    switch (spd.type)
    {
	case DIF_Speed_Unknown:
	    s += "x ";
	    if (!mini)
	    {
		s += "(";
		s += Loc.FormatNumber(spd.begin_kbps);
		s += "kBps) ";
	    }
	    s += "(\?\?\?)";
	    break;

	case DIF_Speed_CAV:
	    s += "~";
	    s += Loc.FormatNumber(spd.end_i, spd.end_f * 100000);
	    s += "x ";
	    if (!mini)
	    {
		s += "(";
		s += Loc.FormatNumber(spd.begin_kbps);
		s += "~";
		s += Loc.FormatNumber(spd.end_kbps);
		s += "kBps) ";
	    }
	    s += "(CAV)";
	    break;

	case DIF_Speed_CLV:
	    s += "x ";
	    if (!mini)
	    {
		s += "(";
		s += Loc.FormatNumber(spd.begin_kbps);
		s += "kBps) ";
	    }
	    s += "(CLV)";
	    break;

	case DIF_Speed_ZCLV:
	    s += "+";
	    s += Loc.FormatNumber(spd.end_i, spd.end_f * 100000);
	    s += "x ";
	    if (!mini)
	    {
		s += "(";
		s += Loc.FormatNumber(spd.begin_kbps);
		s += "+";
		s += Loc.FormatNumber(spd.end_kbps);
		s += "kBps) ";
	    }
	    s += "(Z-CLV)";
	    break;

	case DIF_Speed_CAVCLV:
	    s += "->";
	    s += Loc.FormatNumber(spd.end_i, spd.end_f * 100000);
	    s += "x ";
	    if (!mini)
	    {
		s += "(";
		s += Loc.FormatNumber(spd.begin_kbps);
		s += "->";
		s += Loc.FormatNumber(spd.end_kbps);
		s += "kBps) ";
	    }
	    s += "(CAV+CLV)";
	    break;
    }

    _dx(Lvl_Info, "Speed entry: '%s'", (iptr)s.Data());
    return;
}

