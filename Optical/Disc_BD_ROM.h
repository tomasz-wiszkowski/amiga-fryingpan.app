#ifndef __DISC_BD_ROM_H
#define __DISC_BD_ROM_H

#include "Disc_DVD_ROM.h"

using namespace GenNS;

class Disc_BD_ROM : public Disc_DVD_ROM
{
protected:
    AutoPtrT<BD_DiscInformation>  bdid;
    AutoPtrT<BD_DiscDefinitionStructure>  bdds;
    AutoPtrT<BD_SpareAreaInformation>  bdsai;
    AutoPtrT<BD_RawDefectList> bdrdl;

protected:
    void                    CheckWriteProtected();
    virtual bool	    subInit();	// dvd / hddvd / bluray specific initialization
    virtual bool	    isDVD() { return false; }
    virtual bool	    isBD()  { return true; }
public:
    Disc_BD_ROM(class Drive &);
    virtual                ~Disc_BD_ROM(void);
    virtual bool            Init(void); // OUGHT TO BE INHERITED EVERYWHERE!
    virtual int             DiscType()                 {  return DRT_Profile_BD_ROM;};
    virtual void            FillDiscSpeed(DiscSpeed&);
};


#endif

