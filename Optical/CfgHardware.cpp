#include "CfgHardware.h"
#include <Generic/XMLDocument.h>

static const char* Cfg_Hardware        = "Hardware";
static const char* Cfg_DriveID         = "DriveID";
static const char* Cfg_ReadSpeed       = "ReadSpeed";
static const char* Cfg_WriteSpeed      = "WriteSpeed";
static const char* Cfg_CDInfo          = "CDInfo";
static const char* Cfg_DVDPlusInfo     = "DVDPlusInfo";
static const char* Cfg_DVDMinusInfo    = "DVDMinusInfo";


CfgHardware::CfgHardware(ConfigParser *parent, int32 id)
{
   String *s;
   config   = new ConfigParser(parent, Cfg_Hardware, id);
   
   s  = config->getElement()->GetAttributeValue(Cfg_DriveID);
   if (s != 0)
      driveid = *s;
   else
      config->getElement()->AddAttribute(Cfg_DriveID, "");

   s  = config->getElement()->GetAttributeValue(Cfg_ReadSpeed);
   if (s != 0)
      readspd = s->ToLong();
   else
      config->getElement()->AddAttribute(Cfg_ReadSpeed, 65535);     // set the desired speed to max.

   s  = config->getElement()->GetAttributeValue(Cfg_WriteSpeed);
   if (s != 0)
      writespd = s->ToLong();
   else
      config->getElement()->AddAttribute(Cfg_WriteSpeed, 65535);    // as previously

   cdinfo         = new CfgCDInfo(config, Cfg_CDInfo);
   dvdplusinfo    = new CfgCDInfo(config, Cfg_DVDPlusInfo);
   dvdminusinfo   = new CfgCDInfo(config, Cfg_DVDMinusInfo);
}

CfgHardware::~CfgHardware()
{
   delete cdinfo;
   delete dvdplusinfo;
   delete dvdminusinfo;
   delete config;
}
   
void CfgHardware::onWrite()
{
   cdinfo->onWrite();
   dvdplusinfo->onWrite();
   dvdminusinfo->onWrite();
}

CfgCDInfo *CfgHardware::CDInfo()
{
   return cdinfo;
}

CfgCDInfo *CfgHardware::DVDPlusInfo()
{
   return dvdplusinfo;
}

CfgCDInfo *CfgHardware::DVDMinusInfo()
{
   return dvdminusinfo;
}

String &CfgHardware::getDriveID()
{
   return driveid;
}

void CfgHardware::setDriveID(const char* id)
{
   config->getElement()->FindAttribute(Cfg_DriveID)->SetValue(id);
   driveid = id;
}

uint16 CfgHardware::getReadSpeed()
{
   return readspd;
}

uint16 CfgHardware::getWriteSpeed()
{
   return writespd;
}

void CfgHardware::setReadSpeed(uint16 spd)
{
   config->getElement()->FindAttribute(Cfg_ReadSpeed)->SetValue(spd);
   readspd = spd;
}

void CfgHardware::setWriteSpeed(uint16 spd)
{
   config->getElement()->FindAttribute(Cfg_WriteSpeed)->SetValue(spd);
   writespd = spd;
}

