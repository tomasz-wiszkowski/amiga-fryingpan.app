#ifndef __DISC_GENERIC_H
#define __DISC_GENERIC_H

#include "Drive.h"
#include "Disc_Item.h"
#include "OptDisc.h"
#include <Generic/Vector.h>
#include <Generic/VectorT.h>
#include "IOptItem.h"

using namespace GenNS;

struct cmd_Read;
struct cmd_Write;

class Disc 
{
private:
    int64                         lSeqSector;                // sequential recording - sector number of current track
    int64                         lSeqLastSector;            // sequential recording - last sector to be written for current track
    const IOptItem               *pSeqStructure;             // disc structure
    const IOptItem               *pSeqCurrentTrack;          // current track;
    cmd_Read                     *pRead;
    cmd_Write                    *pWrite;

    unsigned short                read_speed;
    unsigned short                write_speed;
    DRT_WriteMethod               write_method;
    unsigned char                 refresh_period;
    bool                          bNeedUpdate;

    VectorT<DiscSpeed>            readSpeeds;
    VectorT<DiscSpeed>            writeSpeeds;

    OptDisc                      *optcontent;

    int16                         laySessions;
    int16                         layTracks;
    int16                         layFirstTrack;
    int16                         layFirstSession;
    int64                         layCurrentBlock;


private:
    bool                          subinit(); 

protected:
    DiscInfo*		    pDiscInfo;

protected:
    DEFINE_DEBUG;
    Drive&		    drive;
    DriveIO&		    dio;
    DriveStatus&	    result;
    cmd_ReadDiscInfo        cRDI;

protected:

    Disc(Drive &);
    void                          RequestUpdate();
    void                          SetRefreshPeriod(UBYTE fiftieths);
    DriveStatus&                  WritePadData(const IOptItem *pDI, int32 len);
    DriveStatus&                  WriteDisc(int lBlock, unsigned short lLength, unsigned short lSectorSize, APTR pBuffer);

    /*
    ** speeds!
    */
    bool	    ReadSpeeds_perfBased();
    bool	    ReadSpeeds_pageBased();
    bool	    ReadSpeeds_selectBased();
    bool	    ReadSpeeds_selectOldBased();
    void            ReadSpeeds();

    /*
    ** tracks!
    */
    bool            ReadTracks_TrackInfo();
    bool            ReadTracks_TOC();
    bool            ReadTracks_PrimitiveTOC();
    bool	    ReadTracks();


    //--------------------------------------------------------------------------------
    inline void         Notify(DriveStatus& s)
    {
	drive.notify(s);
    }
public:
    static Disc*                  GetDisc(Drive &drv, DRT_Profile type);

    virtual                      ~Disc(void);


    //=======================//
    // now the methods...    //
    //=======================//

    virtual bool                  AllowMultiSessionLayout()  = 0;
    virtual bool                  AllowMultiTrackLayout()    = 0;
    virtual int                   SessionGapSize()           = 0;
    virtual int                   TrackGapSize()             = 0;
    virtual int                   LayoutAdjustment()         = 0;
    virtual bool                  IsFormatted()              = 0;
    virtual bool                  IsWritable()               = 0;
    virtual bool                  IsErasable()               = 0;
    virtual bool                  IsFormattable()            = 0;
    virtual bool                  IsOverwritable()           = 0;

    virtual DriveStatus&          EraseDisc(DRT_Blank)       = 0;
    virtual int                   DiscType()                 = 0;
    virtual int                   DiscSubType()              = 0;
    virtual bool                  Init()                     = 0;
    virtual DriveStatus&          CheckItemData(const IOptItem*);
    virtual DriveStatus&          BeginTrackWrite(const IOptItem*) = 0;     // for writing
    virtual DriveStatus&          EndTrackWrite(const IOptItem*)   = 0;
    virtual void                  FillDiscSpeed(DiscSpeed&)  = 0;

    virtual const char           *DiscVendor();

    virtual DriveStatus&          WriteData(const IOptItem*);

    virtual const uint8           GetRefreshPeriod(void);


    virtual DriveStatus&          RandomRead(const IOptItem*,  int32, int32, void*);
    virtual DriveStatus&          RandomWrite(const IOptItem*, int32, int32, void*);
    virtual DriveStatus&          SequentialWrite(APTR pMem, ULONG lBlocks);
    virtual DriveStatus&          CloseDisc(DRT_Close lType, int lTrackNo);

    virtual DriveStatus&          LayoutTracks(const IOptItem*);
    virtual DriveStatus&          UploadLayout(const IOptItem*);
    virtual const IOptItem       *GetNextWritableTrack(const IOptItem*);

    virtual uint32                GetNumTracks(void);
    virtual uint32                GetNumSessions(void);

    virtual const IOptItem	 *GetContents();

    virtual const IOptItem       *FindSession(int lSessNum);
    virtual const IOptItem       *FindTrack(int lTrackNum);

    virtual DriveStatus&          SetWriteSpeed(uint16);
    virtual DriveStatus&          SetReadSpeed(uint16);
    virtual uint16                GetWriteSpeed();
    virtual uint16                GetReadSpeed();
    virtual bool                  RequiresUpdate();

    virtual const DiscSpeed      *GetReadSpeeds();
    virtual const DiscSpeed      *GetWriteSpeeds();

    virtual DriveStatus&          SetWriteMethod(DRT_WriteMethod);
    virtual DRT_WriteMethod       GetWriteMethod();
    virtual DriveStatus&          OnChangeWriteMethod();

    virtual int16                 GetOperationProgress();
    virtual uint32                GetDiscSize();

    virtual DriveStatus&          Calibrate();
    virtual bool		  WaitOpComplete(iptr period=25);
    virtual bool                  IsWriteProtected() { return true; }
    virtual void                  SetWriteProtected(bool) {};
    virtual bool                  AllowsWriteProtect() { return false; }
};

#endif
