#include "Headers.h"
#include "Disc_DVD_PlusRW.h"
#include "SCSI/scsi_GetConfiguration.h"

Disc_DVD_PlusRW::Disc_DVD_PlusRW(Drive &d) : 
    Disc_DVD_PlusR(d),
    fmt(dio, result)
{
    meas        = 0;
};

Disc_DVD_PlusRW::~Disc_DVD_PlusRW(void)
{
};

bool Disc_DVD_PlusRW::Init(void)
{
    if (false == Disc_DVD_PlusR::Init())
	return false;

    Notify(result(DRT_Operation_Analyse_ControlBlocks));

    fmt.ReadFormats();

    Page<Page_Write> &pw = drive.GetWritePage();
    pw->SetWriteType(Page_Write::WriteType_Packet);
    if (drive.SetPage(pw)) 
    {
	_D(Lvl_Error, "Unable to set up packet writing for DVD+RW/-RAM media!!!");
    }

    _D(Lvl_Info, "Disc formatted? : %ld", fmt.IsFormatted());
    _D(Lvl_Info, "Max capacity    : %ld blocks", fmt.GetMaxCapacity());

    return true;
}

DriveStatus& Disc_DVD_PlusRW::EraseDisc(DRT_Blank met)
{
    Page<Page_Write> &pw = drive.GetWritePage();
    pw->SetWriteType(Page_Write::WriteType_Packet);
    pw->SetPacketSize(16);
    if (drive.SetPage(pw))
	return result.Complete(ODE_CommandError);

    meas = drive.GetHardwareConfig()->DVDPlusInfo()->getCFormatMeas(GetDiscSize(), GetWriteSpeed());
    // all methods work same here.

    Calibrate();

    Notify(result(DRT_Operation_Erase_FormatComplete));
    if (meas != 0)
	meas->begin();

#warning tu powinno byc capacity
    fmt.SetType(true, Format_DVDP_FullFormat, GetDiscSize(), 0);
    fmt.Go();
    WaitOpComplete();

    if (meas != 0)
	meas->end();
    meas = 0;

    RequestUpdate();

    return result;
}

bool Disc_DVD_PlusRW::IsFormatted(void)
{
    return fmt.IsFormatted();
}

DriveStatus& Disc_DVD_PlusRW::CloseDisc(DRT_Close type, int)
{
    cmd_Close cl(dio, result);

    Notify(result(DRT_Operation_Write_Synchronize));
    cl.setType(cmd_Close::Close_FlushBuffers, 0);
    cl.Go();
    if (result != ODE_OK)
	return result;

    if (type == DRT_Close_Track) 
    {
	Notify(result(DRT_Operation_Write_CloseTrack));
	cl.setType(cmd_Close::Close_DVDPlusRW_DeIcing, 0);
    }
    else if (type == DRT_Close_Session) 
    {
	Notify(result(DRT_Operation_Write_CloseSession));
	cl.setType(cmd_Close::Close_DVDPlusRW_FinalizeCompatible, 0);
    }
    else if (type == DRT_Close_Finalize) 
    {
	Notify(result(DRT_Operation_Write_CloseDisc));
	cl.setType(cmd_Close::Close_DVDPlusRW_FinalizeCompatible, 0);
    }

    cl.Go();

    RequestUpdate();
    return result;
}

const IOptItem *Disc_DVD_PlusRW::GetNextWritableTrack(const IOptItem *di)
{
    _D(Lvl_Info, "DVD+RW/RAM Querying next writable track after %08lx", (int)di);

    const IOptItem *disc = GetContents();
    for (int i=0; i<disc->getChildCount(); i++)
    {
	const IOptItem *sess = disc->getChild(i);
	for (int j=0; j<sess->getChildCount(); j++)
	{
	    const IOptItem *trak = sess->getChild(j);

	    if (di == 0)
	    {
		disc->release();
		return trak;
	    }
	    if (di == trak)
		di = 0;
	}
    }

    disc->release();
    return 0;
}

int Disc_DVD_PlusRW::DiscSubType()
{
    Feat_DVD_PlusRW_DualLayer *x;

    x = drive.GetFeatures().GetFeature<Feat_DVD_PlusRW_DualLayer>();
    if (x == 0)
    {
	return DRT_SubType_Unknown;
    }

    if (x->IsCurrent()) {
	_D(Lvl_Debug, "Got DualLayer DVD+RW media! Yuppie!");
	return DRT_SubType_DVD_DualLayer;
    } else {
	_D(Lvl_Debug, "Got SingleLayer DVD+RW media..");
	return DRT_SubType_Unknown;
    }
}

int16 Disc_DVD_PlusRW::GetOperationProgress()
{
    if (meas != 0)
    {
	return meas->getProgress();
    }
    return Disc::GetOperationProgress();
}

uint32 Disc_DVD_PlusRW::GetDiscSize()
{
    if (fmt.IsFormatted())
	return fmt.GetMaxCapacity();
    return Disc_DVD_PlusR::GetDiscSize();
}

