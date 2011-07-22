#ifndef __DISC_CD_R_H
#define __DISC_CD_R_H

#include "Disc_CD_ROM.h"
#include "CDText.h"

using namespace GenNS;

class Disc_CD_R : public Disc_CD_ROM
{
    const IOptItem         *pLastTrack;
    int                     lSAOTrackGap;
    uint8                  *pCueSheet;
    int                     lCueElements;
    unsigned long           lLeadInLength;
    unsigned long           lLeadInStart;
    int                     lAdjustment;
    CDText                 *builder;
    bool                    cdText;
    bool			    bWriteSAO;
    bool			    bCDXA;
    bool			    bCDI;

protected:

    TOC_ATIP               *atip;

public:
    Disc_CD_R(class Drive &);
    virtual                ~Disc_CD_R(void);

    virtual bool            Init();
    virtual DriveStatus&    CloseDisc(DRT_Close type, int lTrackNo);
    virtual DriveStatus&    OnChangeWriteMethod();
    virtual int             SessionGapSize();
    virtual int             TrackGapSize();
    virtual DriveStatus&    BeginTrackWrite(const IOptItem*pDI);
    virtual DriveStatus&    CheckItemData(const IOptItem*);
    virtual int             DiscSubType(void);
    virtual bool            IsWritable(void);
    virtual DriveStatus&    UploadLayout(const IOptItem *pDI);
    virtual const char     *DiscVendor();
    virtual uint32          GetDiscSize();
    virtual bool            wantCDText() const;


    //----------------------------------------------------------------
    virtual DriveStatus&    EndTrackWrite(const IOptItem*pDI)
    {
	return result.Complete();
    }

    //----------------------------------------------------------------
    virtual bool            AllowMultiSessionLayout()
    {
	return true;
    }

    //----------------------------------------------------------------
    virtual bool            AllowMultiTrackLayout()
    {
	return true;
    }

    //----------------------------------------------------------------
    virtual bool            IsFormatted(void)
    {
	return false;
    }

    //----------------------------------------------------------------
    virtual bool            IsErasable(void)
    {
	return false;
    }

    //----------------------------------------------------------------
    virtual bool            IsFormattable(void)
    {
	return false;
    }

    //----------------------------------------------------------------
    virtual DriveStatus&    EraseDisc(int)
    {
	return result.Complete(ODE_IllegalCommand);
    }

    //----------------------------------------------------------------
    virtual DriveStatus&    FormatDisc(int)
    {
	return result.Complete(ODE_IllegalCommand);
    }

    //----------------------------------------------------------------
    virtual DriveStatus&    StructureDisc(void)
    {
	return result.Complete(ODE_IllegalCommand);
    }

    //----------------------------------------------------------------
    virtual int             DiscType(void)
    {
	return DRT_Profile_CD_R;
    }

    //----------------------------------------------------------------
    virtual int             LayoutAdjustment()
    {
	return lAdjustment;
    }

    //----------------------------------------------------------------
    virtual DriveStatus&    LayoutTracks(const IOptItem *pDI)
    {
	return Disc::LayoutTracks(pDI);
    }

    //----------------------------------------------------------------
    virtual bool            IsWriteProtected()
    {
	return false;
    }

    //----------------------------------------------------------------
    virtual void            SetWriteProtected(bool)
    {
    }

};


#endif

