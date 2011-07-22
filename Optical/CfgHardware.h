#ifndef _OPTICAL_CFGHARDWARE_H_
#define _OPTICAL_CFGHARDWARE_H_

#include <Generic/Generic.h>
#include <Generic/ConfigParser.h>
#include "CfgCDInfo.h"

using namespace GenNS;

class CfgHardware
{
protected:
   String                  driveid;
   ConfigParser           *config;
   CfgCDInfo              *cdinfo;
   CfgCDInfo              *dvdminusinfo;
   CfgCDInfo              *dvdplusinfo;
   uint16                  readspd;
   uint16                  writespd;
public:
                           CfgHardware(ConfigParser *parent, int32 id);
   virtual                ~CfgHardware();
   virtual CfgCDInfo      *CDInfo();
   virtual CfgCDInfo      *DVDMinusInfo();
   virtual CfgCDInfo      *DVDPlusInfo();
   virtual String         &getDriveID();
   virtual void            setDriveID(const char*);
   virtual uint16          getReadSpeed();
   virtual uint16          getWriteSpeed();
   virtual void            setReadSpeed(uint16);
   virtual void            setWriteSpeed(uint16);

   virtual void            onWrite();
};

#endif

