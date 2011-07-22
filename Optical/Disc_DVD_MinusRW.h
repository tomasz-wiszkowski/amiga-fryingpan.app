#ifndef __DISC_DVD_MINUSRW_H
#define __DISC_DVD_MINUSRW_H

#include "Disc_DVD_MinusR.h"
#include "SCSI/scsi_Format.h"

using namespace GenNS;

class Disc_DVD_MinusRW : public Disc_DVD_MinusR
{
    scsi_Format	    fmt;
    class CfgIOMeas*meas;
public:
    Disc_DVD_MinusRW(class Drive &);
    virtual                ~Disc_DVD_MinusRW(void);

    virtual bool            Init();
    virtual int             DiscType(void);
    virtual bool            IsFormatted(void);
    virtual bool            IsWritable(void);
    virtual bool            IsOverwritable(void);
    virtual DriveStatus&    EraseDisc(DRT_Blank);
    virtual int16           GetOperationProgress();
    virtual uint32          GetDiscSize();


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
    virtual bool            IsErasable(void)           
    {  
	return true;                     
    };

    //------------------------------------------------------------
    virtual bool            IsFormattable(void)         
    {  
	return true;                     
    };

    //------------------------------------------------------------
    virtual int             DiscSubType(void)          
    {  
	return 0;                        
    };

};


#endif
