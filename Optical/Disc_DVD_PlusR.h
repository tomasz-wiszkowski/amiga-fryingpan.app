#ifndef __DISC_DVD_PlusR
#define __DISC_DVD_PlusR

#include "Disc_DVD_ROM.h"

using namespace GenNS;

class Disc_DVD_PlusR :public Disc_DVD_ROM
{
public:
                           Disc_DVD_PlusR(class Drive &);
   virtual                ~Disc_DVD_PlusR(void);

   virtual DriveStatus&    OnChangeWriteMethod();
   virtual DriveStatus&    CheckItemData(const IOptItem *pDI);
   virtual bool            IsWritable();
   virtual int             DiscSubType();   
   virtual DriveStatus&    CloseDisc(DRT_Close lType, int lTrackNo);

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
       return 0;                        
   };

   //------------------------------------------------------------
   virtual int             TrackGapSize()                      
   {  
       return 0;                        
   };

   //------------------------------------------------------------
   virtual int             DiscType()                          
   {  
       return DRT_Profile_DVD_PlusR;    
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

