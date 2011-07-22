#ifndef __DISC_DVD_PLUSRW_H
#define __DISC_DVD_PLUSRW_H

#include "Disc_DVD_PlusR.h"
#include "SCSI/scsi_Format.h"

using namespace GenNS;

class Disc_DVD_PlusRW : public Disc_DVD_PlusR
{
protected:
   scsi_Format	    fmt;
   class CfgIOMeas *meas;

public:
                           Disc_DVD_PlusRW(class Drive &);
   virtual                ~Disc_DVD_PlusRW(void);

   virtual bool            Init();


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
   virtual bool            IsWritable()               
   {  
       return true;                  
   };

   //------------------------------------------------------------
   virtual bool            IsErasable()               
   {  
       return false;                 
   };

   //------------------------------------------------------------
   virtual bool            IsFormattable()             
   {  
       return true;                  
   };

   //------------------------------------------------------------
   virtual int             DiscType()                 
   {  
       return DRT_Profile_DVD_PlusRW;
   };

   //------------------------------------------------------------
   virtual bool            IsOverwritable()           
   {  
       return true;                  
   };

   virtual bool            IsFormatted();
   virtual DriveStatus&    EraseDisc(DRT_Blank);

   virtual const IOptItem *GetNextWritableTrack(const IOptItem *di);
   virtual DriveStatus&    CloseDisc(DRT_Close type, int lTrackNo);
   virtual int             DiscSubType();   
   virtual int16           GetOperationProgress();
   virtual uint32          GetDiscSize();
};

#endif

