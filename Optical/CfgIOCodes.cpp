#include "CfgIOCodes.h"
   
static const char*   Cfg_IOCodes       =  "IOCodes";
static const char*   Cfg_IOCodeCode    =  "Code";
static const char*   Cfg_IOCodeQual    =  "Qualifier";
static const char*   Cfg_IOCodeMessage =  "Message";

CfgIOCodes::CfgIOCodes(ConfigParser *parent)
{
   XMLElement *xel;
   config = new ConfigParser(parent, Cfg_IOCodes);

   xel = config->getElement();

   iocodes.FreeOnDelete(true);

   for (int i=0; (xel!=0) && (i<xel->GetChildrenCount()); i++)
   {
      XMLElement *el = xel->GetChild(i);
      ASSERT (el != 0);
      if (el != 0)
      {
         String *c = el->GetAttributeValue(Cfg_IOCodeCode);
         String *q = el->GetAttributeValue(Cfg_IOCodeQual);
         String *m = el->GetAttributeValue(Cfg_IOCodeMessage);

         if ((c != 0) && (q != 0) && (m != 0))
         {
            IOCode *ioc = new IOCode;
            ioc->message   = *m;
            ioc->code      = c->ToLong();
            ioc->qual      = q->ToLong();

            iocodes << ioc;
         }
      }
   }
}

CfgIOCodes::~CfgIOCodes()
{
   delete config;
}

