#ifndef _OPTICAL_CONFIG_H_
#define _OPTICAL_CONFIG_H_

#include <Generic/Configuration.h>
#include <Generic/Vector.h>
#include "CfgDrivers.h"
#include "CfgVendors.h"
#include "CfgIOCodes.h"
#include "CfgCDInfo.h"
#include "CfgHardware.h"
#include <Generic/Synchronizer.h>

using namespace GenNS;

class Config 
{
protected:
   Configuration          *config;
   CfgDrivers             *drivers;
   CfgVendors             *vendors;
   CfgIOCodes             *iocodes;
   int32                   hwcount;
   Vector<CfgHardware*>    hardware;
   Synchronizer            sync;

public:
                           Config();
   virtual                ~Config();
   virtual CfgDrivers     *Drivers();
   virtual CfgVendors     *Vendors();
   virtual CfgIOCodes     *IOCodes();
   virtual CfgHardware    *GetHardware(const char *id);

   virtual void            onWrite();
};

#endif

