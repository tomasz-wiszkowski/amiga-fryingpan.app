#include "CfgVendors.h"
#include <Generic/XMLDocument.h>

static const char*   Cfg_Vendors      =  "Vendors";
//static const char*   Cfg_CDVendor     =  "CDVendor";
static const char*   Cfg_CDVendorM    =  "Min";
static const char*   Cfg_CDVendorS    =  "Sec";
static const char*   Cfg_CDVendorF    =  "Frm";
static const char*   Cfg_VendorName   =  "Name";
static const char*   Cfg_VendorV      =  "Valid";

CfgVendors::CfgVendors(ConfigParser *parent)
{
   XMLElement *xel;

   config = new ConfigParser(parent, Cfg_Vendors);
   xel = config->getElement();

   novendor = "No vendor";
   vendors.FreeOnDelete(true);

   for (int i=0; (xel!=0) && (i<xel->GetChildrenCount()); i++)
   {
      XMLElement *el = xel->GetChild(i);
      ASSERT (el != 0);
      if (el != 0)
      {
         String *m = el->GetAttributeValue(Cfg_CDVendorM);
         String *s = el->GetAttributeValue(Cfg_CDVendorS);
         String *f = el->GetAttributeValue(Cfg_CDVendorF);
         String *v = el->GetAttributeValue(Cfg_VendorV);
         String *n = el->GetAttributeValue(Cfg_VendorName);

         if ((m != 0) && (s != 0) && (f != 0) && (v != 0) && (n != 0))
         {
            Vendor *ven = new Vendor;
            ven->m = m->ToLong();
            ven->s = s->ToLong();
            ven->f = f->ToLong();
            ven->valid = (v->ToLong() != 0) ? true : false;
            ven->name = *n;

            vendors << ven;
         }
      }
   }
}

CfgVendors::~CfgVendors()
{
   delete config;
}
   
String &CfgVendors::getVendor(uint8 m, uint8 s, uint8 f)
{
   for (int32 i=0; i<vendors.Count(); i++)
   {
      if ((vendors[i]->m == m) && (vendors[i]->s == s) && (vendors[i]->f == f))
         return vendors[i]->name;
   }
   return novendor;
}

