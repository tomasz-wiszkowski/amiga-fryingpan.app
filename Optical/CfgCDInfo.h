#ifndef _OPTICAL_CFGCDINFO_
#define _OPTICAL_CFGCDINFO_

#include <Generic/ConfigParser.h>
#include <Generic/Vector.h>
#include "CfgIOMeas.h"

using namespace GenNS;

class CfgCDInfo 
{
protected:
   ConfigParser        *config;
   Vector<CfgIOMeas*>   qblankspeeds;     // quick blank
   Vector<CfgIOMeas*>   qformatspeeds;    // quick format
   Vector<CfgIOMeas*>   cblankspeeds;     // complete blank
   Vector<CfgIOMeas*>   cformatspeeds;    // complete format

public:
                        CfgCDInfo(ConfigParser *parent, const char *cls);
   virtual             ~CfgCDInfo();
   virtual CfgIOMeas   *getQBlankMeas(uint32 size, uint16 spd);
   virtual CfgIOMeas   *getQFormatMeas(uint32 size, uint16 spd);
   virtual CfgIOMeas   *getCBlankMeas(uint32 size, uint16 spd);
   virtual CfgIOMeas   *getCFormatMeas(uint32 size, uint16 spd);

   virtual void         onWrite();
};

#endif

