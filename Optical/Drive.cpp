#include "Headers.h"
#include "Config.h"
#include <Generic/Thread.h>
#include "Humming.h"
#include <libclass/utility.h>
#include "Drive.h"
#include "Disc_Generic.h"
#include <Generic/Msg.h>
#include "DriveMsg.h"
#include "DriveClient.h"
#include "DriveSpool.h"
#include <Generic/Signal.h>


#include <SCSI/scsi_GetConfiguration.h>

DriveSpool *DriveSpool::instance = 0;

/*
 * get device name
 */
const String &Drive::GetDeviceName()
{
    return drive_name;
}

/*
 * get unit number
 */
int Drive::GetUnitNumber()
{
    return unit_number;
}

/*
 * construct new drive
 */
Drive  *Drive::GetDrive(const char* pDevice, int lUnit)
{
    AllClear = Clear1 && Clear2;
    return new Drive(pDevice, lUnit);
}

/*
 * pass message to the drive handler
 */
void Drive::SendMessage(DriveMsg* m)
{
    thread.Send(m);
}

/*
 * private destructor
 */
Drive::~Drive()
{
#warning destructors not called anywhere at all!
    _D(Lvl_Info, "Drive done.");
    _ED();
};

/*
 * private constructor
 */
Drive::Drive(const char *drv, int unit) :
    handle(this, &Drive::HandleMessages),
    task(this, &Drive::ProcMsgs),
    thread("Drive Subprocess", handle, task),
    last_result(DRT_Operation_Unknown),
    inquiry(driveio, last_result),
    tur(driveio, last_result),
    mode_io(driveio, last_result),
    features(driveio, last_result)
{
    _ND("Drive");

    driveio.setDebug(DEBUG_ENGINE);

    // initialize all elements   
    {
	bLockInterOperations = false;
	drive_name           = drv;
	unit_number          = unit;
	bRegistered          = Clear2;
	is_disc_in_drive     = false;
	current_disc.Assign(0);
    }

    _D(Lvl_Info, "Spawning new drive process for %s.%ld, class %08lx", (ULONG)drv, unit, (int)this);
    thread.Start();
    _D(Lvl_Info, "Process initialized.");
};

/*
 * MAIN SUBPROCESS
 * - Entry point,
 * - Exit point,
 * - Main loop
 */
bool Drive::ProcInit()
{
    pc             =  0;
    pw             =  0;
    discinfo       =  0;
    hwconfig       =  0;
    selected_read_speed  = 0xffff;
    selected_write_speed = 0xffff;
    current_disc.Assign(0);

    DriveSpool::DriveStarted();

    _D(Lvl_Debug, "Opening drive %s.%ld", (ULONG)drive_name.Data(), unit_number);
    if (!driveio.OpenDev(drive_name.Data(), unit_number))
    {
	_D(Lvl_Warning, "Drive open FAILED");
	return false;
    }

    _D(Lvl_Warning, "Device open successful. Initializing drive");
    {
	cmd_Reset r(driveio, last_result);
	r.Go();
    }

    _D(Lvl_Info, "Drive initialization went fine. Proceeding with thread data initialization");
    return true;
}

/*
** TODO: would be good to add some checking if we can execute scsi commands here
*/
void Drive::ProcMsgs(Thread *pThis)
{
    bool bUpdate = true;
    int  period = 0;
    uint32 lStatus;
    bool bReported = false;
    Disc *d = 0;
    bool bRes;

    _D(Lvl_Info, "Initializing subprocess for class %08lx...", (int)this);
    if (!ProcInit())
    {
	/*
	** nie chcemy zadnych stanow zwracac tutaj
	** bo i tak wykonanie komend sie nie powiedzie.
	*/
	_D(Lvl_Error, "Drive process initialization failed.");
	_D(Lvl_Info, "Handling remaining commands...");
	pThis->HandleSignals(0xffffffff);
    }
    else
    {
	while (!pThis->HandleSignals(period))
	{
	    bUpdate |= update;
	    if (!bLockInterOperations)
	    {
		bRes = tur.Go();
		lStatus = tur.Result().SCSIError();

		/*
		** for faulty platforms: if tur fails
		** mark drive as busy.
		*/
		if (!bRes)
		{
		    if (0 == lStatus)
			lStatus = 0x20000;
		}

		/*
		** check what we can extract:
		** - is disc in drive?
		** - is drive doing anything right now?
		*/
		switch (lStatus & 0xffff00) 
		{
		    case 0x062800:	// medium may have changed
		    case 0x020000:	// just busy
		    case 0x020400:	// iptr operation in progress
		    case 0x023a00:	// tray opening or closing, no disc in drive.
			_D(Lvl_Info, "No disc inserted or detection failed.");
			notify(last_result(DRT_Operation_IdleNoDisc, DRT_OpStatus_Ready));

			if (is_disc_in_drive)
			    bUpdate = true;
			is_disc_in_drive = false;
			break;

		    case 0x000000:	// drive ready.
			/*
			** this status may only be vaild if we got correct
			** result of operation...
			*/
			if (!is_disc_in_drive)
			    bUpdate = true;
			is_disc_in_drive = true;
			break;

		    default:
			_D(Lvl_Error, "Unknown / unexpected result during TUR: %06lx", lStatus);
			break;
		}

		/*
		** further checks if disc requires updating
		*/
		d = current_disc.ObtainRead();
		if (d != 0)
		    bUpdate |= d->RequiresUpdate();
		current_disc.Release();

		/*
		 * shall we update?
		 */
		if (bUpdate)
		{
		    _D(Lvl_Info, "Starting Update");
		    /*
		     * if anyone requested update manually, it won't be needed any more.
		     */
		    bUpdate = false;
		    update = false;

		    AnalyseDrive();
		    AnalyseDisc();

		    d = current_disc.ObtainRead();
		    current_disc.Release();
		
		    /*
		     * we got status saying 'we got disc' but the disc is not there
		     */
		    if ((bReported == false) && ((lStatus == 0) == (d == 0)))
		    {
			_D(Lvl_Warning, "This is weird. There seems to be no disc but the drive still reports ready... Will try the other time");
			_D(Lvl_Warning, "  Status: %lx, Disc: %lx", lStatus, (iptr)d);
			request("Warning", "It appears that there are some hardware configuration problems\n"
				"and your drive will not function properly. Some features such as\n"
				"automation will not report proper data.\n\n"
				"Please note that this program may function in an unpredicted way\n"
				"or not at all. I will take no responsibility of any damage caused by\n"
				"further usage of this program on your configuration.", "OK", 0);
			bReported = true;
		    }
		}

		d = current_disc.ObtainRead();
		if (d != 0)
		    period   = d->GetRefreshPeriod() * 20;
		else
		    period   = 3000;
		current_disc.Release();
	    }
	} 
    }

    ProcExit();
    _D(Lvl_Info, "All done, thank you.");
};

void Drive::ProcExit()
{
    _D(Lvl_Debug, "Cleaning up drive process.");
    driveio.CloseDev();
    DriveSpool::DriveStopped();
}

/*
 * analyze disc - check disc type and read its contents
 */
void Drive::AnalyseDisc()
{
    Disc *d = 0;

    d = current_disc.Assign(0);
    if (d) delete d;

    if (!is_disc_in_drive)
    {
	_D(Lvl_Debug, "No disc in drive.");
	return;
    }

    _D(Lvl_Debug, "Analysing disc...");

    d = Disc::GetDisc(*this, (DRT_Profile)current_drive_profile);

    if (d)
    {
	_D(Lvl_Info, "Disc type: %ld", d->DiscType());
	d->SetReadSpeed(selected_read_speed);
	d->SetWriteSpeed(selected_write_speed);
	notify(last_result(DRT_Operation_Ready, DRT_OpStatus_Ready));
    }


    current_disc.Assign(d);
}

/*
 * analyse drive - obtain as much information about present drive capabilities as you can
 */
void Drive::AnalyseDrive()
{
    notify(last_result(DRT_Operation_Analyze_Drive, DRT_OpStatus_InProgress));
    inquiry.Go();

    if (inquiry.Result() != ODE_OK) 
    {
	/*
	** inquiry fills in last_result
	*/
	_D(Lvl_Error, "%s: DriveIO reported an error, will not continue.", (int)__PRETTY_FUNCTION__);
	notify(last_result);
	return;
    } 
    else
    {
	hwconfig = Cfg->GetHardware(inquiry.ProductID());
	selected_read_speed = hwconfig->getReadSpeed();
	selected_write_speed = hwconfig->getWriteSpeed();

	_D(Lvl_Info, "Drive information:");
	_D(Lvl_Info, "Drive type: %ld", inquiry.DriveType());
	_D(Lvl_Info, "Standard  : %ld", inquiry.SupportedStandard());
	_D(Lvl_Info, "Product   : %s",  (iptr)inquiry.ProductID());
	_D(Lvl_Info, "Vendor    : %s",  (iptr)inquiry.VendorID());
	_D(Lvl_Info, "Version   : %s",  (iptr)inquiry.ProductVersion());
    }

    pc = mode_io.GetPage(cmd_Mode::Id_Page_Capabilities);
    _D(Lvl_Info, "Capabilities: id: %02lx, len: %02lx, hdr: %08lx, page: %08lx", pc.IsValid() ? pc->PageID() : 0xff, pc.IsValid() ? pc->PageSize() : 0, pc.IsValid() ? (int)((Page_Header*)pc) : 0, pc.IsValid() ? (int)((Page_Capabilities*)pc) : 0);
    if (pc.IsValid())
    {
	eMechanismType  = pc->GetMechanismType();
	eCapabilities  << pc->GetCapabilities();
    }
    else
    {
	eMechanismType = DRT_Mechanism_Unknown;
	eCapabilities  = 0;
    }

    pw = mode_io.GetPage(cmd_Mode::Id_Page_Write);
    _D(Lvl_Info, "Write       : id: %02lx, len: %02lx, hdr: %08lx page: %08lx", pw.IsValid() ? pw->PageID() : 0xff, pw.IsValid() ? pw->PageSize() : 0, pw.IsValid() ? (int)((Page_Header*)pw) : 0, pw.IsValid() ? (int)((Page_Write*)pw) : 0);

    media_write_capabilities   =  0;
    media_read_capabilities    =  0;
    current_drive_profile      = DRT_Profile_NoDisc;

    if (is_disc_in_drive)
    {
	if (!discinfo)                               // jesli trzeba utworzyc...
	    discinfo = new cmd_ReadDiscInfo(driveio, last_result);

	discinfo->Go();

	if (discinfo->Result() != ODE_OK) 
	{
	    delete discinfo;
	    discinfo = 0;
	}
    } 
    else 
    {
	if (discinfo) delete discinfo;
	discinfo = 0;
    }

    features.Go();

#ifdef DEBUG
    for (Feature *f = features.GetNextFeature(0); 
	    f != 0; 
	    f = features.GetNextFeature(f))
    {
	_D(Lvl_Info, "Feature: %lx:%lx (C:%ld/P:%ld/V:%ld)", f->GetId(), f->GetLength(), f->IsCurrent(), f->IsPersistent(), f->GetVersion());
	_DDT("Feature Data", f->GetBody(), f->GetBodyLength());
    }
#endif


    if (features.Result() == ODE_OK) 
    {
	media_write_capabilities   =  features.GetMediaWriteSupport();
	media_read_capabilities    =  features.GetMediaReadSupport();
	eCapabilities << features.GetDriveCapabilities();

	if (discinfo != 0)
	    current_drive_profile   =  features.GetCurrentProfile();
	_D(Lvl_Info, "Drive reported profiles porperly. Will now analyse.");
	_D(Lvl_Info, "Current drive profile is %04lx", features.GetCurrentProfile());
	_D(Lvl_Info, "Drive can access media:  %08lx", features.GetMediaReadSupport());
	_D(Lvl_Info, "Drive can write media:   %08lx", features.GetMediaWriteSupport());
    }
    else 
    {
	_D(Lvl_Info, "Drive does not report any profiles. Will try the other way");
	if (pc.IsValid()) 
	{
	    media_write_capabilities   =  pc->GetMediaWriteSupport();
	    media_read_capabilities    =  pc->GetMediaReadSupport();
	    _D(Lvl_Info, "Will try to learn from capabilities...");
	    _D(Lvl_Info, "Drive can access media: %08lx", pc->GetMediaReadSupport());
	    _D(Lvl_Info, "Drive can write media:  %08lx", pc->GetMediaWriteSupport());
	} 

	if (discinfo) 
	{
	    current_drive_profile = DRT_Profile_CD_ROM;
	    if (discinfo->GetData()->IsWritable()) current_drive_profile = DRT_Profile_CD_R;
	    if (discinfo->GetData()->IsErasable()) current_drive_profile = DRT_Profile_CD_RW;
	}
    }   

    _D(Lvl_Info, "Current drive profile: %ld", current_drive_profile);

    // now the further part, speeds a.s.o.
    if (pc.IsValid()) 
    {
	drive_buffer_size          =  pc->GetBufferSize();
    }

    notify(last_result(DRT_Operation_Analyze_Drive));
    Cfg->onWrite();
}

void Drive::notify(IDriveStatus& s)
{
    /*
    ** TODO: add scsi_error -> const char mapping!
    */
    VectorT<DriveClient*> &vec = users.ObtainRead();

    for (uint32 i=0; i<vec.Count(); i++)
    {
	vec[i]->notify(s);
    }

    users.Release();
}

/*
 * handle drive messages
 */
void Drive::HandleMessages(const Port*, GenNS::Msg *msg)
{
    DriveMsg*	m = static_cast<DriveMsg*>(msg);
    Disc*	d = current_disc.ObtainRead();

    last_result.Reset();

    _D(Lvl_Info, "Received message: %ld", m->cmd);

    switch (m->cmd) 
    {
	/*
	 * NO-OP command: do nothing, say it's fine.
	 */
	case DRV_NoOP:
	    _D(Lvl_Debug, "CMD: NoOP");
	    break;

	case DRV_EndDrive:
	    _D(Lvl_Debug, "CMD: Terminate self");
	    thread.Terminate();
	    break;

	    /*
	     * start blanking disc
	     */
	case DRV_Blank:
	    _D(Lvl_Debug, "CMD: Blank");
	    if (!driveio.IsOpened())
		last_result(DRT_Operation_Ready, DRT_OpStatus_Aborted, 0, 0, ODE_NoHandler);
	    else if (!d) 
		last_result(DRT_Operation_Ready, DRT_OpStatus_Aborted, 0, 0, ODE_NoDisc);
	    else
		last_result = d->EraseDisc(m->Blank.method);
	    break;

	    /*
	     * control unit tray and power consumption
	     */
	case DRV_ControlUnit:
	    _D(Lvl_Debug, "CMD: ControlUnit");
	    if (!driveio.IsOpened())
		last_result(DRT_Operation_Ready, DRT_OpStatus_Aborted, 0, 0, ODE_NoHandler);
	    else
		ControlDrive(m->Control.control);
	    break;

	    /*
	     * layout tracks - update structures to reflect how these will look in real life
	     */
	case DRV_UploadLayout:
	    _D(Lvl_Debug, "CMD: LayoutTracks");
	    if (!driveio.IsOpened())
		last_result(DRT_Operation_Ready, DRT_OpStatus_Aborted, 0, 0, ODE_NoHandler);
	    else if (!d) 
		last_result(DRT_Operation_Ready, DRT_OpStatus_Aborted, 0, 0, ODE_NoDisc);
	    else
	    {
		if (m->Write.checkLayout)
		    last_result = d->LayoutTracks(m->Write.item);
		else
		    last_result = d->UploadLayout(m->Write.item);
	    }
	    break;

	    /*
	     * close disc or session
	     */
	case DRV_CloseDisc:
	    _D(Lvl_Debug, "CMD: CloseMedium");
	    if (!driveio.IsOpened())
		last_result(DRT_Operation_Ready, DRT_OpStatus_Aborted, 0, 0, ODE_NoHandler);
	    else if (!d) 
		last_result(DRT_Operation_Ready, DRT_OpStatus_Aborted, 0, 0, ODE_NoDisc);
	    else
		last_result = d->CloseDisc(m->Close.method, m->Close.track_number);
	    break;

	    /*
	     * force drive re-scan and re-read
	     */
	case DRV_ForceUpdate:
	    _D(Lvl_Debug, "CMD: Forcing Update");
	    if (!driveio.IsOpened())
		last_result(DRT_Operation_Ready, DRT_OpStatus_Aborted, 0, 0, ODE_NoHandler);
	    else
		update = true;
	    break;

	case DRV_ReadTrackAbsolute:
	    _D(Lvl_Debug, "CMD: ReadTrackAbsolute");
	    if (!driveio.IsOpened())
		last_result(DRT_Operation_Ready, DRT_OpStatus_Aborted, 0, 0, ODE_NoHandler);
	    else if (!d) 
		last_result(DRT_Operation_Ready, DRT_OpStatus_Aborted, 0, 0, ODE_NoDisc);
	    else
		last_result = d->RandomRead(m->Read.item, m->Read.block, m->Read.length, m->Read.mem);
	    break;

	case DRV_ReadTrackRelative:
	    _D(Lvl_Debug,"CMD: ReadTrackRelative");
	    if (!driveio.IsOpened())
		last_result(DRT_Operation_Ready, DRT_OpStatus_Aborted, 0, 0, ODE_NoHandler);
	    else if (!d) 
		last_result(DRT_Operation_Ready, DRT_OpStatus_Aborted, 0, 0, ODE_NoDisc);
	    else
		last_result = d->RandomRead(
			m->Read.item, 
			m->Read.block + m->Read.item->getStartAddress(),
			m->Read.length,
			m->Read.mem);
	    break;

	default:
	    ASSERTS(false, "Unsupported command");
	    last_result(DRT_Operation_Ready, DRT_OpStatus_Aborted, 0, 0, ODE_IllegalCommand, 0);
    }

    m->Reply(last_result);
    current_disc.Release();
}

bool Drive::ControlDrive(DRT_Control type)
{
    cmd_StartStopUnit	ssu(driveio, last_result);
    cmd_LockDrive	pLock(driveio, last_result);
    bool res = false;

    switch (type) 
    {
	case DRT_Unit_Lock:
	    notify(last_result(DRT_Operation_Control_Lock, DRT_OpStatus_InProgress));
	    pLock.setLocked(false);
	    res = pLock.Go();
	    break;

	case DRT_Unit_Unlock:
	    notify(last_result(DRT_Operation_Control_Unlock, DRT_OpStatus_InProgress));
	    pLock.setLocked(true);
	    res = pLock.Go();
	    break;

	case DRT_Unit_Stop:
	    notify(last_result(DRT_Operation_Control_SpinDown, DRT_OpStatus_InProgress));
	    ssu.setType(cmd_StartStopUnit::StartStop_Stop);
	    res = ssu.Go();
	    break;

	case DRT_Unit_Start:
	    notify(last_result(DRT_Operation_Control_SpinUp, DRT_OpStatus_InProgress));
	    ssu.setType(cmd_StartStopUnit::StartStop_Start);
	    res = ssu.Go();
	    break;

	case DRT_Unit_Eject:
	    notify(last_result(DRT_Operation_Control_Eject, DRT_OpStatus_InProgress));
	    ssu.setType(cmd_StartStopUnit::StartStop_Eject);
	    res = ssu.Go();
	    break;

	case DRT_Unit_Load:
	    notify(last_result(DRT_Operation_Control_Load, DRT_OpStatus_InProgress));
	    ssu.setType(cmd_StartStopUnit::StartStop_Load);
	    res = ssu.Go();
	    break;

	case DRT_Unit_Idle:
	    notify(last_result(DRT_Operation_Control_Standby, DRT_OpStatus_InProgress));
	    ssu.setType(cmd_StartStopUnit::StartStop_Idle);
	    res = ssu.Go();
	    break;

	case DRT_Unit_Standby:
	    notify(last_result(DRT_Operation_Control_Standby, DRT_OpStatus_InProgress));
	    ssu.setType(cmd_StartStopUnit::StartStop_Standby);
	    res = ssu.Go();
	    break;

	case DRT_Unit_Sleep:
	    notify(last_result(DRT_Operation_Control_Standby, DRT_OpStatus_InProgress));
	    ssu.setType(cmd_StartStopUnit::StartStop_Sleep);
	    res = ssu.Go();
	    break;

	default:
	    notify(last_result(DRT_Operation_Control_General, DRT_OpStatus_Aborted, 0, 0, ODE_IllegalParameter));
	    FAIL(true, "Invalid scenario! we should never be here.")
		return false;
    }

    notify(last_result);
    return res;
}

iptr Drive::GetDriveAttrs(iptr tag, iptr attr)
{
    /*
    ** TODO: drive opened or not, this shouldn't bother  us
    ** worst case, we're not doing any actions.
    */
#warning protect this section.
    register Disc* d = current_disc.ObtainRead(); 
    register iptr res = 0;

    switch (tag)
    {
	case DRA_DeviceName:                   
	    res = (int)drive_name.Data();
	    break;

	case DRA_UnitNumber:                   
	    res = unit_number;
	    break;

	case DRA_IsOpened:
	    res = driveio.IsOpened();
	    break;

	case DRA_Drive_ReadSpeeds:             
	    res = d ? (iptr)d->GetReadSpeeds() : 0;
	    break;

	case DRA_Drive_WriteSpeeds:
	    res = d ? (iptr)d->GetWriteSpeeds() : 0;
	    break;

	case DRA_Drive_CurrentOperation:       
	    res = 0;                     // OBSOLETE!?
	    break;

	case DRA_Drive_OperationProgress:      
	    res = d ? d->GetOperationProgress() : 0;
	    break;

	case DRA_Drive_AbsoluteCDAddress:      
	    res = 0;                     // !!
	    break;

	case DRA_Drive_RelativeCDAddress:      
	    res = 0;                     // !!
	    break;

	case DRA_Drive_Vendor:                 
	    res = (iptr)inquiry.VendorID();
	    break;

	case DRA_Drive_Product:                
	    res = (iptr)inquiry.ProductID();
	    break;

	case DRA_Drive_Version:                
	    res = (iptr)inquiry.ProductVersion();
	    break;

	case DRA_Drive_Firmware:               
	    res = (iptr)inquiry.FirmwareVersion();
	    break;

	case DRA_Drive_ReadsMedia:             
	    res = media_read_capabilities;
	    break;

	case DRA_Drive_WritesMedia:            
	    res = media_write_capabilities;
	    break;

	case DRA_Drive_MechanismType:          
	    res = eMechanismType;
	    break;

	case DRA_Drive_Capabilities:           
	    res = eCapabilities;
	    break;

	case DRA_Drive_BufferSize:             
	    res = drive_buffer_size;
	    break;

	case DRA_Drive_BufferUsed:             
	    res = 0;                     // !!
	    break;

	case DRA_Drive_VolumeLevels:           
	    res = 0;                     // !!
	    break;

	case DRA_Drive_CurrentAudioTrackIndex: 
	    res = 0;                     // !!
	    break;

	case DRA_Drive_CurrentProfile:         
	    res = d ? d->DiscType() : DRT_Profile_Unknown;
	    break;

	case DRA_Drive_CurrentReadSpeed:       
	    res = d ? d->GetReadSpeed() : selected_read_speed;
	    break;

	case DRA_Drive_CurrentWriteSpeed:      
	    res = d ? d->GetWriteSpeed() : selected_write_speed;
	    break;

	case DRA_Drive_IsRegistered:           
	    res = bRegistered;
	    break;

	case DRA_Drive_WriteParams:            
	    res = 0;                     // OBSOLETE!!!!!!
	    break;

	case DRA_Disc_NumTracks:               
	    res = d ? (ULONG)d->GetNumTracks() : 0;
	    break;

	case DRA_Disc_SubType:                 
	    res = d ? d->DiscSubType() : 0;
	    break;

	case DRA_Disc_ID:                      
	    res = 0;                     // !!
	    break;

	case DRA_Disc_Size:                    
	    res = d ? d->GetDiscSize() : 0;
	    break;

	case DRA_Disc_State:                   
	    res = 0;                     // !!
	    break;

	case DRA_Disc_LastSess_State:          
	    res = 0;                     // !!
	    break;

	case DRA_Disc_Format:                  
	    res = 0;                     // !! -rom / -xa / -i
	    break;

	case DRA_Disc_BarCode:                 
	    res = 0;                     // !!
	    break;

	case DRA_Disc_CatalogNumber:           
	    res = 0;                     // !!
	    break;

	case DRA_Disc_NextWritableTrack:       
	    res = d ? (uint32)d->GetNextWritableTrack((IOptItem*)attr) : 0;
	    break;

	case DRA_Disc_WriteMethod:             
	    res = d ? d->GetWriteMethod() : 0;
	    break;

	case DRA_Disc_Vendor:                  
	    res = d ? (int)d->DiscVendor() : 0;
	    break;

	case DRA_Disc_WriteProtect:
	    res = d ? (int)d->IsWriteProtected() : 0;
	    break;

	case DRA_Disc_AllowsWriteProtect:
	    res = d ? (int)d->AllowsWriteProtect() : 0;
	    break;

	default:
	    ASSERTS(false, "Unknown tag requested!");
    }

    current_disc.Release();

    return res;
}

DriveStatus& Drive::SetPage(Page_Header *p)
{
    mode_io.SetPage(p);
    return mode_io.Result();
};

Page<Page_Write> &Drive::GetWritePage(void)
{
    return pw;
};

Page<Page_Capabilities> &Drive::GetCapabilitiesPage(void)
{
    return pc;
};

iptr Drive::ScanDevice(char* sDeviceName)
{
    DEFINE_DEBUG;
    DriveIO	IO;
    DriveStatus res(DRT_Operation_Unknown);
    ScanData   *pData= 0;
    ScanData   *pRes = 0;
    cmd_Inquiry	inq(IO, res);
    cmd_Reset	rst(IO, res);

    _NDS("Device Scan");

    IO.setDebug(DEBUG_ENGINE);

    _D(Lvl_Info, "Checking if device %s is harmless...", (int)sDeviceName);
    switch (Cfg->Drivers()->isHarmful(sDeviceName))
    {
	case stYes:
	    {
		_D(Lvl_Warning, "Device is harmful.");
		request("Error", "Device %s is considered harmful\nOperation aborted.", "Ok", ARRAY((int)sDeviceName));
	    }
	    break;

	case stUnknown:
	    {
		_D(Lvl_Warning, "Device is not recorded yet.");
		if (0 == request("Warning", "Device %s is not known and may be harmful.\nDo you want to continue?", "Yes|No", ARRAY((int)sDeviceName)))
		{
		    Cfg->Drivers()->addDevice(sDeviceName, true);
		    break;
		}
	    }
	    // no break here.

	case stNo:
	    {
		_D(Lvl_Info, "Scanning device %s...", (int)sDeviceName);
		for (int i=0; i<8; i++)
		{
		    _D(Lvl_Info, "Accessing device %s, unit %ld...", (int)sDeviceName, i);
		    if (IO.OpenDev(sDeviceName, i))
		    {
			_D(Lvl_Warning, "Device open successful. Initializing drive");
			rst.Go();

			if (inq.Go())
			{
			    if (!pData)
			    {
				pData = new ScanData;
				pRes = pData;
			    }
			    else
			    {
				pData->sd_Next = new ScanData;
				pData = pData->sd_Next;
			    }           
			    pData->sd_Next       = 0;
			    pData->sd_DeviceName = new char[32];
			    pData->sd_Vendor     = new char[16];
			    pData->sd_Unit       = i;
			    pData->sd_Type       = inq.DriveType();
			    strcpy(pData->sd_DeviceName,  inq.ProductID());
			    strcpy(pData->sd_Vendor,      inq.VendorID());
			}
			IO.CloseDev();
		    }

		}    

		Cfg->Drivers()->addDevice(sDeviceName, false);
	    }
	    break;

    }

    _ED();
    return (iptr)pRes;
}

iptr Drive::FreeScanResults(ScanData*pData)
{
    while (pData)
    {
	ScanData *pNext = pData->sd_Next;
	delete [] pData->sd_Vendor;
	delete [] pData->sd_DeviceName;
	delete [] pData;
	pData = pNext;
    }
    return 0;
}

iptr Drive::GetDriveAttrs(TagItem *ti)
{
    TagItem  *t;

    while ((t = Utility->NextTagItem(&ti))!=0)
    {
	int* dest = (int*)((iptr)t->ti_Data);
	ASSERT(dest);
	*dest = GetDriveAttrs(t->ti_Tag, *dest);
    };
    return 0;
}

iptr Drive::GetDriveAttrs(iptr tag)
{
    iptr lRet = GetDriveAttrs(tag, 0UL);
    return lRet;
}

const IOptItem* Drive::GetDiscContents()
{
    Disc* d = current_disc.ObtainRead();
    const IOptItem* item = 0;
    if (d != 0)
	item = d->GetContents();
    current_disc.Release();
    return item;
}

bool Drive::IsDiscPresent()
{
    Disc* d = current_disc.ObtainRead();
    current_disc.Release();
    return (d != 0);
}

bool Drive::IsDiscErasable()
{
    bool flag;

    Disc* d = current_disc.ObtainRead();
    flag = (d != 0 && d->IsErasable());
    current_disc.Release();
    return flag;
}

bool Drive::IsDiscFormattable()
{
    bool flag;

    Disc* d = current_disc.ObtainRead();
    flag = (d != 0 && d->IsFormattable());
    current_disc.Release();
    return flag;
}

bool Drive::IsDiscOverwritable()
{
    bool flag;

    Disc* d = current_disc.ObtainRead();
    flag = (d != 0 && d->IsOverwritable());
    current_disc.Release();
    return flag;
}

bool Drive::IsDiscFormatted()
{
    bool flag;

    Disc* d = current_disc.ObtainRead();
    flag = (d != 0 && d->IsFormatted());
    current_disc.Release();
    return flag;
}

bool Drive::IsDiscWritable()
{
    bool flag;

    Disc* d = current_disc.ObtainRead();
    flag = (d != 0 && d->IsWritable());
    current_disc.Release();
    return flag;
}

/*
** increase number of users.
** safe to call only from drivespool.
*/
DriveClient* Drive::getClient() 
{
    VectorT<DriveClient*> &vec = users.ObtainWrite();
    DriveClient* dc = new DriveClient(*this);
    vec << dc;
    users.Release();
    return dc;
}

/*
** decrease number of users
** safe to call only from drivespool.
*/
bool Drive::freeClient(DriveClient* dc)
{
    bool res;

    VectorT<DriveClient*> &vec = users.ObtainWrite();
    vec >> dc;
    res = (vec.Count() == 0);
    users.Release();

    if (res)
    {
	_D(Lvl_Info, "Last client disconnected. Terminating!");
	thread.Terminate();
	thread.WaitTerminated();
    }
    return res;
}

