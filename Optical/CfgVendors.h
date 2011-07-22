#ifndef _OPTICAL_CFGVENDORS_H_
#define _OPTICAL_CFGVENDORS_H_

#include <Generic/ConfigParser.h>
#include <Generic/Vector.h>

using namespace GenNS;

class CfgVendors 
{
protected:
   struct Vendor
   {
      uint8    m;
      uint8    s;
      uint8    f;
      bool     valid;
      String   name;
   };

protected:
   ConfigParser     *config;
   Vector<Vendor*>   vendors;
   String            novendor;

public:
                     CfgVendors(ConfigParser *parent);
   virtual          ~CfgVendors();                     
   virtual String   &getVendor(uint8 m, uint8 s, uint8 f);
};

#endif

