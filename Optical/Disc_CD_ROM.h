#ifndef __DISC_CD_ROM_H
#define __DISC_CD_ROM_H

#include "Disc_Generic.h"

using namespace GenNS;

struct cmd_ReadCD;

class Disc_CD_ROM : public Disc
{
   cmd_ReadCD             readCD;
   cmd_ReadSubChannel     readSub;
   SUB_MCN                *mcn;
   SUB_Q                  *subq;
   uint32                  freedbid;

protected:
   bool                    probeCDText();
   bool                    probeDataType();
   bool                    probeSubChannel();
   uint32                  probeFreeDBID();
   int                     sumDigits(int32 val);
   bool                    getSubChannelInfo(int32 pos, uint8 &trk, uint8 &idx);

public:
			                  Disc_CD_ROM(class Drive&);
   virtual                ~Disc_CD_ROM();

   virtual bool            Init();

   //----------------------------------------------------------------
   virtual DriveStatus&    LayoutTracks(const IOptItem *)
   {
       return result.Complete(ODE_IllegalCommand);
   };

   //----------------------------------------------------------------
   virtual bool            AllowMultiSessionLayout()
   {
       return false;
   };

   //----------------------------------------------------------------
   virtual bool            AllowMultiTrackLayout()
   {
       return false;
   };

   //----------------------------------------------------------------
   virtual int             SessionGapSize()
   {
       return 0;
   };

   //----------------------------------------------------------------
   virtual int             TrackGapSize()
   {
       return 0;
   };

   //----------------------------------------------------------------
   virtual bool            IsFormatted()
   {
       return false;
   };

   //----------------------------------------------------------------
   virtual bool            IsWritable()
   {
       return false;
   };

   //----------------------------------------------------------------
   virtual bool            IsErasable()
   {
       return false;
   };

   //----------------------------------------------------------------
   virtual bool            IsFormattable()
   {
       return false;
   };

   //----------------------------------------------------------------
   virtual bool            IsOverwritable()
   {
       return false;
   };

   //----------------------------------------------------------------
   virtual DriveStatus&    EraseDisc(DRT_Blank)
   {
       return result.Complete(ODE_IllegalCommand);
   };

   //----------------------------------------------------------------
   virtual int             DiscType()
   {
       return DRT_Profile_CD_ROM;
   };

   //----------------------------------------------------------------
   virtual int             DiscSubType()
   {
       return 0;
   };

   //----------------------------------------------------------------
   virtual int             LayoutAdjustment()
   {
       return 0;
   };

   //----------------------------------------------------------------
   virtual DriveStatus&    BeginTrackWrite(const IOptItem*)
   {
       return result.Complete(ODE_IllegalCommand);
   };

   //----------------------------------------------------------------
   virtual DriveStatus&    EndTrackWrite(const IOptItem*)
   {
       return result.Complete(ODE_IllegalCommand);
   }

   //----------------------------------------------------------------
   virtual DriveStatus&    CheckItemData(const IOptItem*)
   {
       return result.Complete(ODE_DiscFull);
   }

   virtual DriveStatus&    RandomRead(const IOptItem *i, int32 first, int32 count, void *buff);
   virtual void            FillDiscSpeed(DiscSpeed&);
};

#endif
