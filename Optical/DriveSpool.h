#ifndef __DRIVESPOOL_H
#define __DRIVESPOOL_H

#include <Generic/Synchronizer.h>
#include <Generic/Debug.h>
#include "Drive.h"

using namespace GenNS;

class DriveClient;

class DriveSpool 
{
   static DriveSpool*	instance;
   Synchronizer		sync;
   VectorT<Drive*>	drives;
   bool			deleting;
   uint32		ref_count;

private:
                        DriveSpool() {};
                       ~DriveSpool() {};
    static DriveSpool*	Self();
    Drive*		FindDrive(const char*, int);
    void		RegisterDrive(Drive*);
    void		UnregisterDrive(Drive*);
    void		Lock();
    void		Unlock();
    
public:
    static void          Init();
    static void          Exit();
    static DriveClient*	 GetDriveClient(const char*, int);
    static void          FreeDriveClient(DriveClient*);
    static void		DriveStarted();
    static void		DriveStopped();
};

#endif

