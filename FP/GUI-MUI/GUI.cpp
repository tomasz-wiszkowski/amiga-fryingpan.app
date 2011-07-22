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

#include "GUI.h"
#include <libclass/dos.h>
#include <libclass/intuition.h>
#include <libclass/exec.h>
#include <libclass/utility.h>
#include <libraries/mui.h>
#include "MUIPageSelect.h"
#include "MUITracksDataAudio.h"
#include "MUIMedia.h"
#include "MUIContents.h"
#include "MUIDrive.h"
#include "MUISettings.h"
#include "MUIRecord.h"
#include "MUIDriveSelect.h"
#include "MUIPopAction.h"
#include <Generic/Configuration.h>
#include "../IEngine.h"
#include <FP/Engine/Plug.h>
#include <Generic/Debug.h>
#include <libclass/datatypes.h>
#include <LibC/LibC.h>
#include <libclass/Optical.h>
#include <PlugLib/PlugLib.h>
#undef String

using namespace GenNS;

/*
 * libraries...
 */
extern PlugLibIFace *plug;

/*
 * localization area
 */
enum Loc
{
    loc_File                   = lg_Global,
    loc_AboutMUI,
    loc_MUISettings,
    loc_Translate,
    loc_Quit,

    loc_DriveProgress	    = lg_Global+ls_Group1,

    loc_DrSt_Unknown	    = lg_Global+ls_Group2,
    loc_DrSt_NotOpened,
    loc_DrSt_Ready,
    loc_DrSt_NoDisc,
    loc_DrSt_Busy,

    loc_DrOp_Unknown	    = lg_Global+ls_Group3,	
    loc_DrOp_NoOperation,	
    loc_DrOp_Play,		
    loc_DrOp_Pause,		
    loc_DrOp_Read,		

    loc_DrOp_Ctl_Eject	    = lg_Global+ls_Group4,	
    loc_DrOp_Ctl_Load,	
    loc_DrOp_Ctl_SpinUp,	
    loc_DrOp_Ctl_SpinDown,	
    loc_DrOp_Ctl_Standby,	

    loc_DrOp_Als_General    = lg_Global+ls_Group5,	
    loc_DrOp_Als_Drive,	
    loc_DrOp_Als_Tracks,	
    loc_DrOp_Als_Indices,	
    loc_DrOp_Als_DataType,	
    loc_DrOp_Als_CDText,	
    loc_DrOp_Als_FreeDB,	
    loc_DrOp_Als_DiscID,	
    loc_DrOp_Als_DCB,	
    loc_DrOp_Als_Speeds,	
    loc_DrOp_Als_Layout,	

    loc_DrOp_Wrt_General    = lg_Global+ls_Group6,	
    loc_DrOp_Wrt_Calibrate,	
    loc_DrOp_Wrt_Allocate,	
    loc_DrOp_Wrt_CDText,	
    loc_DrOp_Wrt_Data,	
    loc_DrOp_Wrt_Sync,	
    loc_DrOp_Wrt_CloseTrk,	
    loc_DrOp_Wrt_CloseSess,	
    loc_DrOp_Wrt_CloseDisc,	
    loc_DrOp_Wrt_Repair,	

    loc_DrOp_Blk_General    = lg_Global+ls_Group7,	
    loc_DrOp_Blk_CBlank,	
    loc_DrOp_Blk_QBlank,	
    loc_DrOp_Blk_SBlank,	
    loc_DrOp_Blk_CFormat,	
    loc_DrOp_Blk_QFormat,	
    loc_DrOp_Blk_SFormat,	

    loc_FailedToCreateApp   = lg_Global+ls_Req,
};

static class Localization::LocaleSet LocaleSets[] =
{
    {	loc_File,               "File",			    "MNU_FILE"		    },
    {	loc_AboutMUI,           "a&About MUI",		    "MNU_ABOUT_MUI"	    },
    {	loc_MUISettings,        "s&MUI Settings",	    "MNU_MUI_SETTINGS"	    },
    {	loc_Translate,          "t&Translate",		    "MNU_TRANSLATE"	    },
    {	loc_Quit,               "q&Quit",		    "MNU_QUIT"		    },
    {	loc_DriveProgress,      "Drive Progress",	    "LBL_DRIVE_PROGRESS"    },

    {	loc_DrSt_Unknown,	"Unknown",		    "LBL_DRSTAT_UNKNOWN"    },
    {	loc_DrSt_NotOpened,	"Not Opened",		    "LBL_DRSTAT_NOTOPENED"  },
    {	loc_DrSt_Ready,		"Ready",		    "LBL_DRSTAT_READY" 	    },
    {	loc_DrSt_NoDisc,	"No Disc",		    "LBL_DRSTAT_NODISC"     },
    {	loc_DrSt_Busy,		"Busy",			    "LBL_DRSTAT_BUSY"	    },

    {	loc_DrOp_Unknown,	"Unknown",		    "LBL_DROPER_UNKNOWN"    },
    {	loc_DrOp_NoOperation,	"Idle",			    "LBL_DROPER_IDLE"	    },
    {	loc_DrOp_Play,		"Playing",		    "LBL_DROPER_PLAYING"    },
    {	loc_DrOp_Pause,		"Paused",		    "LBL_DROPER_PAUSED"	    },
    {	loc_DrOp_Read,		"Reading",		    "LBL_DROPER_READING"    },

    {	loc_DrOp_Ctl_Eject,	"Ejecting",		    "LBL_DROPER_CTLEJECT"   },
    {	loc_DrOp_Ctl_Load,	"Loading",		    "LBL_DROPER_CTLLOAD"    },
    {	loc_DrOp_Ctl_SpinUp,	"Spinning up",		    "LBL_DROPER_CTLSPINUP"  },
    {	loc_DrOp_Ctl_SpinDown,	"Spinning down",	    "LBL_DROPER_CTLSPINDN"  },
    {	loc_DrOp_Ctl_Standby,	"Going standby",	    "LBL_DROPER_CTLSTANDBY" },

    {	loc_DrOp_Als_General,	"Analysing",		    "LBL_DROPER_ANALYSE"    },
    {	loc_DrOp_Als_Drive,	"Analysing capabilities",   "LBL_DROPER_ALSDRIVE"   },
    {	loc_DrOp_Als_Tracks,	"Analysing tracks",	    "LBL_DROPER_ALSTRACKS"  },
    {	loc_DrOp_Als_Indices,	"Detecting indices",	    "LBL_DROPER_ALSINDICES" },
    {	loc_DrOp_Als_DataType,	"Analysing track types",    "LBL_DROPER_ALSTYPES"   },
    {	loc_DrOp_Als_CDText,	"Reading CD Text",	    "LBL_DROPER_ALSCDTEXT"  },
    {	loc_DrOp_Als_FreeDB,	"Checking FreeDB Entries",  "LBL_DROPER_ALSFREEDB"  },
    {	loc_DrOp_Als_DiscID,	"Reading Disc ID",	    "LBL_DROPER_ALSDISCID"  },
    {	loc_DrOp_Als_DCB,	"Analysing control blocks", "LBL_DROPER_ALSDCB"	    },
    {	loc_DrOp_Als_Speeds,	"Analysing speeds",	    "LBL_DROPER_ALSSPEEDS"  },
    {	loc_DrOp_Als_Layout,	"Calculating layout",	    "LBL_DROPER_ALSLAYOUT"  },

    {	loc_DrOp_Wrt_General,	"Writing",		    "LBL_DROPER_WRITING"	},
    {	loc_DrOp_Wrt_Calibrate,	"Calibrating laser",	    "LBL_DROPER_WRTCALIBRATE"   },
    {	loc_DrOp_Wrt_Allocate,	"Allocating disc space",    "LBL_DROPER_WRTALLOCATE" 	},
    {	loc_DrOp_Wrt_CDText,	"Writing CD Text",	    "LBL_DROPER_WRTCDTEXT"	},
    {	loc_DrOp_Wrt_Data,	"Writing data",		    "LBL_DROPER_WRTDATA"	},
    {	loc_DrOp_Wrt_Sync,	"Synchronizing cache",	    "LBL_DROPER_WRTSYNC"	},
    {	loc_DrOp_Wrt_CloseTrk,	"Closing track",	    "LBL_DROPER_WRTCLOSETRK"	},
    {	loc_DrOp_Wrt_CloseSess,	"Closing session",	    "LBL_DROPER_WRTCLOSESESS"	},
    {	loc_DrOp_Wrt_CloseDisc,	"Closing disc",		    "LBL_DROPER_WRTCLOSEDISC"	},
    {	loc_DrOp_Wrt_Repair,	"Repairing disc",	    "LBL_DROPER_WRTREPAIRING"   },

    {	loc_DrOp_Blk_General,	"Erasing disc",		    "LBL_DROPER_BLANKING"	},
    {	loc_DrOp_Blk_CBlank,	"Blanking entire disc",	    "LBL_DROPER_BLKFULLBLANK"	},
    {	loc_DrOp_Blk_QBlank,	"Quick-blanking disc",	    "LBL_DROPER_BLKQUICKBLANK"	},
    {	loc_DrOp_Blk_SBlank,	"Blanking last session",    "LBL_DROPER_BLKSESSIONBLANK"},
    {	loc_DrOp_Blk_CFormat,	"Formatting entire disc",   "LBL_DROPER_BLKFULLFMT"	},
    {	loc_DrOp_Blk_QFormat,	"Quick-formatting disc",    "LBL_DROPER_BLKQUICKFMT"	},
    {	loc_DrOp_Blk_SFormat,	"Formatting new session",   "LBL_DROPER_BLKSESSIONFMT"  },

    {  loc_FailedToCreateApp,  
	"Failed to create application\nFryingPan will now close",   
	"REQ_APP_CREATE_FAILED"                                     },

    {  Localization::LocaleSet::Set_Last, 0, 0                     }
};

static const char* LocaleGroup = "MAIN";
static const int   LocaleVersion = 2;
/*
 * that's it :)
 */

int main()
{
    request("Information", "This module carries FryingPan interface.\nTo use it you need to launch main executable.", "Ok", 0);
    return -1;
}

GUI::GUI()
{
    _createDebug(true, "MUI.interface");

    _dx(Lvl_Info, "MUI.interface " __DATE__ "");
    pApp                 = 0;
    Config               = new Configuration("FryingPan");
    _dx(Lvl_Info, "Created configuration element (%08lx)", (iptr)Config);

    Glb.CurrentEngine    = new SyncProperty<IEngine*>();
    Glb.WriteSelection   = Globals::Select_Tracks;
    bCompact             = false;

    /* all the drive stuff goes here */
    drv_oper             = DRT_Operation_Unknown;
    drv_stat             = DRT_DriveStatus_Unknown;

    _dx(Lvl_Info, "Initializing libraries/modules");
    MUIMaster            = MUIMasterIFace::GetInstance(0);
    Datatypes            = DatatypesIFace::GetInstance(0);
    iicon                = IconIFace::GetInstance(0);

    _dx(Lvl_Info, "Loading engine plugin");
    {
	static TagItem gui_tags[] =
	{
	    { PLO_NameSpace,      (iptr)"FryingPan"	},
	    { PLO_PluginName,     (iptr)Engine_Name	},
	    { PLO_MinVersion,     Engine_Version	},
	    { PLO_MinRevision,    Engine_Revision	},
	    { 0,		  0			}
	};

	Glb.Engine = (EnginePlugin*)plug->OpenPlugin(gui_tags);
    }
    _dx(Lvl_Info, "Opened engine (%08lx), Open result: %ld", (iptr)Glb.Engine->IsValid());

    appobj               = iicon->GetDiskObject("FryingPan");
    _dx(Lvl_Info, "Opened application icon (%08lx)", (iptr)appobj);
    _dx(Lvl_Info, "Initializing hooks");

    /*
     * initialize all hooks
     */
    hShowHide.Initialize(this, &GUI::doShowHide);
    hPager.Initialize(this, &GUI::doChangePage);
    hEngineCmd.Initialize(this, &GUI::doEngineMessage);
    hEngineMsg.Initialize(this, &GUI::doEngineInternalMessage);
    hEngineChanged.Initialize(this, &GUI::doEngineChanged);
    hAction.Initialize(this, &GUI::doUserAction);

    /*
     * open engines
     */
    _dx(Lvl_Info, "Probing/setting engine");
    for (int i=0; i<4; i++)
	Glb.Engines[i]  = (*Glb.Engine)->GetEngine(i);

    Glb.CurrentEngine->Assign(Glb.Engines[0]);

    /*
     * open configuration
     */ 
    ASSERT(NULL != Config);
    if (NULL != Config)
    {
	_dx(Lvl_Info, "Reading configuration file");
	Config->readFile("ENV:FryingPan/Interface-MUI.prefs");
    }

    /*
     * here all objects are required to initialize:
     * - Localization strings
     * no object should initialize here:
     * - GUI elements
     */
    _dx(Lvl_Info, "Building localization map");
    Glb.Loc.AddGroup((Localization::LocaleSet*)&LocaleSets, LocaleGroup);

    _dx(Lvl_Info, "Creating GUI elements");
    Select      =  new MUIPageSelect(Config, Glb);
    DriveSelect =  new MUIDriveSelect(Config, Glb);
    Tracks      =  new MUITracksDataAudio(Config, Glb);
    Media       =  new MUIMedia(Config, Glb);
    Contents    =  new MUIContents(Config, Glb);
    Drive       =  new MUIDrive(Config, Glb);
    Settings    =  new MUISettings(Config, Glb);
    Record      =  new MUIRecord(Config, Glb);
    Action      =  new MUIPopAction(Glb);
    Menu        =  new MUIWindowMenu();

    _dx(Lvl_Info, "Reading locale");
    Glb.Loc.ReadCatalog("fryingpan.catalog", LocaleVersion);

    /*
     * starting here, we can use localization :)
     */
    _dx(Lvl_Info, "Setting up timer");
    pTimer      =  new Timer();
    pMsgPort    =  new Port(hEngineMsg.GetHook());
    Cfg         =  new ConfigParser(Config, "Main", 0);

    _dx(Lvl_Info, "Init complete");
}

GUI::~GUI()
{
    _dx(Lvl_Info, "Disposing individual modules");
    delete Record;
    delete Settings;
    delete Drive;
    delete Contents;
    delete Media;
    delete Tracks;
    delete DriveSelect;
    delete Select;
    delete Action;

    _dx(Lvl_Info, "Disposing configuration element");
    delete Cfg;

    if (NULL != Config)
    {
	_dx(Lvl_Info, "Creating configuration directories (if needed)");
	BPTR lock;
	lock = DOS->CreateDir("ENVARC:FryingPan");
	if (lock)
	    DOS->UnLock(lock);
	lock = DOS->CreateDir("ENV:FryingPan");
	if (lock)
	    DOS->UnLock(lock);

	_dx(Lvl_Info, "Recording configuration back to disk");

	Config->writeFile("ENVARC:FryingPan/Interface-MUI.prefs");
	Config->writeFile("ENV:FryingPan/Interface-MUI.prefs");

	_dx(Lvl_Info, "Disposing main configuration element");
	delete Config;
    }

    _dx(Lvl_Info, "Halting and disposing timer");
    delete pTimer;
    delete pMsgPort;

    _dx(Lvl_Info, "Disposing disk object");
    if (0 != appobj)
	iicon->FreeDiskObject(appobj);
    iicon->FreeInstance();

    _dx(Lvl_Info, "Disposing engine");
    Glb.Engine->Dispose();

    _dx(Lvl_Info, "Closing libraries");
    Datatypes->FreeInstance();
    MUIMaster->FreeInstance();

    _dx(Lvl_Info, "All done.");
    _destroyDebug();
}

bool GUI::start()
{
    uint32 signals;

    _dx(Lvl_Info, "Composing program menu");
    {  // Compose menu
	Menu->addMenu(Glb.Loc[loc_File]);
	Menu->addItem(Glb.Loc[loc_AboutMUI],      Action_AboutMUI,        Glb.Loc.Shortcut(loc_AboutMUI));
	Menu->addItem(Glb.Loc[loc_MUISettings],   Action_MUISettings,     Glb.Loc.Shortcut(loc_MUISettings));
	Menu->addSeparator();
#ifdef DEBUG
	Menu->addItem(Glb.Loc[loc_Translate],     Action_Translate,       Glb.Loc.Shortcut(loc_Translate));
	Menu->addSeparator();
#endif
	Menu->addItem(Glb.Loc[loc_Quit],          Action_Quit,            Glb.Loc.Shortcut(loc_Quit));
	Menu->setHook(hAction);
    }

    _dx(Lvl_Info, "Building main application");
    if (pApp == 0)
    {  // Create application object
	pApp = ApplicationObject,
	     MUIA_Application_SingleTask,  true,
	     MUIA_Application_Base,        "FryingPan",
	     MUIA_Application_Title,       "The Frying Pan",
	     MUIA_Application_DiskObject,  appobj,
	     SubWindow,                    pWin = WindowObject,
	     MUIA_Window_Title,            "FryingPan 1.3.2",
	     MUIA_Window_ID,               'MAIN',
	     MUIA_Window_Menustrip,        Menu->getObject(),
	     MUIA_Window_AppWindow,        true,
	     WindowContents,               VGroup,
	     Child,                        elements = VGroup,
	     Child,                        Select->getObject(),
	     Child,                        DriveSelect->getObject(),
	     Child,                        pages = PageGroup,
	     Child,                        Tracks->getObject(),
	     Child,                        Media->getObject(),
	     Child,                        Contents->getObject(),
	     Child,                        Drive->getObject(),
	     Child,                        Settings->getObject(),
	     Child,                        Record->getObject(),
	     End,
	     End,

	     Child,                        switchview = HGroup,
	     GroupFrameT(Glb.Loc[loc_DriveProgress].Data()),
	     Child,                        gauge = HGroup,
	     Child,                        muiGauge("...", ID_MainProgress),
	     MUIA_InputMode,               MUIV_InputMode_RelVerify,
	     MUIA_ShowSelState,            false,
	     End,

	     Child,                        Action->getObject(),
	     End,

	     End,
	     End,
	     End;
    }
    _dx(Lvl_Info, "Application built at %08lx", (iptr)pApp);

    /*
     * still not ctreated
     */
    if (pApp == 0)
    {
	request(Glb.Loc[Globals::loc_Error], Glb.Loc[loc_FailedToCreateApp], Glb.Loc[Globals::loc_OK], 0);
	return false;
    }
    else
    {
	DoMtd(pWin,  ARRAY(MUIM_Notify, MUIA_Window_CloseRequest, MUIV_EveryTime, (int)pApp, 2, MUIM_Application_ReturnID, (unsigned)MUIV_Application_ReturnID_Quit));
	DoMtd(gauge, ARRAY(MUIM_Notify, MUIA_Pressed,             false,          (int)pApp, 2, MUIM_CallHook,             (int)hShowHide.GetHook()));
	(*Glb.CurrentEngine) << hEngineChanged.GetHook();
	Select->setHook(hPager.GetHook());
    }

    _dx(Lvl_Info, "Spinning up elements");
    Select->start();
    Tracks->start();
    Media->start();
    Contents->start();
    Drive->start();
    Settings->start();
    Record->start();

    _dx(Lvl_Info, "*pop*");
    Intuition->SetAttrsA(pWin, TAGARRAY(
		MUIA_Window_Open, true,
		TAG_DONE,         0));

    _dx(Lvl_Info, "Registering to all handles");
    for (int i=0; i<4; i++)
	Glb.Engines[i]->registerHandler(hEngineCmd.GetHook());

    _dx(Lvl_Info, "Setting up timer (500ms)");
    pTimer->AddRequest(500);

    _dx(Lvl_Info, "Initial update");
    update();

    _dx(Lvl_Info, "Entering main loop");
    while (DoMtd(pApp, ARRAY(MUIM_Application_NewInput, (int)&signals)) != (unsigned)MUIV_Application_ReturnID_Quit)
    {
	if (signals)
	    signals = Exec->Wait(signals | pTimer->GetSignals() | pMsgPort->GetSignals());

	pMsgPort->HandleSignals();

	if (signals & pTimer->GetSignals())
	{
	    periodicUpdate();
	    pTimer->AddRequest(500);
	}
    }

    _dx(Lvl_Info, "Unregistering from all handles");
    for (int i=0; i<4; i++)
	Glb.Engines[i]->unregisterHandler(hEngineCmd.GetHook());

    _dx(Lvl_Info, "Unregistration complete");
    return true;
}

void GUI::stop()
{
    if (NULL == pApp)
	return;

    _dx(Lvl_Info, "Hiding main window");
    Intuition->SetAttrsA(pWin, TAGARRAY(
		MUIA_Window_Open, false,
		TAG_DONE,         0));

    _dx(Lvl_Info, "Deregistering property notifier");
    (*Glb.CurrentEngine) >> hEngineChanged.GetHook();

    _dx(Lvl_Info, "Halting individual modules");

    Select->stop();
    Tracks->stop();
    Media->stop();
    Contents->stop();
    Record->stop();
    Settings->stop();
    Drive->stop();

    /*
    ** early cleanup supposed to be in 'stop'
    ** late cleanup supposed to be in 'destroy'
    */
    _dx(Lvl_Info, "Disposing MUI application");
    MUIMaster->MUI_DisposeObject(pApp);
    pApp = 0;
}

void GUI::dispose()
{
    _dx(Lvl_Info, "Shutting down");
    delete this;
}

iptr GUI::doShowHide(void*, void*)
{
    _dx(Lvl_Info, "Switching UI");
    _dx(Lvl_Info, "Hiding main window");
    Intuition->SetAttrsA(pWin, TAGARRAY(
		MUIA_Window_Open,     false,
		TAG_DONE,             0));

    _dx(Lvl_Info, "Rearranging elements (new state: %ld)", !bCompact);
    Intuition->SetAttrsA(elements, TAGARRAY(
		MUIA_ShowMe,   (unsigned)bCompact,
		TAG_DONE,      0
		));

    bCompact = !bCompact;

    _dx(Lvl_Info, "Applying new ID");
    Intuition->SetAttrsA(pWin, TAGARRAY(
		MUIA_Window_ID,       bCompact ? 'MINI' : 'MAIN',
		TAG_DONE,             0));

    _dx(Lvl_Info, "Showing main window");
    Intuition->SetAttrsA(pWin, TAGARRAY(
		MUIA_Window_Open,     true,
		TAG_DONE,             0));

    _dx(Lvl_Info, "Done.");
    return 0;
}

iptr GUI::doChangePage(void*, iptr page)
{
    _dx(Lvl_Info, "Flipping page to %ld", page);
    Intuition->SetAttrsA(pages, (TagItem*)ARRAY(
		MUIA_Group_ActivePage,  page-1,
		TAG_DONE,               0
		));

    return 0;
}

iptr GUI::doEngineMessage(EngineMessage msg, IEngine* src)
{
    pMsgPort->DoSync(msg, src);
    return 0;
}

iptr GUI::doEngineInternalMessage(EngineMessage msg, IEngine* src)
{
    if (0 == pApp)
	return 0;

    IEngine *pEng = Glb.CurrentEngine->ObtainRead();
    _dx(Lvl_Info, "Received new engine message %ld", msg);

    if (pEng == src)
    {
	switch ((EngineMessage)msg)
	{
	    case Eng_DriveUpdate:
		{
		    _dx(Lvl_Info, "Drive Update!");
		    driveUpdate(pEng->obtainDriveUpdateTags());
		    pEng->releaseDriveUpdateTags();
		}
		break;

	    case Eng_Update:
		{
		    _dx(Lvl_Info, "Engine Update");
		    update();
		}
		break;

	    case Eng_JobStarted:
		{
		    _dx(Lvl_Info, "Engine Job started");
		    //IEngine *pEng = Glb.CurrentEngine->ObtainRead();
		    //muiSetText(ID_MainProgress, pEng->getActionName());
		    //Glb.CurrentEngine->Release();
		}
		break;

	    case Eng_JobFinished:
		{
		    _dx(Lvl_Info, "Engine Job complete");
		    //IEngine *pEng = Glb.CurrentEngine->ObtainRead();
		    //muiSetText(ID_MainProgress, pEng->getActionName());
		    //Glb.CurrentEngine->Release();
		}
		break;

	    case Eng_UpdateLayout:
		{
		    _dx(Lvl_Info, "Layout updated");
		    layoutUpdate();
		}
		break;
	};
    }

    _dx(Lvl_Info, "Message handling complete", msg);
    Glb.CurrentEngine->Release();
    return 0;
}

void GUI::update()
{
    _dx(Lvl_Info, "Updating all elements");
    periodicUpdate();
    Drive->update();
    Media->update();
    Contents->update();
    Tracks->update();
    Settings->update();
    Record->update();
    Action->update();
    _dx(Lvl_Info, "Update complete");
}

void GUI::periodicUpdate()
{
    IEngine *eng = Glb.CurrentEngine->ObtainRead();
    if (NULL != eng)
    {
	muiSetValue(ID_MainProgress, eng->getActionProgress());
	//muiSetText(ID_MainProgress, eng->getActionName());
    }
    Glb.CurrentEngine->Release();
}

void GUI::layoutUpdate()
{
    //Tracks->layoutUpdate();
}

iptr GUI::doEngineChanged(IEngine *old, IEngine *current)
{
    _dx(Lvl_Info, "Switching engine from %08lx to %08lx. Updating", (iptr)old, (iptr)current);
    update();
    return 0;
}

void GUI::setDebug(DbgHandler *h)
{
    Glb.dbg = h;
}

DbgHandler *GUI::getDebug()
{
    return Glb.dbg;
}

iptr GUI::doUserAction(ActionID act, void*)
{
    switch (act)
    {
	case Action_AboutMUI:
	    DoMtd(pApp, ARRAY(MUIM_Application_AboutMUI, (iptr)pWin));
	    break;

	case Action_MUISettings:
	    DoMtd(pApp, ARRAY(MUIM_Application_OpenConfigWindow, 0));
	    break;

	case Action_Translate:
#warning give user some neat request here!!!!
	    Glb.Loc.ExportCD("RAM:fryingpan.cd", LocaleVersion);
	    Glb.Loc.ExportCT("RAM:fryingpan.ct", LocaleVersion);
	    break;

	case Action_Quit:
	    DoMtd(pApp, ARRAY(MUIM_Application_ReturnID, (iptr)MUIV_Application_ReturnID_Quit));
	    break;
    }

    return 0;
}

/*
 * so i'm back mixing the interface with drive directly.
 * all things considered, i think i shoud've done it this way right in the beginning.
 * engine may be completely separated from the interface as it is now, but apparently
 * this feature has no fans ;-). i will put the engine in the optical guts soon enough.
 */
void GUI::driveUpdate(const TagItem* tags)
{
    DRT_DriveStatus stat;
    DRT_Operation oper;
    const char *s1, *s2;

    stat = (DRT_DriveStatus)Utility->GetTagData(DRA_Drive_Status, drv_stat, tags);
    oper = (DRT_Operation)  Utility->GetTagData(DRA_Drive_CurrentOperation, drv_oper, tags);

    if ((stat == drv_stat) && (oper == drv_oper))
	return;

    drv_stat = stat;
    drv_oper = oper; 

    switch (stat)
    {
	case DRT_DriveStatus_Unknown:	    s1 = Glb.Loc[loc_DrSt_Unknown];	    break;
	case DRT_DriveStatus_NotOpened:	    s1 = Glb.Loc[loc_DrSt_NotOpened];	    break;
	case DRT_DriveStatus_Ready:	    s1 = Glb.Loc[loc_DrSt_Ready];	    break;
	case DRT_DriveStatus_NoDisc:	    s1 = Glb.Loc[loc_DrSt_NoDisc];	    break;
	case DRT_DriveStatus_Busy:	    s1 = Glb.Loc[loc_DrSt_Busy];	    break;
	default:			    s1 = "???";				    break;
    }

    switch (oper)
    {
	case DRT_Operation_Unknown:         s2 = Glb.Loc[loc_DrOp_Unknown]; break;
	case DRT_Operation_NoOperation:     s2 = Glb.Loc[loc_DrOp_NoOperation]; break;
	case DRT_Operation_Play:            s2 = Glb.Loc[loc_DrOp_Play]; break;
	case DRT_Operation_Pause:           s2 = Glb.Loc[loc_DrOp_Pause]; break;
	case DRT_Operation_Read:            s2 = Glb.Loc[loc_DrOp_Read]; break;

					    /* :-) */
	case DRT_Operation_Control_Eject:   s2 = Glb.Loc[loc_DrOp_Ctl_Eject]; break;
	case DRT_Operation_Control_Load:    s2 = Glb.Loc[loc_DrOp_Ctl_Load]; break;
	case DRT_Operation_Control_SpinUp:  s2 = Glb.Loc[loc_DrOp_Ctl_SpinUp]; break;
	case DRT_Operation_Control_SpinDown:s2 = Glb.Loc[loc_DrOp_Ctl_SpinDown]; break;
	case DRT_Operation_Control_Standby: s2 = Glb.Loc[loc_DrOp_Ctl_Standby]; break;

					    /* :-) */
	case DRT_Operation_Analyse_General:	    s2 = Glb.Loc[loc_DrOp_Als_General]; break;
	case DRT_Operation_Analyze_Drive:	    s2 = Glb.Loc[loc_DrOp_Als_Drive]; break;
	case DRT_Operation_Analyse_Tracks:	    s2 = Glb.Loc[loc_DrOp_Als_Tracks]; break;
	case DRT_Operation_Analyse_Indices:	    s2 = Glb.Loc[loc_DrOp_Als_Indices]; break;
	case DRT_Operation_Analyse_DataType:	    s2 = Glb.Loc[loc_DrOp_Als_DataType]; break;
	case DRT_Operation_Analyse_CDText:	    s2 = Glb.Loc[loc_DrOp_Als_CDText]; break;
	case DRT_Operation_Analyse_FreeDB:	    s2 = Glb.Loc[loc_DrOp_Als_FreeDB]; break;
	case DRT_Operation_Analyse_DiscID:	    s2 = Glb.Loc[loc_DrOp_Als_DiscID]; break;
	case DRT_Operation_Analyse_ControlBlocks:   s2 = Glb.Loc[loc_DrOp_Als_DCB]; break;
	case DRT_Operation_Analyse_Speeds:	    s2 = Glb.Loc[loc_DrOp_Als_Speeds]; break;
	case DRT_Operation_Analyse_Layout:	    s2 = Glb.Loc[loc_DrOp_Als_Layout]; break;

	case DRT_Operation_Write_General:	    s2 = Glb.Loc[loc_DrOp_Wrt_General]; break;
	case DRT_Operation_Write_Calibrate:	    s2 = Glb.Loc[loc_DrOp_Wrt_Calibrate]; break;
	case DRT_Operation_Write_Allocate:	    s2 = Glb.Loc[loc_DrOp_Wrt_Allocate]; break;
	case DRT_Operation_Write_CDText:	    s2 = Glb.Loc[loc_DrOp_Wrt_CDText]; break;
	case DRT_Operation_Write_Data:		    s2 = Glb.Loc[loc_DrOp_Wrt_Data]; break;
	case DRT_Operation_Write_Synchronize:	    s2 = Glb.Loc[loc_DrOp_Wrt_Sync]; break;
	case DRT_Operation_Write_CloseTrack:	    s2 = Glb.Loc[loc_DrOp_Wrt_CloseTrk]; break;
	case DRT_Operation_Write_CloseSession:	    s2 = Glb.Loc[loc_DrOp_Wrt_CloseSess]; break;
	case DRT_Operation_Write_CloseDisc:	    s2 = Glb.Loc[loc_DrOp_Wrt_CloseDisc]; break;
	case DRT_Operation_Write_Repair:	    s2 = Glb.Loc[loc_DrOp_Wrt_Repair]; break;

	case DRT_Operation_Erase_General:	    s2 = Glb.Loc[loc_DrOp_Blk_General]; break;
	case DRT_Operation_Erase_BlankComplete:	    s2 = Glb.Loc[loc_DrOp_Blk_CBlank]; break;
	case DRT_Operation_Erase_BlankFast:	    s2 = Glb.Loc[loc_DrOp_Blk_QBlank]; break;
	case DRT_Operation_Erase_BlankSession:	    s2 = Glb.Loc[loc_DrOp_Blk_SBlank]; break;
	case DRT_Operation_Erase_FormatComplete:    s2 = Glb.Loc[loc_DrOp_Blk_CFormat]; break;
	case DRT_Operation_Erase_FormatFast:	    s2 = Glb.Loc[loc_DrOp_Blk_QFormat]; break;
	case DRT_Operation_Erase_FormatSession:	    s2 = Glb.Loc[loc_DrOp_Blk_SFormat]; break;

	default:				    s2 = "???";
    }

    statusText = s1;
    if (s2 != 0)
	statusText = statusText + " (" + s2 + ")";

    muiSetText(ID_MainProgress, statusText);
}

