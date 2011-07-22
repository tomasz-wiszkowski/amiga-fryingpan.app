#ifndef __DISC_CD_RW_H
#define __DISC_CD_RW_H

#include "Disc_CD_R.h"
#include "SCSI/scsi_Format.h"

using namespace GenNS;

class Disc_CD_RW : public Disc_CD_R
{
   scsi_Format	    fmt;
   class CfgIOMeas *meas;

public:
                           Disc_CD_RW(class Drive &);
   virtual                ~Disc_CD_RW(void);

   //------------------------------------------------------------
   virtual bool            IsErasable()           
   {  
       return true;               
   };

   //------------------------------------------------------------
   virtual DriveStatus&    EndTrackWrite(const IOptItem*pDI)
   {
       return result.Complete();
   }

   virtual bool            Init();

   virtual bool            IsFormattable();
   virtual bool            IsOverwritable();
   virtual bool            IsFormatted();
   virtual bool            IsWritable();
   virtual int             DiscType();
   virtual int             DiscSubType();
   virtual bool            AllowMultiSessionLayout();
   virtual bool            AllowMultiTrackLayout();
   virtual int             SessionGapSize();
   virtual int             TrackGapSize();
   virtual DriveStatus&    BeginTrackWrite(const IOptItem *pDI);

   virtual DriveStatus&    CloseDisc(DRT_Close type, int lTrackNo);

   virtual DriveStatus&    CheckItemData(const IOptItem*);
   virtual DriveStatus&    EraseDisc(DRT_Blank);

   virtual int16           GetOperationProgress();
   virtual uint32          GetDiscSize();
   virtual bool            wantCDText() const;
};


#endif
