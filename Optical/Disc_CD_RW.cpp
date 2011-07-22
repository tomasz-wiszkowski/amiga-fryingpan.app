#include "Headers.h"
#include "Disc_CD_RW.h"
#include "Config.h"
#include "CfgCDInfo.h"
#include "SCSI/scsi_GetConfiguration.h"


Disc_CD_RW::Disc_CD_RW(Drive &d) : 
    Disc_CD_R(d),
    fmt(dio, result)
{
    fmt.ReadFormats();

    _D(Lvl_Info, "Disc formatted? : %ld", fmt.IsFormatted());
    _D(Lvl_Info, "Max capacity    : %ld blocks", fmt.GetMaxCapacity());

    meas        = 0;
};

Disc_CD_RW::~Disc_CD_RW(void)
{
};

bool Disc_CD_RW::Init()
{
    bool flag = Disc_CD_R::Init();
    const IOptItem* item = GetContents();

    if (fmt.IsFormatted() && (item != 0))
    {
	const_cast<IOptItem*>(item->getChild(0)->getChild(0))->setIncremental(true);
    }
    item->release();

    return flag;
}

DriveStatus& Disc_CD_RW::EraseDisc(DRT_Blank met)
{
    cmd_StartStopUnit	    ssu(dio, result);
    SCSICommand*	    prep = 0;
    DRT_Operation	    op;
    DRT_WriteMethod	    mtd = DRT_WriteMethod_TAO;
    
    meas = 0;

    ssu.setType(cmd_StartStopUnit::StartStop_Start);
    ssu.Go();

    if (NULL != atip)
    {
	switch (met)
	{
	    case DRT_Blank_Erase_Complete:
		{
		    mtd = DRT_WriteMethod_TAO;
		    op	= DRT_Operation_Erase_BlankComplete;

		    cmd_Blank* blk = new cmd_Blank(dio, result);
		    blk->setType(cmd_Blank::Blank_All, 0);
		    blk->setImmediate(true);
		    prep = blk;
		    if (0 != atip)
			meas = drive.GetHardwareConfig()->CDInfo()->getCBlankMeas(atip->GetLeadOutPos(), GetWriteSpeed());
		}
		break;

	    case DRT_Blank_Default:
	    case DRT_Blank_Erase_Fast:
		{
		    mtd = DRT_WriteMethod_TAO;
		    op	= DRT_Operation_Erase_BlankFast;

		    cmd_Blank* blk = new cmd_Blank(dio, result);
		    blk->setType(cmd_Blank::Blank_Minimal, 0);
		    blk->setImmediate(true);
		    prep = blk;
		    if (0 != atip)
			meas = drive.GetHardwareConfig()->CDInfo()->getQBlankMeas(atip->GetLeadOutPos(), GetWriteSpeed());
		}
		break;

	    case DRT_Blank_Erase_Session:
		{
		    mtd = DRT_WriteMethod_TAO;
		    op	= DRT_Operation_Erase_BlankSession;

		    cmd_Blank* blk = new cmd_Blank(dio, result);
		    blk->setType(cmd_Blank::Blank_Session, 0);
		    blk->setImmediate(true);
		    prep = blk;
		}
		break;

	    case DRT_Blank_Format_Complete:
	    case DRT_Blank_Format_Fast:
		{
		    mtd = DRT_WriteMethod_Packet;
		    op	= DRT_Operation_Erase_FormatComplete;

		    scsi_Format* blk = new scsi_Format(dio, result);

		    blk->SetType(true, Format_CD_DVD_FullFormat, GetDiscSize() & ~15, 16);
		    blk->SetImmediate(true);
		    prep = blk;
		    if (0 != atip)
			meas = drive.GetHardwareConfig()->CDInfo()->getCFormatMeas(atip->GetLeadOutPos(), GetWriteSpeed());
		}
		break;

	    default:
		return result.Complete(ODE_IllegalParameter);
	}
    }
    else
    {
	return result.Complete(ODE_IllegalCommand);
    }

    result = SetWriteMethod(mtd);
    if (result != ODE_OK) 
    {
	delete prep;
	return result;
    }
    /* TODO: CALIBRATING ? */
    Calibrate();

    if (0 != meas)
	meas->begin();

    Notify(result(op, DRT_OpStatus_InProgress));
    prep->Go();
    WaitOpComplete();
    Notify(result(op, DRT_OpStatus_Completed));

    if ((result == ODE_OK) && (0 != meas))
	meas->end();

    meas = 0;

    //   if (err == ODE_OK) Init();
    RequestUpdate();

    delete prep;

    return result;
}

DriveStatus& Disc_CD_RW::CloseDisc(DRT_Close lType, int lTrack)
{
    if (DiscType() != DRT_Profile_CD_MRW)
	return (result = Disc_CD_R::CloseDisc(lType, lTrack));

    cmd_Close *cl = new cmd_Close(dio, result);

    Notify(result(DRT_Operation_Write_Synchronize));
    cl->setType(cmd_Close::Close_FlushBuffers, 0);
    cl->Go();
    WaitOpComplete();

    if (lType == DRT_Close_Track) 
    {
	Notify(result(DRT_Operation_Write_CloseTrack));
	cl->setType(cmd_Close::Close_CDR_Track, lTrack);
    } 
    else if (lType == DRT_Close_Session) 
    {
	Notify(result(DRT_Operation_Write_CloseSession));
	cl->setType(cmd_Close::Close_CDMRW_StopFormat, 0);
    } 
    else if (lType == DRT_Close_Finalize) 
    {
	Notify(result(DRT_Operation_Write_CloseDisc));
	cl->setType(cmd_Close::Close_CDMRW_Finalize, 0);
    }

    cl->setImmediate(true);
    cl->Go();
    WaitOpComplete();
    delete cl;

    return result;
}

bool Disc_CD_RW::AllowMultiSessionLayout()
{
    if (DiscType() == DRT_Profile_CD_MRW)
	return false;
    else
	return Disc_CD_R::AllowMultiSessionLayout();
}

bool Disc_CD_RW::AllowMultiTrackLayout()
{
    if (DiscType() == DRT_Profile_CD_MRW)
	return false;
    else
	return Disc_CD_R::AllowMultiTrackLayout();
}

int Disc_CD_RW::SessionGapSize()
{
    if (DiscType() == DRT_Profile_CD_MRW)
	return 0;
    else
	return Disc_CD_R::SessionGapSize();
}

int Disc_CD_RW::TrackGapSize()
{
    if (DiscType() == DRT_Profile_CD_MRW)
	return false;
    else
	return Disc_CD_R::TrackGapSize();
}

int Disc_CD_RW::DiscType()
{
    if (IsFormatted()) 
	return DRT_Profile_CD_MRW;
    return DRT_Profile_CD_RW;
}

int Disc_CD_RW::DiscSubType()
{
    if (atip) 
    {
	_D(Lvl_Info, "Analysing ATIP data...");
	switch (atip->GetDiscSubType()) 
	{
	    case 0:
		_D(Lvl_Debug, "Got Low-Speed rewritable disc.");
		return DRT_SubType_CD_RW_LowSpeed;
	    case 1:
		_D(Lvl_Debug, "Got High-Speed rewritable disc.");
		return DRT_SubType_CD_RW_HighSpeed;
	    case 2:
		_D(Lvl_Debug, "Got Ultra-Speed rewritable disc.");
		return DRT_SubType_CD_RW_UltraSpeed;
	    default:
		_D(Lvl_Debug, "Unknown disc sub type %ld.", atip->GetDiscSubType());
		return DRT_SubType_Unknown;
	}
    } 
    else 
    {
	_D(Lvl_Debug, "No atip data, unknown disc sub type.");
	return DRT_SubType_Unknown;
    }
}

bool Disc_CD_RW::IsOverwritable(void)
{
    if (DiscType() == DRT_Profile_CD_MRW)
	return true;

    Feature *x;

    if (!IsFormatted())
	return false;

    x = drive.GetFeatures().GetFeature<Feat_RigidOverwrite>();
    if (x && x->IsCurrent()) 
	return true;

    x = drive.GetFeatures().GetFeature<Feat_RestrictedOverwrite>();
    if (x && x->IsCurrent()) 
	return true;

    return false;

};

bool Disc_CD_RW::IsWritable(void)
{
    if (DiscType() == DRT_Profile_CD_MRW)
	return true;
    return Disc_CD_R::IsWritable();
};

bool Disc_CD_RW::IsFormatted()
{
    return (fmt.IsFormatted());
}

bool Disc_CD_RW::IsFormattable()
{
    Feature *x = drive.GetFeatures().GetFeature<Feat_Formattable>();

    return (x && x->IsCurrent());
}

DriveStatus& Disc_CD_RW::CheckItemData(const IOptItem *pDI)
{
    if (DiscType() != DRT_Profile_CD_MRW)
    {
	return Disc_CD_R::CheckItemData(pDI);
    }

    if (pDI->getPreGapSize() != 0)
    {
	_D(Lvl_Error, "Pad size other than 0 not allowed");
	return result.Complete(ODE_BadLayout);
    }
    if (pDI->isIncremental())
    {
	_D(Lvl_Error, "Incremental tracks not allowed");
	return result.Complete(ODE_BadLayout);
    }
    if (pDI->hasCDText())
    {
	_D(Lvl_Error, "CD-Text allowed only on CD media");
	return result.Complete(ODE_BadLayout);
    }
    if (pDI->isPreemphasized())
    {
	_D(Lvl_Error, "Preemphasy allowed only on CD media");
	return result.Complete(ODE_BadLayout);
    }

    if (pDI->getItemType() == Item_Disc)
    {
	if (pDI->getFlags() && DIF_Disc_MasterizeCD)
	{
	    _D(Lvl_Error, "Disc can not be masterized");
	    return result.Complete(ODE_BadLayout);
	}
    }
    else if (pDI->getItemType() == Item_Session)
    {
    }
    else if (pDI->getItemType() == Item_Track)
    {
	if (pDI->getDataType() != Data_Mode1)
	{
	    _D(Lvl_Error, "Only Data/Mode1 tracks allowed on MRW media");
	    return result.Complete(ODE_BadLayout);
	}
	if (pDI->getSectorSize() != 2048)
	{
	    _D(Lvl_Error, "Invalid sector size");
	    return result.Complete(ODE_BadLayout);
	}
    }
    else if (pDI->getItemType() == Item_Index)
    {
	_D(Lvl_Error, "Indices not allowed");
	return result.Complete(ODE_BadLayout);
    }

    return Disc::CheckItemData(pDI);
} 

DriveStatus& Disc_CD_RW::BeginTrackWrite(const IOptItem*pDI)
{
    if (DiscType() != DRT_Profile_CD_MRW)
    {
	return Disc_CD_R::BeginTrackWrite(pDI);
    }
    const_cast<IOptItem*>(pDI)->setPreGapSize(0);
    return result.Complete();
}

int16 Disc_CD_RW::GetOperationProgress()
{
    if (meas != 0)
    {
	return meas->getProgress();
    }
    return Disc::GetOperationProgress();
}

uint32 Disc_CD_RW::GetDiscSize()
{
    if (fmt.IsFormatted())
	return fmt.GetMaxCapacity();
    return Disc_CD_R::GetDiscSize();
}

bool Disc_CD_RW::wantCDText() const
{
    return false;
}

