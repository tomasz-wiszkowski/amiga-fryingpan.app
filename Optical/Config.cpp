#include "Config.h"

   static const char*   Cfg_Optical       =  "Optical";
   static const char*   Cfg_Location1     =  "ENV:FryingPan/Optical.prefs";
   static const char*   Cfg_Location2     =  "ENVARC:FryingPan/Optical.prefs";
   static const char*   Cfg_HardwareCount =  "HWCount"; 

Config::Config()
{
   config = new Configuration(Cfg_Optical);
   config->readFile(Cfg_Location1);

   vendors = new CfgVendors(config);
   drivers = new CfgDrivers(config);
   iocodes = new CfgIOCodes(config);
   hardware.FreeOnDelete(true);
   String *s = 0;
   s = config->getElement()->GetAttributeValue(Cfg_HardwareCount);
   if (s != 0)
   {
      hwcount = s->ToLong();
   }
   else
   {
      config->getElement()->AddAttribute(Cfg_HardwareCount, "");
      hwcount = 0;
   }

   for (int i=0; i<hwcount; i++)
   {
      CfgHardware *hw = new CfgHardware(config, i);
      hardware << hw;
   }
}

Config::~Config()
{
   onWrite();

   if (NULL != iocodes)
      delete iocodes;
   if (NULL != drivers)
      delete drivers;
   if (NULL != vendors)
      delete vendors;
   hardware.Empty();

   delete config;
}
   
void Config::onWrite()
{
   sync.Acquire();
   // iocodes and vendors do not use onWrite because the data is static.
   for (int i=0; i<hardware.Count(); i++)
   {
      hardware[i]->onWrite();
   }
   drivers->onWrite();

   config->getElement()->FindAttribute(Cfg_HardwareCount)->SetValue(hwcount);
   config->writeFile(Cfg_Location1);
   config->writeFile(Cfg_Location2);

   sync.Release();
}

CfgDrivers *Config::Drivers()
{
   return drivers;
}

CfgVendors *Config::Vendors()
{
   return vendors;
}

CfgIOCodes *Config::IOCodes()
{
   return iocodes;
}

CfgHardware *Config::GetHardware(const char* id)
{
   CfgHardware *h;

   for (int i=0; i<hardware.Count(); i++)
   {
      if (hardware[i]->getDriveID() == id)
         return hardware[i];
   }

   h = new CfgHardware(config, hwcount++);
   h->setDriveID(id);
   hardware << h;

   return h;
}


