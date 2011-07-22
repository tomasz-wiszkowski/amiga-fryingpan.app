#ifndef _OPTICAL_DRIVEIO_H_
#define _OPTICAL_DRIVEIO_H_

#include <Generic/Types.h>
#include <Generic/Debug.h>
#include <devices/scsidisk.h>

class DriveSense;
struct IOStdReq;
struct MsgPort;

using namespace GenNS;

class DriveIO 
{
    MsgPort       *ioport;
    IOStdReq      *ioreq;
    bool           opened;
    DriveSense    *sense;
    DEFINE_DEBUG;

public:
    DriveIO();
    ~DriveIO(void);
    bool   OpenDev(char* AName, int AUnit);
    void   CloseDev();
    bool   Exec(SCSICmd&, uint32& scsierr);
    uint32 SCSIError();
    uint8* SenseData();
    uint32 SenseLength();

    bool IsOpened()
    {  
	return opened;                 
    };

    inline DbgHandler* getDebug()
    {
	return DEBUG_ENGINE;
    }

    inline void setDebug(DbgHandler* h)
    {
	SET_DEBUG_ENGINE(h);
    }

};


#endif
