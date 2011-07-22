#ifndef _OPTICAL_DRIVESTATUS_H
#define _OPTICAL_DRIVESTATUS_H

#include <Generic/Types.h>
#include "Optical.h"
#include "IDriveClient.h"


class DriveStatus : public IDriveStatus
{
public:
    DriveStatus(DRT_Operation o, 
		DRT_OperationStatus s=DRT_OpStatus_Completed, 
		uint16 maj = 0,
		uint16 min = 0,
		EOpticalError e=ODE_OK, 
		uint32 se = 0)
    {
	(*this)(o, s, maj, min, e, se);
    }

    DriveStatus& operator()(DRT_Operation o,
		DRT_OperationStatus s=DRT_OpStatus_Completed, 
		uint16 maj = 0,
		uint16 min = 0,
		EOpticalError e=ODE_OK, 
		uint32 se = 0)
    {
	operation = o;
	status = s;
	error = e;
	progress_minor = min;
	progress_major = maj;
	scsi_error = se;
	description = 0;

	return *this;
    }    

    DriveStatus& Init(DRT_Operation o)
    {
	/*
	** set up for new operation
	*/
	operation   = o;
	error	    = ODE_OK;
	status      = DRT_OpStatus_InProgress;
	scsi_error  = 0;
	description = 0;
	progress_minor = 0;
	progress_major = 0;

	return *this;
    }

    DriveStatus& Complete(EOpticalError e = ODE_OK, uint32 scsie = 0)
    {
	/*
	** don't touch operation here.
	*/
	if ((e != ODE_OK) || (scsie != 0))
	{
	    if (scsie != 0)
		e = ODE_CommandError;
	    status = DRT_OpStatus_Aborted;
	}
	else
	{
	    status = DRT_OpStatus_Completed;
	}

	error	    = e;
	scsi_error  = scsie;
	description = 0;
	progress_minor = Progress_Max;	/* sub progress: maximum */
	progress_major = Progress_Max;
	return *this;
    }

    DriveStatus& Progress(uint16 maj, uint16 min)
    {
	progress_major = maj;
	progress_minor = min;
	return *this;
    }

    operator const EOpticalError()
    {
	return error;
    }

    /*
    ** copy contents of other result
    */
    DriveStatus& operator=(const DriveStatus &o)
    {
	operation	= o.operation;
	status		= o.status;
	error		= o.error;
	scsi_error	= o.scsi_error;
	description	= o.description;
	progress_major	= o.progress_major;
	progress_minor	= o.progress_minor;
	return *this;
    }

    /*
    ** we don't want anyone to mess it up
    */
    inline const uint32& SCSIError()
    {
	return scsi_error;
    }

    inline DriveStatus& SCSIError(uint32 err)
    {
	if (err != 0)
	{
	    error = ODE_CommandError;
	    status = DRT_OpStatus_Aborted;
	}
	else
	{
	    error = ODE_OK;
	    status = DRT_OpStatus_Completed;
	}
	scsi_error = err;

	return *this;
    }

    /*
    ** we don't want anyone to mess it up
    */
    inline const EOpticalError& Error()
    {
	return error;
    }

    /*
    ** reset stuff
    ** mind that whole optical relies on these values.
    */
    inline DriveStatus& Reset()
    {
	operation = DRT_Operation_Ready;
	progress_minor = 0;
	progress_major = 0;
	error = ODE_OK;
	scsi_error = 0;
	status = DRT_OpStatus_Completed;
	description = 0;
	return *this;
    }
};

#endif
