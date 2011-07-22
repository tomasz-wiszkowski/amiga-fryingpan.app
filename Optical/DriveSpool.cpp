#include "DriveClient.h"
#include "DriveSpool.h"
#include <LibC/LibC.h>
#include <libclass/dos.h>

DriveSpool* DriveSpool::Self()
{
   /* mark invalid situation */
    FAIL(DriveSpool::instance == 0, "Optical drive not initialized!")
	return 0;

    return DriveSpool::instance;
}
/*
** Initialize drivespool
*/
void DriveSpool::Init()
{
    FAIL(DriveSpool::instance != 0, "DriveSpool already instantiated!")
	return;

    DriveSpool::instance = new DriveSpool();
    DriveSpool::instance->deleting = false;
}

/*
** dispose drivespool
*/
void DriveSpool::Exit()
{
    DriveSpool* dsp = DriveSpool::Self();

    if (dsp == 0)
	return;

    dsp->deleting = true;

    while (true)
    {
	dsp->Lock();
	/*
	** take under consideration, that application may signal closing, while optical is still worknig.
	** in that case we want no messages to be printed.
	*/
	FAIL(dsp->drives.Count() != 0, "At least one task has still not released drive\nThis is odd, as Optical appears to be no longer in use.")
	{
	    dsp->Unlock();
	    DOS->Delay(5*60);   // wait 5 seconds;
	}
	else
	{
	    break;
	}
    }

    delete dsp;
}

/*
** Register Drive
*/
void DriveSpool::RegisterDrive(Drive* drv)
{
    drives << drv;
}

/*
** Unregister drive
*/
void DriveSpool::UnregisterDrive(Drive* drv)
{
    drives >> drv;
}

/*
** Find drive
**
** caveats:
** assumes the drivespool is locked and operational (it better be)
*/
Drive* DriveSpool::FindDrive(const char* drv, int unit)
{
    /*
    ** find matching drive
    */
    for (uint32 i=0; i<drives.Count(); i++)
    {
	if ((drives[i]->GetDeviceName().Equals(drv)) &&
		(drives[i]->GetUnitNumber() == unit))
	{
	    return drives[i];
	}
    }

    return 0;
}

/*
** find or create a drive
** return driveclient interface
*/
DriveClient* DriveSpool::GetDriveClient(const char* drv, int unit)
{
    DriveSpool* dsp = DriveSpool::Self();
    Drive *d = 0;
    DriveClient* dc = 0;

    FAIL(DriveSpool::instance->deleting == true, "Optical drive is closing.\nTry again later.")
	return 0;

    /* mark invalid situation */
    if (dsp == 0)
	return 0;

    dsp->Lock();
    d = dsp->FindDrive(drv, unit);

    /*
     * if no drive yet - create one
     */
    if (d == 0)
    {
	d = Drive::GetDrive(drv, unit);
	if (d != 0)
	    dsp->RegisterDrive(d);
    }

    if (d != 0)
	dc = d->getClient();

    dsp->Unlock();
    return dc;
}

/*
** this method is invoked by driveclient upon dispose.
*/
void DriveSpool::FreeDriveClient(DriveClient *dc)
{
    DriveSpool* dsp = DriveSpool::Self();

    /* mark invalid situation */
    if ((dsp == 0) || (dc == 0))
	return;

    Drive &d = dc->getDrive();
    dsp->Lock();
    if (d.freeClient(dc))
    {
	dsp->UnregisterDrive(&d);
	delete &d;
    }
    dsp->Unlock();
}

/*
** Lock drivespool
*/
void DriveSpool::Lock()
{
    DriveSpool *dsp = DriveSpool::Self();

    if (dsp)
	DriveSpool::instance->sync.Acquire();
}

/*
** Make a mark that we have a new registered drive
*/
void DriveSpool::DriveStarted()
{
    DriveSpool *dsp = DriveSpool::Self();

    FAIL(!dsp, "Critical, we've lost track on our drives!")
	return;

    FAIL(dsp->deleting, "This is not meant to happen! Please report");
    dsp->ref_count++;
}

/* 
** and release
*/
void DriveSpool::DriveStopped()
{
    DriveSpool *dsp = DriveSpool::Self();

    FAIL(!dsp, "Critical, we've lost track on our drives!")
	return;

    dsp->ref_count--;
}


/*
** Unlock drivespool
*/
void DriveSpool::Unlock()
{
    /* mark invalid situation */
    FAIL(DriveSpool::instance == 0, "DriveSpool not instantiated!")
	return;

    DriveSpool::instance->sync.Release();
}
