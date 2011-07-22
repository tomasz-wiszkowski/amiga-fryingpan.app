#ifndef _OPTICAL_CFGDRIVERS_H_
#define _OPTICAL_CFGDRIVERS_H_

#include <Generic/ConfigParser.h>
#include <Generic/Vector.h>

using namespace GenNS;

class CfgDrivers 
{
protected:
   struct Driver
   {
      String      driver;
      bool        harmful;
   };

protected:
   ConfigParser     *config;
   Vector<Driver*>   drivers;

public:
                     CfgDrivers(ConfigParser *parent);
   virtual          ~CfgDrivers();                     
   virtual TriState  isHarmful(const char *device);
   virtual void      addDevice(const char *name, bool harmful);

   virtual void      onWrite();
};

#endif

