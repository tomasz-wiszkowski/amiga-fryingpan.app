#ifndef __OPTICAL_FIFO_H
#define __OPTICAL_FIFO_H

#include "Optical.h"
#include <Generic/Types.h>

class OptFIFO : public IOptWrite
{
protected:
    uint8		    memory;	    //
    const class IOptItem*   optitem;	    // pointer to current *TRACK*
    const uint8		    blocksperchunk; // number of logical blocks that should be squeezed in a chunk (10, 20..)
public:

};

#endif

