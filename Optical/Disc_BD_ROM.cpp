#include "Headers.h"
#include "Disc_BD_ROM.h"

Disc_BD_ROM::Disc_BD_ROM(Drive &d) : 
   Disc_DVD_ROM(d)
{
};

bool Disc_BD_ROM::Init()
{
    return Disc_DVD_ROM::Init();
}

bool Disc_BD_ROM::subInit()
{
   bdid = rds.ReadBDStructure<BD_DiscInformation>(0, 0);
   bdds = rds.ReadBDStructure<BD_DiscDefinitionStructure>(0, 0);
   bdsai = rds.ReadBDStructure<BD_SpareAreaInformation>(0, 0);
   bdrdl = rds.ReadBDStructure<BD_RawDefectList>(0, 0);
   //bdpac = rbd.ReadBDStructure<BD_PhysicalAccessControl>(0, 0);
   return true;
};

Disc_BD_ROM::~Disc_BD_ROM(void)
{
};

void Disc_BD_ROM::FillDiscSpeed(DiscSpeed &spd)
{
   spd.begin_f = (((int32)100*spd.begin_kbps)/44955)%10;
   spd.begin_i = ((int32)10*spd.begin_kbps)/44955;
   spd.end_f = (((int32)100*spd.end_kbps)/44955)%10;
   spd.end_i = ((int32)10*spd.end_kbps)/44955;
}

