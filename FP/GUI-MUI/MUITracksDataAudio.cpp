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

#include "MUITracksDataAudio.h"
#include <libclass/dos.h>
#include <libclass/intuition.h>
#include <libraries/mui.h>
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>
#include <mui/NListview_mcc.h>
#include <ISOBuilder/ISOBuilder.h>
#include "../IEngine.h"
#include "Globals.h"
#include <workbench/startup.h>

/*
 * localization area
 */
enum Loc
{
    loc_Name    =  lg_TracksData,
    loc_Add,
    loc_Remove,
    loc_Play,
    loc_Return,
    loc_AddISO,

    loc_ColTrack   =  lg_TracksData + ls_Group1,
    loc_ColModule,
    loc_ColInfo,

    loc_SelectTracks = lg_TracksData + ls_Req,
    loc_DirectoriesSelected,
    loc_DirectoriesSelectedButtons,
};

static class Localization::LocaleSet LocaleSets[] =
{
    {	loc_Name,      "Data / Audio Tracks",     "LBL_DATAAUDIOTRACKS"   },
    {	loc_Add,       "a&Add File",              "BTN_ADD"               },
    {	loc_Remove,    "r&Remove",                "BTN_REMOVE"            },
    {	loc_Play,      "p&Play",                  "BTN_PLAY"              },
    {	loc_Return,    "u&Return",		  "BTN_RETURN"		  },
    {	loc_AddISO,    "i&Add ISO",		  "BTN_ADDISO"		  },

    {	loc_ColTrack,  "Track Name",              "COL_TRACKNAME"         },
    {	loc_ColModule, "Module Name",             "COL_MODULENAME"        },
    {	loc_ColInfo,   "Track Information",       "COL_TRACKINFO"         },

    {	loc_SelectTracks,
	"Select tracks to be added to compilation",
	"REQ_SELECTTRACKS" },
    {	loc_DirectoriesSelected,
	"You have specified a directory to be added:\n"
	    "%s\n"
	    "Please specify, whether to\n"
	    "- skip this directory,\n"
	    "- add files contained in this directory only, or\n"
	    "- add all files, recursively from this point",
	"REQ_DIRECTORYSELECTED" },
    {	loc_DirectoriesSelectedButtons,
	"Skip|Add contained|Add all",
	"REQ_DIRECTORYSELECTEDBTNS" },

    {  Localization::LocaleSet::Set_Last, 0, 0                                 }
};

static const char* LocaleGroup = "TRACKS_DA";
static const char* Cfg_LastImagePath    =  "LastImagePath";

MUITracksDataAudio::MUITracksDataAudio(ConfigParser *parent, Globals &glb) :
    Glb(glb)
{
    all    = 0;
    addReq = 0;
    tracks = 0;
    Config = new ConfigParser(parent, "DataAudio", 0);

    hHkButton.Initialize(this, &MUITracksDataAudio::onButton);
    hConstruct.Initialize(this, &MUITracksDataAudio::onConstruct);
    hDestruct.Initialize(this, &MUITracksDataAudio::onDestruct);
    hDisplay.Initialize(this, &MUITracksDataAudio::onDisplay);
    hWBMessage.Initialize(this, &MUITracksDataAudio::onWBMessage);
    hDragSort.Initialize(this, &MUITracksDataAudio::onDragSort);
    hDblClick.Initialize(this, &MUITracksDataAudio::onDblClick);

    setButtonCallBack(hHkButton.GetHook());
    Glb.Loc.AddGroup((Localization::LocaleSet*)LocaleSets, LocaleGroup);
}

MUITracksDataAudio::~MUITracksDataAudio()
{
    if (0 != addReq)
	delete addReq;
    if (0 != tracks)
	delete tracks;
    if (0 != config)
	delete config;
    addReq = 0;
    tracks = 0;
    all = 0;
    delete Config;
}

DbgHandler *MUITracksDataAudio::getDebug()
{
    return Glb.dbg;
}

iptr *MUITracksDataAudio::getObject()
{
    if (NULL != all)
	return all;

    tracks = new GenNS::MUIList("BAR,BAR,", true);
#warning nlist still does not support MUIA_DragDropSort notification.
    tracks->setDragSortable(true);
    tracks->setConstructHook(hConstruct.GetHook());
    tracks->setDestructHook(hDestruct.GetHook());
    tracks->setDisplayHook(hDisplay.GetHook());
    tracks->setWBDropHook(hWBMessage.GetHook());
    tracks->setDragSortHook(hDragSort.GetHook());
    tracks->setDblClickHook(hDblClick.GetHook());

    addReq = new GenNS::FileReq(Glb.Loc[loc_SelectTracks]);
    addReq->setMultiSelect(true);
	    
    config = new GenNS::MUIDynamicObject(true, true);

    all = PageGroup,
	Child,			VGroup,
	    Child,                  tracks->getObject(),
	    Child,                  ColGroup(4),
		Child,			muiButton(Glb.Loc[loc_AddISO], Glb.Loc.Accel(loc_AddISO), ID_AddISO),
		Child,                  muiButton(Glb.Loc[loc_Add], Glb.Loc.Accel(loc_Add), ID_Add),
		Child,                  muiButton(Glb.Loc[loc_Remove], Glb.Loc.Accel(loc_Remove), ID_Remove),
		Child,                  muiButton(Glb.Loc[loc_Play], Glb.Loc.Accel(loc_Play), ID_Play),
	    End,
	End,

	Child,			VGroup,
	    Child,		    config->getObject(),
	    Child,		    muiButton(Glb.Loc[loc_Return], Glb.Loc.Accel(loc_Return), ID_Return),
	End,
    End;

    return all;
}

bool MUITracksDataAudio::start()
{
    addReq->setPath(Config->getValue(Cfg_LastImagePath, ""));
    return true;
}

void MUITracksDataAudio::stop()
{
    Config->setValue(Cfg_LastImagePath, addReq->getPath());
    config->resetChild();
}

const char *MUITracksDataAudio::getName()
{
    return Glb.Loc[loc_Name];
}

void MUITracksDataAudio::update()
{
    IEngine *eng = Glb.CurrentEngine->ObtainRead();

    _d(Lvl_Info, "Updating Data/Audio tracks page");
    VectorT<IData*> &t = eng->tracks().ObtainRead();

    _d(Lvl_Info, "Clearing old items");
    tracks->clear();

    _d(Lvl_Info, "Adding current set of tracks.");
    for (uint32 i=0; i<t.Count(); i++)
    {
	tracks->addItem(t[i]);
    }

    eng->tracks().Release();

    _d(Lvl_Info, "Data/Audio tracks page updated.");

    Glb.CurrentEngine->Release();
}

iptr MUITracksDataAudio::onConstruct(void*, IData* t)
{
    ITrackEntry *e = new ITrackEntry;
    e->track = t;
    e->information = "";

    return (uint32)e;
}

iptr MUITracksDataAudio::onDestruct(void*, ITrackEntry* e)
{
    delete e;
    return 0;
}

iptr MUITracksDataAudio::onDisplay(const char** a, ITrackEntry* e)
{
    if (e == NULL)
    {
	a[0] = Glb.Loc[loc_ColTrack];
	a[1] = Glb.Loc[loc_ColModule];
	a[2] = Glb.Loc[loc_ColInfo];
    }
    else
    {
	uint64 s = 1ull*e->track->getBlockSize() * e->track->getBlockCount();
	const char* t = "B";
	uint32 c = (uint32)s;
	uint32 f = 0;

	if (s > 1024)
	{
	    f = (iptr)(s & 1023) * 100 / 1024;
	    s >>= 10;
	    c = s;
	    t = "kB";
	}
	if (s > 1024)
	{
	    f = (iptr)(s & 1023) * 100 / 1024;
	    s >>= 10;
	    c = s;
	    t = "MB";
	}
	if (s > 1024)
	{
	    f = (iptr)(s & 1023) * 100 / 1024;
	    s >>= 10;
	    c = s;
	    t = "GB";
	}

	e->information = Glb.Loc.FormatNumber(e->track->getBlockCount(), 0) + " sectors (";
	e->information += Glb.Loc.FormatNumber(c, f*10000) + t + ")";

	a[0] = e->track->getTrackName();
	a[1] = e->track->getName();
	a[2] = e->information.Data();
    }
    return (uint32)a;
}

void MUITracksDataAudio::addTracks()
{
    VectorT<const char*> &res = addReq->openReq();
    IEngine *eng = Glb.CurrentEngine->ObtainRead();

    for (uint32 i=0; i<res.Count(); i++)
    {
	addItem(0, res[i], true, false);
    }

    eng->layoutTracks(false, false);
    Glb.CurrentEngine->Release();
}

void MUITracksDataAudio::remTracks()
{
    VectorT<void*> &res = tracks->getSelectedItems();
    IEngine *eng = Glb.CurrentEngine->ObtainRead();

    for (uint32 i=0; i<res.Count(); i++)
    {
	ITrackEntry *e = (ITrackEntry*)res[i];
	eng->remTrack(e->track);
    }

    eng->layoutTracks(false, false);
    Glb.CurrentEngine->Release();
}

iptr MUITracksDataAudio::onButton(BtnID id, void*)
{
    switch (id)
    {
	case ID_Add:
	    {
		addTracks();
	    };
	    break;

	case ID_Remove:
	    {
		remTracks();
	    }
	    break;

	case ID_Play:
	    break;

	case ID_Return:
	    Intuition->SetAttrsA(all, TAGARRAY(MUIA_Group_ActivePage, 0));
	    break;

	case ID_AddISO:
	    {
		addISOBuilder();
	    }
	    break;
    }

    return 0;
}

void MUITracksDataAudio::addItem(BPTR parent, const char* name, bool descent, bool donotask)
{
#warning fill me up!
#if 0
    char *c = new char[1024];
    FileInfoBlock *fib = new FileInfoBlock;
    BPTR lock;
    int res=0;
    bool add = descent;
    IEngine *pEng = Glb.CurrentEngine->ObtainRead();

    if (parent != 0)
	DOS->NameFromLock(parent, c, 1024);
    DOS->AddPart(c, name, 1024);

    lock = DOS->Lock(c, ACCESS_READ);
    if ((lock != 0) && (DOS->Examine(lock, fib)))
    {
	/* are we dealing with a directory or a file? */
	if (fib->fib_EntryType < 0)
	{
	    EDtError rc;
	    /*
	     * file -- add directly
	     */
	    rc = pEng->addTrack(c);
	    const char *s = 0;

	    switch (rc)
	    {
		case DT_OK:
		    break;

		case DT_InvalidOperation:
		    s = "Module was unable to perform requested operation.";
		    break;

		case DT_InvalidFormat:
		    s = "Invalid file format. File will not be imported.";
		    break;

		case DT_OutOfMemory:
		    s = "Not enough memory to open file.";
		    break;

		case DT_RequiredResourceNotAvailable:
		    s = "Required library (or another resource) is not available.";
		    break;

		case DT_UnableToOpenFile:
		    s = "The selected file could not be found.";
		    break;

		case DT_FileMalformed:
		    s = "File appears to be damaged.";
		    break;

		case DT_WrongChannelCount:
		    s = "Number of audio channels is different than two (stereo).";
		    break;

		case DT_WrongFrequency:
		    s = "Audio frequency is different than 44.1kHz.";
		    break;

		case DT_WrongResolution:
		    s = "Audio resolution is different than 16bits per sample.";
		    break;

		case DT_FileFormatNotSupported:
		    s = "This particular file format (i.e. compression) is not supported.";
		    break;
	    }

	    if (rc != DT_OK)
	    {
		request(Glb.Loc[Globals::loc_Error], "An error occured while opening the file:\n%s\nOperation will be aborted.", Glb.Loc[Globals::loc_OK], ARRAY((int)s));
	    }
	}
	else
	{
	    /*
	     * directory: first ask if the user wants to add it.. 
	     */
	    if (!donotask)
	    {
		res = request(Glb.Loc[Globals::loc_Req], 
			Glb.Loc[loc_DirectoriesSelected], 
			Glb.Loc[loc_DirectoriesSelectedButtons],
			ARRAY((iptr)c));
		donotask = true;
		descent = (res == 0);
		add = (res != 1);
	    }

	    /*
	     * if he does, read contents and add items
	     */
	    if (add)
	    {
		BPTR lock2 = DOS->Lock(c, ACCESS_READ);
		if ((lock2 != 0) && (DOS->Examine(lock2, fib)))
		{
		    while (DOS->ExNext(lock2, fib))
		    {
			addItem(lock2, (char*)fib->fib_FileName, descent, true);
		    }
		}

		if (lock2 != 0)
		    DOS->UnLock(lock2);
	    }
	}
    }

    if (0 != lock)
	DOS->UnLock(lock);

    delete[]c;
    delete fib;
    Glb.CurrentEngine->Release();
#endif
}

void MUITracksDataAudio::addISOBuilder()
{
    IEngine *pEng = Glb.CurrentEngine->ObtainRead();

    IData* rdr = Glb.ISO->CreateISO(0);
    if (rdr != 0)
	pEng->addTrack(rdr);

    Glb.CurrentEngine->Release();
}

iptr MUITracksDataAudio::onWBMessage(AppMessage* m, void*)
{
    for (int32 i=0; i<m->am_NumArgs; i++)
	addItem(m->am_ArgList[i].wa_Lock, (char*)m->am_ArgList[i].wa_Name, true, false);

    IEngine *pEng = Glb.CurrentEngine->ObtainRead();
    pEng->layoutTracks(false, false);
    Glb.CurrentEngine->Release();
    return 0;
}

iptr MUITracksDataAudio::onDragSort(VectorT<ITrackEntry*>*vec, void*)
{
    request("Info", "Newly sorted vector has %ld items.", "Ok", ARRAY(vec->Count()));
    return 0;
}

iptr MUITracksDataAudio::onDblClick(MUIList*, ITrackEntry* e)
{
#warning musimy dodac rozpoznanie modulu tutaj!
    config->setChild(e->track->getSettingsPage());
    Intuition->SetAttrsA(all, TAGARRAY(MUIA_Group_ActivePage, 1));
    return 0;
}
