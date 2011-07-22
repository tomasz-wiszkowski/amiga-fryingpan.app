#ifndef __SCSI_SCSICOMMAND_H
#define __SCSI_SCSICOMMAND_H

#include <Generic/Types.h>
#include "../Optical.h"
#include <LibC/LibC.h>
#include "../DriveStatus.h"
#include "../DriveIO.h"
#include "libclass/dos.h"

#ifndef mc68000
#pragma pack(1)
#endif
#include <devices/scsidisk.h>
#ifndef mc68000
#pragma pack()
#endif

class SCSICommand : public SCSICmd
{
private:
    DriveStatus&    res;
    class DriveIO&  dio;

protected:
    const char*	    cmdname;
    uint8	    cmd[16];
    unsigned	    need_probe:1;
    unsigned	    dump_data:1;

protected:
public:

    SCSICommand(DriveIO &, DriveStatus& res);
    virtual		    ~SCSICommand();
    
    /*
    ** * never * overload *
    */
    bool Go();

    /*
    ** initialize. return true if okay.
    */
    virtual bool	    onInit() = 0;

    /*
    ** no returns here.
    ** cleanup and, if necessary, set flags.
    ** no need to call parent
    ** default: no-op
    */
    inline virtual void	    onExit(bool success, uint32 scsi)
    {
	/*
	** let's don't pretend:
	** this won't happen very often.
	*/
	if ((!success) || (scsi != 0))
	{
	    if (Result() == ODE_OK)
		Result().SCSIError(scsi);
	    _d(Lvl_Info, "%s: Command execution aborted.", (iptr)CmdName());
	    _d(Lvl_Info, "Error: %02ld, SCSIError: %06ld", (iptr)Result().Error(), Result().SCSIError());
	    return;
	}
#ifdef DEBUG
	FAIL(Result() != ODE_OK, "Wow, unexpected state for command %s.", (iptr)CmdName());
#endif
	Result().Complete();
	_d(Lvl_Info, "%s: Command execution complete.", (iptr)CmdName());
    }

    /*
    ** loop while busy, for example.
    ** return false to abort.
    ** default: wait 400ms and retry
    */
    inline virtual bool	    onLoop(bool ok, uint32 scsierr)
    {
	if (!ok)
	    return false;

	switch (scsierr)
	{
	    case 0x20404:
	    case 0x20407:
	    case 0x20408:
	    case 0x20000:
		return true;
	}
	return false;
    }
    
    /*
    ** return true to continue execution
    ** false to abort
    ** default: check result
    */
    inline virtual bool	    onProbe(bool ok, uint32 scsierr)
    {
	if ((!ok) || (scsierr != 0))
	{
	    Result().SCSIError(scsierr);
	    _d(Lvl_Warning, "Command probe failed: Exec: %d, SCSI: %06x", ok, scsierr);
	    return false;
	}

	return true;
    }

    inline uint8*	    Cmd()
    {
	return cmd;
    };

    inline uint8	    CmdLength()
    {
	return scsi_CmdLength;
    };

    inline const char*	    CmdName()
    {
	return cmdname;
    };

    inline bool 	    NeedProbe()
    {
	return need_probe;
    };

    inline uint8*	    Data()
    {
	return (uint8*)scsi_Data;
    };

    inline int32	    DataLength()
    {
	return scsi_Length;
    };
    
    inline DriveStatus&	    Result()
    {
	return res;
    };

    inline bool 	    DebugDumpData()
    {
	return dump_data;
    }

    inline DbgHandler*	    getDebug()
    {
	return dio.getDebug();
    }
};
// removed__attribute__((packed));


#endif

