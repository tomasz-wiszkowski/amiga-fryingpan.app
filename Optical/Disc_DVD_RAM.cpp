#include "Headers.h"
#include "Disc_DVD_RAM.h"

Disc_DVD_RAM::Disc_DVD_RAM(Drive &d) : Disc_DVD_PlusRW(d)
{
};

Disc_DVD_RAM::~Disc_DVD_RAM(void)
{
};

DriveStatus& Disc_DVD_RAM::CloseDisc(DRT_Close type, int)
{
   cmd_Close cl(dio, result);
   
   Notify(result(DRT_Operation_Write_Synchronize));
   
   cl.setType(cmd_Close::Close_FlushBuffers, 0);
   cl.Go();
   WaitOpComplete();
   
   RequestUpdate();
   return result;
}

