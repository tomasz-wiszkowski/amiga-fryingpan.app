#ifndef __DISC_DVD_RAM_H
#define __DISC_DVD_RAM_H

#include "Disc_DVD_PlusRW.h"

using namespace GenNS;

class Disc_DVD_RAM : public Disc_DVD_PlusRW
{
public:
                           Disc_DVD_RAM(class Drive &);
   virtual                ~Disc_DVD_RAM(void);

   virtual int             DiscType(void)             {  return DRT_Profile_DVD_RAM;   };
   virtual int             DiscSubType(void)          {  return 0;                     };

   virtual DriveStatus&    CloseDisc(DRT_Close type, int lTrackNo);
};

#endif

