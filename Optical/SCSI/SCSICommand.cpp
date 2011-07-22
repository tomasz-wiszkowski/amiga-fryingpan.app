#include "SCSICommand.h"
#include <libclass/dos.h>
#include "../Drive.h"

SCSICommand::SCSICommand(DriveIO &drvio, DriveStatus& r) :
    res(r),
    dio(drvio)
{
    scsi_Data         = 0;
    scsi_Length       = 0;
    scsi_Actual       = 0;
    scsi_CmdLength    = 0;
    scsi_CmdActual    = 0;
    scsi_Status       = 0;
    scsi_SenseActual  = 0;

    for (uint32 i=0; i<sizeof(cmd); i++)
	cmd[i] = 0;

    scsi_Command      = (uint8*)&cmd;
    scsi_Flags        = SCSIF_AUTOSENSE;
    scsi_SenseData    = dio.SenseData();
    scsi_SenseLength  = dio.SenseLength();

    need_probe        = 0;
    dump_data         = 0;
    cmdname           = "SCSI Command";
};

SCSICommand::~SCSICommand()
{
};

bool SCSICommand::Go()
{
    bool ec = true;
    uint32 scsi;
    /*
    ** initialize result
    ** the only place where it should happen
    */
    Result().Reset();

#ifdef DEBUG
    if (0 != (DataLength() & 1))
    {
	_d(Lvl_Failure, "ERROR: ODD TRANSFER LENGTH IS NOT ALLOWED! PLEASE REPORT!");
	_ddt("COMMAND CAUSING THE FAULT:", Cmd(), CmdLength());
	_d(Lvl_Failure, "REQUEST ABORTED.");
	Result().Complete(ODE_BadTransferSize, 0);
	return false;
    }
#endif

    /*
    ** watch it! we do not touch anything in onInit
    ** unless it really fails!
    */
    if (!onInit())
    {
#ifdef DEBUG
	FAIL(Result() == ODE_OK, "%s:\nInitialization failed, but return code is ok!", (iptr)CmdName());
#endif
	return false;
    }

    /*
    ** loop while we need
    */

    do 
    {
	if (NeedProbe()) 
	{
	    _ddt(CmdName(), Cmd(), CmdLength());
	    ec = dio.Exec(*this, scsi);
	    _d(Lvl_Info, "Probe result: %08lx", scsi); 
	    if (DebugDumpData()) 
		_ddt("Read data:", Data(), DataLength());
	    
	    /*
	    ** check if this probe is okay
	    */
	    ec = onProbe(ec, scsi);
	    if (!ec)
		break;
	}

	/*
	** actual command execution!
	*/
	_ddt(CmdName(), Cmd(), CmdLength());
	ec = dio.Exec(*this, scsi);
	_d(Lvl_Info, "Execution result: %s, %08lx", (iptr)(ec ? "OK" : "FAIL"), scsi); 
	if ((ec) && (DebugDumpData()))
	    _ddt("Read data:", Data(), DataLength());



	/*
	** see if we want to keep looping?
	*/
	if (!onLoop(ec, scsi))
	    break;
	DOS->Delay(20);
    } 
    while (true);

    /*
    ** we do not touch anything here
    ** unless we really want to fail command!
    ** (say: bad result)
    ** passing true if we were successful :)
    */
    onExit(ec, scsi);

    /*
    ** check if we aborted.
    */
    return ec;
}


