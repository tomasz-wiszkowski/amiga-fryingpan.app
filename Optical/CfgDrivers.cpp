#include "CfgDrivers.h"
   
static const char*   Cfg_Drivers       =  "Drivers";
static const char*   Cfg_Driver        =  "Driver";
static const char*   Cfg_DriverHarmful =  "Harmful";
static const char*   Cfg_DriverName    =  "Name";

CfgDrivers::CfgDrivers(ConfigParser *parent)
{
   XMLElement *xel;
   config = new ConfigParser(parent, Cfg_Drivers);

   xel = config->getElement();

   drivers.FreeOnDelete(true);

   for (int i=0; (xel!=0) && (i<xel->GetChildrenCount()); i++)
   {
      XMLElement *el = xel->GetChild(i);
      ASSERT (el != 0);
      if (el != 0)
      {
         String *h = el->GetAttributeValue(Cfg_DriverHarmful);
         String *n = el->GetAttributeValue(Cfg_DriverName);

         if ((h != 0) && (n != 0))
         {
            Driver *d = new Driver;
            d->driver   = *n;
            d->harmful  = (h->ToLong() != 0) ? true : false;

            drivers << d;  
         }
      }
   }
}

CfgDrivers::~CfgDrivers()
{
   delete config;
}

void CfgDrivers::onWrite()
{
   XMLElement *xel = config->getElement();

   while (xel->GetChildrenCount() > 0)
   {
      xel->RemChild(xel->GetChild(0));
   }

   for (int32 i=0; i<drivers.Count(); i++)
   {
      XMLElement *el = new XMLElement(Cfg_Driver, 0);
      el->AddAttribute(Cfg_DriverHarmful, drivers[i]->harmful ? 1 : 0);
      el->AddAttribute(Cfg_DriverName,    drivers[i]->driver);
      xel->AddChild(el);
   }
}

TriState CfgDrivers::isHarmful(const char *device)
{
   for (int32 i=0; i<drivers.Count(); i++)
   {
      if (drivers[i]->driver == device)
         return drivers[i]->harmful ? stTrue : stFalse;
   }
   return stUnknown;
}
   
void CfgDrivers::addDevice(const char *name, bool harmful)
{
   for (int32 i=0; i<drivers.Count(); i++)
   {
      if (drivers[i]->driver == name)
      {
         drivers[i]->harmful = harmful;
         return;
      }
   }

   Driver *n = new Driver;
   n->driver = name;
   n->harmful = harmful;
   drivers << n;
}

