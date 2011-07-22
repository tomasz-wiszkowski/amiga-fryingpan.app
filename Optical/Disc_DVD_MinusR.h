#ifndef __DISC_DVD_MINUSR_H
#define __DISC_DVD_MINUSR_H

#include "Disc_DVD_ROM.h"

using namespace GenNS;

class Disc_DVD_MinusR : public Disc_DVD_ROM
{
   AutoPtrT<DVD_PreRecordedLeadIn> information;
public:
                           Disc_DVD_MinusR(class Drive &);
			   
   virtual                ~Disc_DVD_MinusR(void);

   virtual bool            Init();
   virtual DriveStatus&    OnChangeWriteMethod();  // this parses -RW as well
   virtual DriveStatus&    CheckItemData(const IOptItem *pDI);
   virtual DriveStatus&    CloseDisc(DRT_Close lType, int lTrackNo);
   virtual bool            IsWritable();


   //------------------------------------------------------------
   virtual bool            AllowMultiSessionLayout()           
   {  
       return true;                     
   };

   //------------------------------------------------------------
   virtual bool            AllowMultiTrackLayout()             
   {  
       return true;                     
   };

   //------------------------------------------------------------
   virtual int             SessionGapSize()                    
   {  
       return 7696;                     
   };

   //------------------------------------------------------------
   virtual int             TrackGapSize()                      
   {  
       return 16;                       
   };

   //------------------------------------------------------------
   virtual int             DiscType()                          
   {  
       return DRT_Profile_DVD_MinusR;   
   };

   //------------------------------------------------------------
   virtual int             DiscSubType()                       
   {  
       return 0;                        
   };

   //------------------------------------------------------------
   virtual DriveStatus&    LayoutTracks(const IOptItem *pDI)   
   {  
       return Disc::LayoutTracks(pDI);  
   };

   //------------------------------------------------------------
   virtual DriveStatus&    UploadLayout(const IOptItem *pDI)   
   {  
       return Disc::UploadLayout(pDI);  
   };

   //------------------------------------------------------------
   virtual DriveStatus&    BeginTrackWrite(const IOptItem*pDI) 
   {  
       return result.Complete();                   
   };

   //------------------------------------------------------------
   virtual DriveStatus&    EndTrackWrite(const IOptItem*)      
   {  
       return result.Complete(ODE_IllegalCommand);       
   };

};

#endif

