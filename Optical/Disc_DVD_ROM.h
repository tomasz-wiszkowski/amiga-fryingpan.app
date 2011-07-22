#ifndef __DISC_DVD_ROM_H
#define __DISC_DVD_ROM_H

#include "Disc_Generic.h"
#include <Generic/VectorT.h>
#include "SCSI/scsi_DiscStructure.h"
#include <Generic/AutoPtrT.h>

using namespace GenNS;

class Disc_DVD_ROM : public Disc
{
protected:
    enum WriteProtection
    {
	WP_Via_SendDVD_0x30 =	1,
	WP_Via_SendDVD_0xC0 =	2,
	WP_Via_ModePage_0x1D =	4,
	WP_Via_Media	    =   8,
	WP_Via_Cartridge    =   16
    };

protected:
    scsi_DiscStructure		    rds;
    VectorT<uint32>		    dcbs;
    iptr			    writeprotectmethods;
    iptr			    activewriteprotect;
    AutoPtrT<DVD_DiscControlBlock>  cb;

protected:
    void                    CheckWriteProtected();
    virtual bool	    subInit();	// dvd / hddvd / bluray specific initialization
    virtual bool	    isDVD() { return true; }
    virtual bool	    isBD()  { return false; }
public:
    Disc_DVD_ROM(class Drive &);
    virtual                ~Disc_DVD_ROM(void);

    virtual bool            Init(void); // OUGHT TO BE INHERITED EVERYWHERE!
    virtual void            FillDiscSpeed(DiscSpeed&);

    virtual bool            IsWriteProtected();
    virtual void            SetWriteProtected(bool);
    virtual bool            AllowsWriteProtect();

    //------------------------------------------------------------
    virtual DriveStatus&    CheckItemData(const IOptItem*)
    {
	return result.Complete(ODE_DiscFull);
    }

    //------------------------------------------------------------
    virtual DriveStatus&    LayoutTracks(const IOptItem *)   
    {  
	return result.Complete(ODE_IllegalCommand);
    };

    //------------------------------------------------------------
    virtual bool            AllowMultiSessionLayout()  
    {  
	return false;              
    };

    //------------------------------------------------------------
    virtual bool            AllowMultiTrackLayout()    
    {  
	return false;              
    };
    
    //------------------------------------------------------------
    virtual int             SessionGapSize()           
    {  
	return 0;                  
    };

    //------------------------------------------------------------
    virtual int             TrackGapSize()             
    {  
	return 0;                  
    };

    //------------------------------------------------------------
    virtual bool            IsFormatted()              
    {  
	return false;              
    };

    //------------------------------------------------------------
    virtual bool            IsWritable()               
    {  
	return false;              
    };

    //------------------------------------------------------------
    virtual bool            IsErasable()               
    {  
	return false;              
    };

    //------------------------------------------------------------
    virtual bool            IsFormattable()             
    {  
	return false;              
    };

    //------------------------------------------------------------
    virtual bool            IsOverwritable()           
    {  
	return false;              
    };

    //------------------------------------------------------------
    virtual DriveStatus&    EraseDisc(DRT_Blank)             
    {  
	return result.Complete(ODE_IllegalCommand); 
    };

    //------------------------------------------------------------
    virtual int             DiscType()                 
    {  
	return DRT_Profile_DVD_ROM;
    };

    //------------------------------------------------------------
    virtual int             DiscSubType()              
    {  
	return 0;                  
    };

    //------------------------------------------------------------
    virtual int             LayoutAdjustment()         
    {  
	return 0;                  
    };

    //------------------------------------------------------------
    virtual DriveStatus&    BeginTrackWrite(const IOptItem*) 
    {  
	return result.Complete(ODE_IllegalCommand); 
    };

    //------------------------------------------------------------
    virtual DriveStatus&    EndTrackWrite(const IOptItem*)   
    {  
	return result.Complete(ODE_IllegalCommand); 
    };

};


#endif

