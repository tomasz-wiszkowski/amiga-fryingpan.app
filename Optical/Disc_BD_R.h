#ifndef __DISC_BD_R
#define __DISC_BD_R

#include "Disc_BD_ROM.h"

using namespace GenNS;

class Disc_BD_R :public Disc_BD_ROM
{
    bool    isPOW;
    bool    isRRM;
public:
                           Disc_BD_R(class Drive &);
   virtual                ~Disc_BD_R(void);

   virtual DriveStatus&    OnChangeWriteMethod();
   virtual DriveStatus&    CheckItemData(const IOptItem *pDI);
   virtual bool            IsWritable();
   virtual int             DiscSubType();   
   virtual DriveStatus&    CloseDisc(DRT_Close lType, int lTrackNo);
   virtual bool		    Init();


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
   virtual int             DiscType();

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

