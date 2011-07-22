#include "CfgIOMeas.h"
#include <Generic/LibrarySpool.h>
#include <libclass/intuition.h>

CfgIOMeas::CfgIOMeas(uint32 ds, uint16 io, uint16 cnt, uint32 t)
{
   discsize = ds;
   iospeed  = io;
   count    = cnt;
   time     = t;
}

CfgIOMeas::~CfgIOMeas()
{
}

void   CfgIOMeas::setTime(uint16 t)
{
   uint32 val;

   val  = (count * time) + t;
   ++count;
   val /= count;
   time = val;
}

uint16 CfgIOMeas::getTime()
{
   return time;
}

uint32 CfgIOMeas::getDiscSize()
{
   return discsize;
}

uint16 CfgIOMeas::getIOSpeed()
{
   return iospeed;
}

uint16 CfgIOMeas::getCount()
{
   return count;
}

void CfgIOMeas::begin()
{
   Intuition->CurrentTime(&ctime, &ctenth);  
   ctenth /= 100000;
}

void CfgIOMeas::end()
{
   uint32 t1, t2;

   Intuition->CurrentTime(&t1, &t2);
   t2 /= 100000;

   t1 -= ctime;
   t2 -= ctenth;

   t1 *= 10;
   t1 += t2;
   t1 += 9;
   t1 /= 10;
   setTime(t1);
}

uint16 CfgIOMeas::getProgress()
{
   int val = 0;
   
   if (time == 0)
      return 0;

   uint32 t1, t2;
   Intuition->CurrentTime(&t1, &t2);

   t2 /= 100000;
   t2 -= ctenth;
   t1 -= ctime;

   t1  = t1*10+t2;

   if (t1 < (unsigned)(time * 10))
      val = (t1 << 16)/(time * 10);
   else
      val = 65535;
   return val;
}

