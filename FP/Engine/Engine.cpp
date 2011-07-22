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


#include "Engine.h"
#include "Globals.h"
#include <libclass/dos.h>
#include <Generic/Thread.h>
#include "Jobs/Job.h"
#include "Track.h"
#include "Event.h"

#include "Jobs/JobControl.h"
#include "Jobs/JobMediumAction.h"
#include "Jobs/JobDownload.h"
#include "Jobs/JobUpload.h"
#include "Jobs/JobUploadSession.h"
#include "Jobs/JobUpdate.h"
#include "Jobs/JobCreateISO.h"
#include "Jobs/JobLayout.h"
#include "Jobs/JobWriteProtect.h"
#include <utility/tagitem.h>

#include <dos/filehandler.h>
#include <dos/dosextens.h>
#include <Generic/XMLDocument.h>

static const char      *Cfg_DriveName        = "DriveName";
static const char      *Cfg_Device           = "Device";
static const char      *Cfg_Unit             = "Unit";
static const char      *Cfg_DOSDevice        = "DOSDevice";
static const char      *Cfg_DOSInhibit       = "DOSInhibit";

#if 0
static const char      *Cfg_ISOBuilder       = "ISOBuilder";
static const char      *Cfg_ISODiscName      = "DiscName";
static const char      *Cfg_ISOSystemID      = "SystemID";
static const char      *Cfg_ISOVolSetID      = "VolumeSetID";
static const char      *Cfg_ISOPreparerID    = "PreparerID";
static const char      *Cfg_ISOPublisherID   = "PublisherID";
static const char      *Cfg_ISOApplicationID = "ApplicationID";
#endif

static const char      *Cfg_DataExport       = "DataExportHook";
static const char      *Cfg_AudioExport      = "AudioExportHook";
static const char      *Cfg_SessionExport    = "SessionExportHook";

Engine::Engine(Globals *glb, int which) :
    g(*glb)
{
    _createDebug(1, "Engine");

    _dx(Lvl_Info, "Initializing");
    hLayoutStatus	= "";
    hLayoutSize		= 0;

    pImportedSession	= 0;
    pDriveUpdateTags.Assign(0);

    bErasable		= false;
    bFormatable		= false;
    bRecordable		= false;
    bOverwritable	= false;
    bWriteProtectable	= false;
    bWriteProtected	= false;

    Drive.Assign(0);
    hJob.Assign(0);
    sDriveName.FormatStr("Drive %ld", ARRAY(which));

    _dx(Lvl_Info, "Reading export hooks");

    if (g.Optical.IsValid())
    {
	_dx(Lvl_Info, "Building Disc Layout");
	pImportedSession = g.Optical->CreateDisc();
    }

    _dx(Lvl_Info, "Creating config parser");
    Config = new ConfigParser(&g.Config, "Drive", which);

    _dx(Lvl_Info, "Reading configuration");
    readSettings();

    _dx(Lvl_Info, "Initializing hooks");
    hHkThrOperations.Initialize(this, &Engine::mainOperations);
    hHkHndOperations.Initialize(this, &Engine::hndlOperations);
    hHkDriveUpdate.Initialize(this, &Engine::onDriveUpdate);

    _dx(Lvl_Info, "Starting threads");
    thrOperations = new Thread("Engine Ops", hHkThrOperations.GetHook(), 0);
    thrOperations->SetHandler(hHkHndOperations.GetHook());

    _dx(Lvl_Info, "Opening device");
    openDevice(sDevice, lUnit);

    _dx(Lvl_Info, "Initialization complete");
}

Engine::~Engine()
{
    _dx(Lvl_Info, "Finalizing");
    const TagItem *tags = pDriveUpdateTags.Assign(0);

    _dx(Lvl_Info, "Freeing tags");
    if (0 != tags)
	Utility->FreeTagItems(tags);

    VectorT<IData*> &vecTracks = tracks().ObtainWrite();

    _dx(Lvl_Info, "Shutting down tasks");
    delete thrOperations;

    _dx(Lvl_Info, "Updating configuration");
    writeSettings();  // writes data from iso image. need to be before;

    /*
    ** note: the code below automagically disposes our iso
    ** so we don't need to dispose it again.
    */
    _dx(Lvl_Info, "Freeing tracks");
    while (vecTracks.Count() > 0)
    {
	vecTracks[0]->dispose();
	vecTracks >> vecTracks[0];
    }
    tracks().Release();

    _dx(Lvl_Info, "Disposing session");
    if (pImportedSession != 0)
	pImportedSession->dispose();
    pImportedSession = 0;

    _dx(Lvl_Info, "Disposing session reader");
    if (pSessImportReader != 0)
	pSessImportReader->dispose();

    _dx(Lvl_Info, "Closing device");
    closeDevice();

    _dx(Lvl_Info, "Disposing configuration");
    delete Config;

    _dx(Lvl_Info, "Bye.");
    _destroyDebug();
}

iptr Engine::mainOperations(Thread* thread, void*)
{
    iptr           did = ~0;
    iptr           lid = ~0;
    iptr  drv = 0;

    do
    {
	if (g.Optical.IsValid())
	{
	    drv = Drive.ObtainRead();
	    if (drv != 0)
	    {
		g.Optical->DoMethodA(ARRAY(DRV_GetAttrs, drv, 
			    DRA_Disc_CurrentDiscSeqNum,   (iptr)&did,
			    TAG_DONE,                     0));
		if (did != lid)
		    do_update();
		lid = did;
	    }
	    Drive.Release();
	}
    }
    while (false == thread->HandleSignals(1000));

    return 0;
}

iptr Engine::hndlOperations(Command cmd, Event* ev)
{
    if (cmd == Cmd_NoOperation)
	return 0;

    switch (ev->getEventType())
    {
	case Event::Ev_Job:
	    {
		Job     *job      = ev->getJob();
		String   dosdev   = sDOSDevice;
		bool     inhibit  = bDOSInhibit;

		// if user marked device for inhibition, try to obtain it
		// and remember if we succeeded.
		if (inhibit & job->inhibitDOS())
		    inhibit = setInhibited(dosdev, true);

		hJob.Assign(job);
		sJobName = job->getActionName();
		notify(Eng_JobStarted);
		job->execute();
		sJobName.Assign("Ready");
		hJob.Assign(0);
		notify(Eng_JobFinished);

		if (inhibit & job->inhibitDOS())
		    setInhibited(dosdev, false);
	    }
	    break;
	case Event::Ev_Notify:
	    {
		notify(ev->getMessage());
	    }
	    break;
	case Event::Ev_Action:
	    {
		switch (ev->getAction())
		{
		    case Cmd_Update:
			do_update();
			break;

		    case Cmd_SetWriteSpeed:
			do_setwritespeed();
			break;

		    case Cmd_SetReadSpeed:
			do_setreadspeed();
			break;

		    case Cmd_NoOperation:
		    case Cmd_ExecuteJob:
		    case Cmd_SendEvent:
			break;

		    case Cmd_UpdateLayout:
			do_layoutupdate();
			break;
		}
	    }
	    break;
    };
    delete ev;
    return 0;
}

iptr Engine::onDriveUpdate(iptr drive, const TagItem* tags)
{
    /*
     * this is executed from drive task
     */
    /*
     * TODO: values reported by drive prior to app hook registration are lost.
     */
    const TagItem *t = pDriveUpdateTags.Assign(Utility->CloneTagItems(tags));
    if (0 != t)
	Utility->FreeTagItems(t);

    notify(Eng_DriveUpdate);
    return 0;
}

bool Engine::setInhibited(String& device, bool state)
{
    String               s;
    DeviceNode          *dn;

    dn = (DeviceNode*)DOS->LockDosList(LDF_DEVICES | LDF_READ);

    while (NULL != (dn = (DeviceNode*)DOS->NextDosEntry((DosList*)dn, LDF_DEVICES)))
    {
	if ((int)dn->dn_Startup > 1024)
	{
	    s.BstrCpy(dn->dn_Name);
	    if (s == device)
		break;
	}
    }

    DOS->UnLockDosList(LDF_DEVICES);

    if (dn == 0)
	return false;

    s += ":";
    DOS->Inhibit(s.Data(), state);

    return state;
}

void Engine::readSettings()
{
#warning "ISO settings do not work."
#if 0
    ConfigParser *ciso = new ConfigParser(Config, Cfg_ISOBuilder);

    if (iso != 0)
    {
	_dx(Lvl_Info, "Reading ISO configuration");
	iso->getRoot()->setVolumeID(ciso->getValue(Cfg_ISODiscName,             "CDROM"));
	iso->getRoot()->setSystemID(ciso->getValue(Cfg_ISOSystemID,             "AMIGA"));
	iso->getRoot()->setVolumeSetID(ciso->getValue(Cfg_ISOVolSetID,          ""));
	iso->getRoot()->setPreparerID(ciso->getValue(Cfg_ISOPreparerID,         ""));
	iso->getRoot()->setPublisherID(ciso->getValue(Cfg_ISOPublisherID,       ""));
	iso->getRoot()->setApplicationID(ciso->getValue(Cfg_ISOApplicationID,   "THE FRYING PAN - AMIGA CD/DVD MASTERING SOFTWARE"));
    }
    delete ciso;
#endif

    _dx(Lvl_Info, "Reading Engine configuration");
    sDriveName        = Config->getValue(Cfg_DriveName,      sDriveName); 
    sDevice           = Config->getValue(Cfg_Device,         "");
    lUnit             = Config->getValue(Cfg_Unit,           (iptr)0);
    sDOSDevice        = Config->getValue(Cfg_DOSDevice,      "");
    bDOSInhibit       = Config->getValue(Cfg_DOSInhibit,     (iptr)false);
}

void Engine::writeSettings()
{
    ASSERT(NULL != Config);

    if (NULL == Config)
	return;

    Config->setValue(Cfg_DriveName,        sDriveName);
    Config->setValue(Cfg_Device,           sDevice);
    Config->setValue(Cfg_Unit,             lUnit);

    Config->setValue(Cfg_DOSDevice,        sDOSDevice);
    Config->setValue(Cfg_DOSInhibit,       bDOSInhibit);

#warning "ISO settings do not work."
#if 0
    if (0 != iso)
    {
	ConfigParser *ciso = new ConfigParser(Config, Cfg_ISOBuilder);

	ciso->setValue(Cfg_ISODiscName,      iso->getRoot()->getVolumeID());
	ciso->setValue(Cfg_ISOSystemID,      iso->getRoot()->getSystemID());
	ciso->setValue(Cfg_ISOVolSetID,      iso->getRoot()->getVolumeSetID());
	ciso->setValue(Cfg_ISOPreparerID,    iso->getRoot()->getPreparerID());
	ciso->setValue(Cfg_ISOPublisherID,   iso->getRoot()->getPublisherID());
	ciso->setValue(Cfg_ISOApplicationID, iso->getRoot()->getApplicationID());

	delete ciso;
    }
#endif
}

void Engine::registerHandler(const Hook* h)
{ 
    DistributorT<EngineMessage, IEngine*> &hx = handlers.ObtainWrite();
    hx << h;
    handlers.Release();
}

void Engine::unregisterHandler(const Hook* h)
{
    DistributorT<EngineMessage, IEngine*> &hx = handlers.ObtainWrite();
    hx >> h;
    handlers.Release();
}

void Engine::do_update()
{
    iptr     ret;
    iptr     drv;

    drv = Drive.ObtainRead();

    sDriveVendor   = "";
    sDriveProduct  = "";
    sDriveVersion  = "";
    pCurrentDisc = 0;

    //MessageBox("Info", "reading drive info", "ok", 0);
    if (drv != 0)
    {
	ret = g.Optical->DoMethodA(ARRAY(DRV_GetAttr, drv, DRA_Drive_IsDiscInserted));
	bDiscPresent   = ret ? true : false;
	ret = g.Optical->DoMethodA(ARRAY(DRV_GetAttr, drv, DRA_Drive_Vendor));
	sDriveVendor   = (const char*)ret;
	ret = g.Optical->DoMethodA(ARRAY(DRV_GetAttr, drv, DRA_Drive_Product));
	sDriveProduct  = (const char*)ret;
	ret = g.Optical->DoMethodA(ARRAY(DRV_GetAttr, drv, DRA_Drive_Version));
	sDriveVersion  = (const char*)ret;
	ret = g.Optical->DoMethodA(ARRAY(DRV_GetAttr, drv, DRA_Drive_MechanismType));
	eMechanismType = (DRT_Mechanism)ret;
	ret = g.Optical->DoMethodA(ARRAY(DRV_GetAttr, drv, DRA_Drive_ReadsMedia));
	eReadableMedia = ret;
	ret = g.Optical->DoMethodA(ARRAY(DRV_GetAttr, drv, DRA_Drive_WritesMedia));
	eWritableMedia = ret;
	ret = g.Optical->DoMethodA(ARRAY(DRV_GetAttr, drv, DRA_Drive_Capabilities));
	eCapabilities  = ret;
	ret = g.Optical->DoMethodA(ARRAY(DRV_GetAttr, drv, DRA_Drive_CurrentProfile));
	eCurrentProfile = (DRT_Profile)ret;
	ret = g.Optical->DoMethodA(ARRAY(DRV_GetAttr, drv, DRA_Drive_IsRegistered));
	bRegistered = ret ? true : false;
	ret = g.Optical->DoMethodA(ARRAY(DRV_GetAttr, drv, DRA_Disc_SubType));
	eDiscSubType = (DRT_SubType)ret;
	ret = g.Optical->DoMethodA(ARRAY(DRV_GetAttr, drv, DRA_Disc_Vendor));
	sVendor = (char*)ret;

	readSpeeds  = (DiscSpeed*)g.Optical->DoMethodA(ARRAY(DRV_GetAttr, drv, DRA_Drive_ReadSpeeds));
	writeSpeeds = (DiscSpeed*)g.Optical->DoMethodA(ARRAY(DRV_GetAttr, drv, DRA_Drive_WriteSpeeds));
	readSpeed = g.Optical->DoMethodA(ARRAY(DRV_GetAttr, drv, DRA_Drive_CurrentReadSpeed));
	writeSpeed = g.Optical->DoMethodA(ARRAY(DRV_GetAttr, drv, DRA_Drive_CurrentWriteSpeed));
    }
    Drive.Release();

    //MessageBox("Info", "reading drive contents", "ok", 0);
    readContents();

    //MessageBox("Info", "setting contents", "ok", 0);
    notify(Eng_Update);

    //MessageBox("Info", "writing settings", "ok", 0);
    writeSettings();

    //MessageBox("Info", "all done", "ok", 0);
}

void Engine::do_layoutupdate()
{
    String res;
    VectorT< IData* > &v = tracks().ObtainRead();
    iptr cnt = v.Count();
    tracks().Release();

    hLayoutSize = 0;
    hLayoutStatus = "Nothing to write.";

    if (cnt > 0)
    {
	iptr drive = Drive.ObtainRead();
	if (0 != drive)
	{
	    // we do not want to run this job in background.
	    JobLayout *job = new JobLayout(g, drive, tracks(), false, false, hLayoutStatus, hLayoutSize);

	    /*
	     * append operations to the end of the list ;)
	     * this way we're sure all events modifying the iso image or things are complete.
	     */        
	    thrOperations->DoAsync(Cmd_ExecuteJob, new Event(job));
	}
	Drive.Release();
    }

    thrOperations->DoAsync(Cmd_SendEvent, new Event(Eng_UpdateLayout));
}

void Engine::readContents()
{
    iptr     drv;
    const IOptItem   *items = pCurrentDisc;

    drv = Drive.ObtainRead();
    if (drv == 0)
    {
	Drive.Release();
	return;
    }

    pCurrentDisc = (const IOptItem*)g.Optical->DoMethodA(ARRAY(DRV_GetAttr, drv, DRA_Disc_Contents));

    {
	int32 rec, era, fmt, ovw, wp1, wp2;

	g.Optical->DoMethodA(ARRAY(DRV_GetAttrs, drv, 
		    DRA_Disc_IsWritable,    (int)&rec, 
		    DRA_Disc_IsErasable,    (int)&era, 
		    DRA_Disc_IsFormatable,  (int)&fmt,
		    DRA_Disc_IsOverwritable,(int)&ovw,
		    DRA_Disc_Size,          (int)&lDiscSize,
		    DRA_Disc_AllowsWriteProtect, (int)&wp1,
		    DRA_Disc_WriteProtect,  (int)&wp2,
		    TAG_DONE,               0));

	bRecordable    = rec ? true : false;
	bErasable      = era ? true : false;
	bFormatable    = fmt ? true : false;
	bOverwritable  = ovw ? true : false;
	bWriteProtectable = wp1 ? true : false;
	bWriteProtected = wp2 ? true : false;
    }

    if (pCurrentDisc != 0)
	pCurrentDisc->claim();
    if (items != 0)
	items->dispose();
    Drive.Release();
    return;
}

void Engine::notify(EngineMessage msg)
{
    DistributorT<EngineMessage, IEngine*> &h = handlers.ObtainRead();
    h.Send(msg, this);
    handlers.Release();
}

const char *Engine::getName()
{
    return sDriveName.Data();
}

void Engine::setName(const char* name)
{
    sDriveName = name;
}

const char *Engine::getDevice()
{
    return sDevice.Data();
}

int Engine::getUnit()
{
    return lUnit;
}

bool Engine::openDevice(const char* name, int lun)
{
    iptr pdrv = 0;

    ASSERT(g.Optical.IsValid());

    if (!g.Optical.IsValid())
	return false;

    pdrv = Drive.ObtainRead();
    Drive.Release();


    // 
    // only if we have changed device or unit OR if we dont have the drive yet
    // like, if the previous try failed;
    //
    if ((lUnit   != lun)  ||
	    (sDevice != name) ||
	    (pdrv    == 0))
    {
	closeDevice();

	if (strlen(name) == 0)
	    return false;

	pdrv = g.Optical->DoMethodA(ARRAY(DRV_NewDrive, (int)name, lun));
	g.Optical->DoMethodA(ARRAY(DRV_SetAttr, pdrv, DRA_AddTagNotify, (iptr)hHkDriveUpdate.GetHook(), 0));

	Drive.Assign(pdrv);
    }

    if (pdrv != 0)
    {
	sDevice  = name;
	lUnit    = lun;
	update();
	return true;
    }
    else
    {
	sDevice  = "";
	lUnit    = 0;
	return false;
    }
}

void Engine::closeDevice()
{
    iptr pdrv;
    ASSERT(g.Optical.IsValid());

    if (!g.Optical.IsValid())
	return;

    pdrv = Drive.Assign(0);

    if (pdrv != 0)
    {
	sDevice  = "";
	lUnit    = 0;
	g.Optical->DoMethodA(ARRAY(DRV_SetAttrs, pdrv, DRA_RemTagNotify, (iptr)hHkDriveUpdate.GetHook(), 0));
	g.Optical->DoMethodA(ARRAY(DRV_EndDrive, pdrv));
    }
}

bool Engine::isOpened()
{
    iptr drv;
    drv = Drive.ObtainRead();
    Drive.Release();
    return (drv != 0) ? true : false;
}

const char *Engine::getDriveVendor()
{
    if (isOpened())
	return sDriveVendor.Data();
    return "";
}

const char *Engine::getDriveProduct()
{
    if (isOpened())
	return sDriveProduct.Data();
    return "";
}

const char *Engine::getDriveVersion()
{
    if (isOpened())
	return sDriveVersion.Data();
    return "";
}

DRT_Mechanism Engine::getMechanismType()
{
    if (isOpened())
	return eMechanismType;
    return DRT_Mechanism_Unknown;
}

iptr Engine::getReadableMedia()
{
    if (isOpened())
	return eReadableMedia;
    return 0;
}

iptr Engine::getWritableMedia()
{
    if (isOpened())
	return eWritableMedia;
    return 0;
}

iptr Engine::getCapabilities()
{ 
    if (isOpened())
	return eCapabilities;
    return 0;
}

DiscSpeed *Engine::getReadSpeeds()
{
    if (isOpened())
	return readSpeeds;
    return 0;
}

DiscSpeed *Engine::getWriteSpeeds()
{
    if (isOpened())
	return writeSpeeds; 
    return 0;
}

uint16 Engine::getWriteSpeed()
{
    return writeSpeed;
}

uint16 Engine::getReadSpeed()
{
    return readSpeed;
}

void Engine::setWriteSpeed(uint16 speed)
{
    writeSpeed = speed;
    Event *e = new Event(Cmd_SetWriteSpeed);
    thrOperations->DoAsync(Cmd_Update, e);
}

void Engine::setReadSpeed(uint16 speed)
{
    readSpeed = speed;
    Event *e = new Event(Cmd_SetReadSpeed);
    thrOperations->DoAsync(Cmd_Update, e);
}

void Engine::do_setwritespeed()
{
    uint32 drv = Drive.ObtainRead();

    if (drv != 0)
    {
	g.Optical->DoMethodA(ARRAY(DRV_SetAttr, drv, DRA_Drive_CurrentWriteSpeed, writeSpeed, 0));
	writeSpeed = g.Optical->DoMethodA(ARRAY(DRV_GetAttr, drv, DRA_Drive_CurrentWriteSpeed));
    }

    Drive.Release();
}

void Engine::do_setreadspeed()
{
    uint32 drv = Drive.ObtainRead();

    if (drv != 0)
    {
	g.Optical->DoMethodA(ARRAY(DRV_SetAttr, drv, DRA_Drive_CurrentReadSpeed, readSpeed, 0));
	readSpeed = g.Optical->DoMethodA(ARRAY(DRV_GetAttr, drv, DRA_Drive_CurrentReadSpeed));
    }

    Drive.Release();
}

bool Engine::isDiscInserted()
{
    return bDiscPresent;
}

bool Engine::isDVD()
{
    if (!isOpened())
	return false; 

    switch (getDiscType())
    {
	case DRT_Profile_DVD_ROM:
	case DRT_Profile_DVD_MinusR:
	case DRT_Profile_DVD_PlusR:
	case DRT_Profile_DVD_MinusRW_Sequential:
	case DRT_Profile_DVD_MinusRW_Restricted:
	case DRT_Profile_DVD_PlusRW:
	case DRT_Profile_DVD_RAM:
	    return true;
	default:
	    return false;
    }
}

bool Engine::isCD()
{
    if (!isOpened())
	return false;

    switch (getDiscType())
    {
	case DRT_Profile_CD_ROM:
	case DRT_Profile_CD_R:
	case DRT_Profile_CD_RW:
	case DRT_Profile_CD_MRW:
	case DRT_Profile_DDCD_ROM:
	case DRT_Profile_DDCD_R:
	case DRT_Profile_DDCD_RW:
	    return true;
	default:
	    return false;
    }
}

bool Engine::isBD()
{
    if (!isOpened())
	return false;

    switch (getDiscType())
    {
	case DRT_Profile_BD_ROM:
	case DRT_Profile_BD_R_Sequential:
	case DRT_Profile_BD_R_RandomWrite:
	case DRT_Profile_BD_RW:
	    return true;
	default:
	    return false;
    }
}

bool Engine::isRecordable()
{
    return bRecordable;
}

bool Engine::isErasable()
{
    return bErasable;
}

bool Engine::isFormatable()
{
    return bFormatable;
}

bool Engine::isOverwritable()
{
    return bOverwritable;
}

DRT_Profile Engine::getDiscType()
{
    if (isOpened())
	return eCurrentProfile;
    return DRT_Profile_NoDisc;
}

bool Engine::isRegistered()
{
    if (isOpened())
	return bRegistered;
    return false;              
}

const IOptItem *Engine::getContents()
{
    if (isOpened())
	return pCurrentDisc;
    return 0;
}

void Engine::layoutTracks(bool masterize, bool closedisc)
{
    thrOperations->DoAsync(Cmd_UpdateLayout, new Event(Cmd_UpdateLayout));
}

DRT_SubType Engine::getDiscSubType()
{
    return eDiscSubType;
}

const char *Engine::getDiscVendor()
{
    return sVendor.Data();
}

void Engine::setDOSDevice(const char* dev)
{
    sDOSDevice = dev;
}

const char *Engine::getDOSDevice()
{
    return sDOSDevice.Data();
}

void Engine::setDOSInhibit(bool flag)
{
    bDOSInhibit = flag;
}

bool Engine::getDOSInhibit()
{
    return bDOSInhibit;
}

const char* Engine::findMatchingDOSDevice()
{
    String               s;
    String               d;
    DeviceNode          *dn;
    FileSysStartupMsg   *fs;

    s.AllocBuf(128);

    dn = (DeviceNode*)DOS->LockDosList(LDF_DEVICES | LDF_READ);


    while (NULL != (dn = (DeviceNode*)DOS->NextDosEntry((DosList*)dn, LDF_DEVICES)))
    {
	if ((int)dn->dn_Startup > 1024)
	{
	    fs = (FileSysStartupMsg*)((int)dn->dn_Startup << 2);
	    if ((fs != 0) && (fs->fssm_Device))
	    {
		s.BstrCpy(dn->dn_Name);
		d.BstrCpy(fs->fssm_Device);
		if (d == sDevice)
		{
		    if ((int32)fs->fssm_Unit == lUnit)
		    {
			setDOSDevice(s.Data());
			break;
		    }
		}
	    }
	}
    }

    DOS->UnLockDosList(LDF_DEVICES);

    return getDOSDevice();
}

/*
 * device commands go here
 */

void Engine::update()
{
    iptr drv = Drive.ObtainRead();

    if (drv != 0)
    {
	Event *e = new Event(Cmd_Update);
	thrOperations->DoAsync(Cmd_Update, e);
    }

    Drive.Release();
}

void Engine::ejectTray()
{
    iptr drv = Drive.ObtainRead();

    if (drv != 0)
    {
	Event *e = new Event(new JobControl(g, drv, DRT_Unit_Eject));
	thrOperations->DoAsync(Cmd_ExecuteJob, e);
    }

    Drive.Release();
}

void Engine::loadTray()
{
    iptr drv = Drive.ObtainRead();

    if (drv != 0)
    {
	Event *e = new Event(new JobControl(g, drv, DRT_Unit_Load));
	thrOperations->DoAsync(Cmd_ExecuteJob, e);
    }

    Drive.Release();
}

const char *Engine::getActionName()
{
    return sJobName.Data();
}

const char *Engine::getLayoutInfo()
{
    return hLayoutStatus.Data();
}

void Engine::closeSession()
{
    iptr drv = Drive.ObtainRead();

    if (drv != 0)
    {
	Event *e = new Event(new JobMediumAction(g, drv, JobMediumAction::Act_CloseSession));
	thrOperations->DoAsync(Cmd_ExecuteJob, e);
    }

    Drive.Release();
}

void Engine::closeDisc()
{
    iptr drv = Drive.ObtainRead();

    if (drv != 0)
    {
	Event *e = new Event(new JobMediumAction(g, drv, JobMediumAction::Act_CloseDisc));
	thrOperations->DoAsync(Cmd_ExecuteJob, e);
    }

    Drive.Release();
}

void Engine::closeTracks()
{
    iptr drv = Drive.ObtainRead();

    if (drv != 0)
    {
	Event *e = new Event(new JobMediumAction(g, drv, JobMediumAction::Act_CloseTracks));
	thrOperations->DoAsync(Cmd_ExecuteJob, e);
    }

    Drive.Release();
}

void Engine::repairDisc()
{
    iptr drv = Drive.ObtainRead();

    if (drv != 0)
    {
	Event *e = new Event(new JobMediumAction(g, drv, JobMediumAction::Act_RepairDisc));
	thrOperations->DoAsync(Cmd_ExecuteJob, e);
    }

    Drive.Release();
}

void Engine::quickErase()
{
    iptr drv = Drive.ObtainRead();

    if (drv != 0)
    {
	Event *e = new Event(new JobMediumAction(g, drv, JobMediumAction::Act_QuickErase));
	thrOperations->DoAsync(Cmd_ExecuteJob, e);
    }

    layoutTracks(false, false);
    Drive.Release();
}

void Engine::completeErase()
{
    iptr drv = Drive.ObtainRead();

    if (drv != 0)
    {
	Event *e = new Event(new JobMediumAction(g, drv, JobMediumAction::Act_CompleteErase));
	thrOperations->DoAsync(Cmd_ExecuteJob, e);
    }

    layoutTracks(false, false);
    Drive.Release();
}

void Engine::quickFormat()
{
    iptr drv = Drive.ObtainRead();

    if (drv != 0)
    {
	Event *e = new Event(new JobMediumAction(g, drv, JobMediumAction::Act_QuickFormat));
	thrOperations->DoAsync(Cmd_ExecuteJob, e);
    }

    layoutTracks(false, false);
    Drive.Release();
}

void Engine::completeFormat()
{
    iptr drv = Drive.ObtainRead();

    if (drv != 0)
    {
	Event *e = new Event(new JobMediumAction(g, drv, JobMediumAction::Act_CompleteFormat));
	thrOperations->DoAsync(Cmd_ExecuteJob, e);
    }

    layoutTracks(false, false);
    Drive.Release();
}

void Engine::structureDisc()
{
    iptr drv = Drive.ObtainRead();

    if (drv != 0)
    {
	Event *e = new Event(new JobMediumAction(g, drv, JobMediumAction::Act_Prepare));
	thrOperations->DoAsync(Cmd_ExecuteJob, e);
    }

    layoutTracks(false, false);
    Drive.Release();
}

void Engine::downloadTrack(const IOptItem *track, const char* name)
{
#warning reimplement this.
#if 0
    if (NULL == expData)
    {
	request("Information", "The ISO Export module has not been selected", "Ok", 0);
	return;
    }

    if (NULL == expAudio)
    {
	request("Information", "The Audio Export module has not been selected", "Ok", 0);
	return;
    }

    iptr drv = Drive.ObtainRead();
    track->claim();

    if (drv != 0)
    {
	JobDownload *dl;
	if (track->getItemType() == Item_Track)
	{
	    if (track->getDataType() == Data_Audio)
	    {
		dl = new JobDownload(g, drv, track, expAudio, name);
	    }
	    else
	    {
		dl = new JobDownload(g, drv, track, expData, name);
	    }
	}
	else
	{
	    dl = new JobDownload(g, drv, track, expSession, name);
	}
	thrOperations->DoAsync(Cmd_ExecuteJob, new Event(dl));
    }

    Drive.Release();
#endif
}

unsigned short Engine::getActionProgress()
{
    unsigned short progress = 0;

    Job *pjob = hJob.ObtainRead();

    if (NULL != pjob)
    {
	progress = pjob->getProgress();
    }

    if (progress == 0)
    {
	uint32 drv = Drive.ObtainRead();
	if (drv != 0)
	{
	    progress = g.Optical->DoMethodA(ARRAY(DRV_GetAttr, drv, DRA_Drive_OperationProgress));
	}
	Drive.Release();
    }

    hJob.Release();

    return progress;
}

void Engine::addTrack(IData *f)
{
    VectorT<IData*> &vecTracks = tracks().ObtainWrite();
    vecTracks << f;
    tracks().Release();

    thrOperations->DoAsync(Cmd_SendEvent, new Event(Eng_Update));
}

void Engine::remTrack(IData* track)
{
    VectorT<IData*> &vecTracks = tracks().ObtainWrite();
    vecTracks >> track;
    tracks().Release();

    thrOperations->DoAsync(Cmd_SendEvent, new Event(Eng_Update));
}

RWSyncT< VectorT<IData*> > &Engine::tracks()
{
    return hTracks;
}

void Engine::recordDisc(bool masterize, bool closedisc)
{
    VectorT<IData*> &v = tracks().ObtainRead();
    iptr cnt = v.Count();
    tracks().Release();

    if (0 == cnt)
	return;

    iptr drive = Drive.ObtainRead();
    if (0 != drive)
    {
	JobUpload *job = new JobUpload(g, drive, tracks(), masterize, closedisc);
	thrOperations->DoAsync(Cmd_ExecuteJob, new Event(job));
    }
    Drive.Release();
}

void Engine::recordSession()
{
    iptr drive = Drive.ObtainRead();
    if (0 != drive)
    {
	JobUploadSession *job = new JobUploadSession(g, drive, pImportedSession, pSessImportReader);
	thrOperations->DoAsync(Cmd_ExecuteJob, new Event(job));
    }
    Drive.Release();
}

void Engine::updateAll()
{
    iptr drv = Drive.ObtainRead();

    if (drv != 0)
    {
	JobUpdate *job = new JobUpdate(g, drv);
	thrOperations->DoAsync(Cmd_ExecuteJob, new Event(job));
    }

    Drive.Release();
}

void Engine::createImage(const char* name)
{
#warning please fill me!
#if 0
     if (NULL == expData)
    {
	request("Information", "The ISO Export module has not been selected", "Ok", 0);
	return;
    }

   iptr drv = Drive.ObtainRead();

    if (drv != 0)
    {
	JobCreateISO *dl;

	dl = new JobCreateISO(g, drv, iso, expData, name);
	thrOperations->DoAsync(Cmd_ExecuteJob, new Event(dl));
    }

    Drive.Release();
#endif
}

uint32 Engine::getLayoutSize()
{
    return hLayoutSize;
}

uint32 Engine::getDiscSize()
{
    return lDiscSize;
}

const IOptItem *Engine::getSessionContents()
{
    return pImportedSession;
}

bool Engine::isDiscWriteProtectable()
{
    return bWriteProtectable;
}

bool Engine::isDiscWriteProtected()
{
    return bWriteProtected;
}

void Engine::setDiscWriteProtected(bool state)
{
    iptr drv = Drive.ObtainRead();

    if (drv != 0)
    {
	bWriteProtected = state;
	Event *e = new Event(new JobWriteProtect(g, drv, bWriteProtected));
	thrOperations->DoAsync(Cmd_ExecuteJob, e);
	thrOperations->DoAsync(Cmd_SendEvent, new Event(Eng_Update));
    }

    Drive.Release();
}

const TagItem* Engine::obtainDriveUpdateTags()
{
    return pDriveUpdateTags.ObtainRead();
}

void Engine::releaseDriveUpdateTags()
{
    pDriveUpdateTags.Release();
}
   
/*
 * let the application have some fun with the drive if needed
 */
iptr Engine::obtainDrive()
{
    return Drive.ObtainRead();
}

void Engine::releaseDrive()
{
    Drive.Release();
}

DbgHandler* Engine::getDebug() const
{
    return dbg;
}

void Engine::setDebug(DbgHandler* h)
{
    dbg = h;
}
