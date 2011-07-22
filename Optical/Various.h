#ifndef __VARIOUS_H
#define __VARIOUS_H

#include "Internals.h"
#include <intuition/intuition.h>
#include <Generic/Generic.h>

class Transform
{
   public:
   static EDataType        CtlToTrackType(int);
   static int32            MsfToLba(int, int, int);
   static int32            MsfToLba(unsigned long);
   static unsigned long    TrackTypeToSectorSize(int);
   static int              CtlToProtection(int);
   static int              CtlToEmphasy(int);
};


#endif
