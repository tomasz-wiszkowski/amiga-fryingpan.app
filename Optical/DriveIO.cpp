#include "DriveIO.h"
#include <exec/ports.h>
#include <exec/io.h>
#include "SCSI/uniform.h"

class DriveSense 
{
   struct _sense
   {
      struct _1 : protected aLong
      {
         bool     isValid()      { return getField(31, 1); }
         uint8    getCode()      { return getField(24, 7); }
         void     setCode(uint8 v) { setField(24, 7, v);   }
         bool     isFileMark()   { return getField(15, 1); }
         bool     isEndOfMedium(){ return getField(14, 1); }
         bool     isILI()        { return getField(13, 1); }
         uint8    getKey()       { return getField(8, 3);  }
      } primary;
      
      struct _2 : protected aLong
      {
         uint8    getLength()    { return getField(0, 8);  }
      } extended;
      
      aLong       specific;

      struct _3 : protected aWord
      {
         uint8    getASC()       { return getField(8, 8);  }
         uint8    getASCQ()      { return getField(0, 8);  }
      } secondary __attribute__((packed));

      struct _4 : protected aLong
      {
         uint8    getFRU()       { return getField(24, 8); }
         bool     isDataValid()  { return getField(23, 1); }
         uint32   getSenseData() { return getField(0, 23); }
      } sensedata __attribute__((packed));
   };

   _sense   *sense;

   public:

   DriveSense()
   {
       sense = new _sense;
   }

   ~DriveSense()
   {
       delete sense;
   }

   uint8*      SenseData()
   {
       return (uint8*)sense;
   }

   int         SenseLength()
   {
       return sizeof(_sense);
   }

   void        Prepare()
   {
       sense->primary.setCode(0);
   };

   int         Key()
   {
       if (sense->primary.getCode())
	   return sense->primary.getKey();
       else
	   return 0;
   };

   int         Code()
   {
       if (sense->primary.getCode())
	   return sense->primary.getCode();
       else
	   return 0;
   };

   int         ASC()
   {
       if (sense->primary.getCode())
	   return sense->secondary.getASC();
       else
	   return 0;
   };

   int         ASCQ()
   {
       if (sense->primary.getCode())
	   return sense->secondary.getASCQ();
       else
	   return 0;
   };

   uint32      SCSIError()
   {
       if (sense->primary.getCode())
	   return (Key()<<16) | (ASC()<<8) | (ASCQ());
       else
	   return 0;
   };
};







DriveIO::DriveIO()
{
    setDebug(0);
    opened      = false;
    ioport      = 0;
    ioreq       = 0;
    sense	= new DriveSense();

};

bool DriveIO::OpenDev(char* AName, int AUnit)
{
    CloseDev();

    ioport = ::Exec->CreateMsgPort();
    ioreq  = (IOStdReq*)::Exec->CreateIORequest(ioport, sizeof(*ioreq));
    _D(Lvl_Info, "Port: %08lx, Req: %08lx", (iptr)ioport, (iptr)ioreq);
    _D(Lvl_Info, "Attempting to open device %s, unit %ld...", (iptr)AName, AUnit);

    if (::Exec->OpenDevice(AName, AUnit, (IORequest*)ioreq, 0))
    {
	_D(Lvl_Warning, "OpenDevice() failed.");
	CloseDev();
	return false;
    }
    opened = true;

    return true;
}

void DriveIO::CloseDev()
{
    if (opened)       ::Exec->CloseDevice((IORequest*)ioreq);
    if (ioreq)        ::Exec->DeleteIORequest((IORequest*)ioreq);
    if (ioport)       ::Exec->DeleteMsgPort(ioport);

    opened      = false;
    ioport      = 0;
    ioreq       = 0;
}

DriveIO::~DriveIO()
{
    if (sense)        delete sense;
};

bool DriveIO::Exec(SCSICmd &scmd, uint32& sns)
{
    int rc;

    if (!opened)
	return false;

    ioreq->io_Error      = 0;
    ioreq->io_Actual     = 0;
    ioreq->io_Offset     = 0;
    ioreq->io_Command    = HD_SCSICMD;
    ioreq->io_Data       = &scmd;
    ioreq->io_Length     = sizeof(scmd);
    ioreq->io_Flags      = IOF_QUICK;
	    
    sense->Prepare();
    rc = ::Exec->DoIO((IORequest*)ioreq);
    _D(Lvl_Info, "Exec result: %ld", rc);
    _DDT("Sense", sense->SenseData(), sense->SenseLength());

    sns = sense->SCSIError();

    if (rc != 0)
	return false;
    if (sns != 0)
	return false;

    return true;
}

uint32 DriveIO::SCSIError()
{
    return sense->SCSIError();
}

uint8* DriveIO::SenseData()
{
    return sense->SenseData();
}

uint32 DriveIO::SenseLength()
{
    return sense->SenseLength();
}


