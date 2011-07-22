#ifndef _OPTICAL_CFGIOCODES_H_
#define _OPTICAL_CFGIOCODES_H_

#include <Generic/ConfigParser.h>
#include <Generic/Vector.h>

using namespace GenNS;

class CfgIOCodes 
{
protected:
   struct IOCode
   {
      uint8       code;
      uint8       qual;
      String      message;
   };

protected:
   ConfigParser     *config;
   Vector<IOCode*>   iocodes;

public:
                     CfgIOCodes(ConfigParser *parent);
   virtual          ~CfgIOCodes();                     
};

#endif

