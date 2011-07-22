#ifndef _OPTICAL_CFGIOMEAS_H_
#define _OPTICAL_CFGIOMEAS_H_

#include <Generic/Types.h>

class CfgIOMeas 
{
protected:
   uint32         discsize;
   uint16         iospeed;
   uint16         count;
   uint16         time;

   uint32         ctime;
   uint32         ctenth;
public:
                  CfgIOMeas(uint32 ds, uint16 io, uint16 cnt=0, uint32 time=0);
   virtual       ~CfgIOMeas();
   virtual void   setTime(uint16 time);
   virtual uint16 getTime();
   virtual uint32 getDiscSize();
   virtual uint16 getIOSpeed();
   virtual uint16 getCount();

   virtual void   begin();             // initialize var for time count
   virtual void   end();               // calculate time & store it
   virtual uint16 getProgress();       // returns value from 0 to 100
};

#endif

