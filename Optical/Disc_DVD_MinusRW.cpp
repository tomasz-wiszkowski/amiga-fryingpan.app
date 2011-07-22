#include "Headers.h"
#include "Disc_DVD_MinusRW.h"
#include "SCSI/scsi_GetConfiguration.h"

Disc_DVD_MinusRW::Disc_DVD_MinusRW(Drive &d) : 
    Disc_DVD_MinusR(d),
    fmt(dio, result)
{
    meas = 0;
};

bool Disc_DVD_MinusRW::Init()
{
    if (false == Disc_DVD_MinusR::Init())
	return false;

    Notify(result(DRT_Operation_Analyse_ControlBlocks));

    fmt.ReadFormats();
    _D(Lvl_Info, "Disc formatted? : %ld", fmt.IsFormatted());
    _D(Lvl_Info, "Max capacity    : %ld blocks", fmt.GetMaxCapacity());

    return true;
}

Disc_DVD_MinusRW::~Disc_DVD_MinusRW(void)
{
};

bool Disc_DVD_MinusRW::IsFormatted(void)
{
    return fmt.IsFormatted();
}

bool Disc_DVD_MinusRW::IsOverwritable(void)
{
    Feature *x;

    if (DiscType() != DRT_Profile_DVD_MinusRW_Restricted)
	return 0;

    x = drive.GetFeatures().GetFeature<Feat_RigidOverwrite>();
    if (x)
	if (x->IsCurrent()) return 1;

    x = drive.GetFeatures().GetFeature<Feat_RestrictedOverwrite>();
    if (x)
	if (x->IsCurrent()) return 1;

    return 0;
}

DriveStatus& Disc_DVD_MinusRW::EraseDisc(DRT_Blank met)
{
    cmd_StartStopUnit    ssu(dio, result);
    SCSICommand*	prep = 0;
    Page<Page_Write> &pw = drive.GetWritePage();

    ssu.setType(cmd_StartStopUnit::StartStop_Start);
    ssu.Go();

    Calibrate();

    switch (met) 
    {
	case DRT_Blank_Erase_Complete:
	    {
		pw->SetWriteType(Page_Write::WriteType_SessionAtOnce);
		if (drive.SetPage(pw))
		    return result.Complete(ODE_CommandError);

		cmd_Blank *b = new cmd_Blank(dio, result);
		meas = drive.GetHardwareConfig()->DVDMinusInfo()->getCBlankMeas(GetDiscSize(), GetWriteSpeed());
		Notify(result(DRT_Operation_Erase_BlankComplete));
		b->setType(cmd_Blank::Blank_All, 0);
		b->setImmediate(true);
		prep = b;
	    }
	    break;

	case DRT_Blank_Erase_Fast:
	    {
		pw->SetWriteType(Page_Write::WriteType_SessionAtOnce);
		if (drive.SetPage(pw))
		    return result.Complete(ODE_CommandError);

		cmd_Blank *b = new cmd_Blank(dio, result);
		meas = drive.GetHardwareConfig()->DVDMinusInfo()->getQBlankMeas(GetDiscSize(), GetWriteSpeed());
		Notify(result(DRT_Operation_Erase_BlankFast));
		b->setType(cmd_Blank::Blank_Minimal, 0);
		b->setImmediate(true);
		prep = b;
	    }
	    break;

	case DRT_Blank_Format_Complete:
	    {
		pw->SetWriteType(Page_Write::WriteType_Packet);
		pw->SetPacketSize(16);
		if (drive.SetPage(pw))
		    return result.Complete(ODE_CommandError);

		scsi_Format* f = new scsi_Format(dio, result);
		meas = drive.GetHardwareConfig()->DVDMinusInfo()->getCFormatMeas(GetDiscSize(), GetWriteSpeed());
		Notify(result(DRT_Operation_Erase_FormatComplete));
		f->SetType(true, Format_FullFormat, GetDiscSize(), 16);
		f->SetImmediate(true);
		prep = f;
	    }
	    break;

	case DRT_Blank_Format_Fast:
	    {
		pw->SetWriteType(Page_Write::WriteType_Packet);
		pw->SetPacketSize(16);
		if (drive.SetPage(pw))
		    return result.Complete(ODE_CommandError);

		scsi_Format* f = new scsi_Format(dio, result);
		meas = drive.GetHardwareConfig()->DVDMinusInfo()->getQFormatMeas(GetDiscSize(), GetWriteSpeed());
		Notify(result(DRT_Operation_Erase_FormatComplete));
		f->SetType(true, Format_DVDM_QuickFormat, 0, 16);
		f->SetImmediate(true);
		prep = f;
	    }
	    break;

	case DRT_Blank_Default:
	    request("Info", "SORRY NO DEFAULT! F_I_X_M_E!", "OK", 0);
	    return result.Complete(ODE_IllegalCommand);

	default:
	    return result.Complete(ODE_IllegalParameter);
    }

    if (meas != 0)
	meas->begin();
    prep->Go();
    WaitOpComplete();
    if (meas != 0)
	meas->end();
    meas = 0;
    //   if (err == ODE_OK) Init();
    RequestUpdate();
    delete prep;
    return result;
}

int Disc_DVD_MinusRW::DiscType()
{
    if (fmt.IsFormatted())
	return DRT_Profile_DVD_MinusRW_Restricted;
    else
	return DRT_Profile_DVD_MinusRW_Sequential;
}

bool Disc_DVD_MinusRW::IsWritable()
{
    //   _D("Checking whether disc is formatted...");
    //   if (!IsFormatted()) 
    //      return false;
    return Disc_DVD_MinusR::IsWritable();
}

int16 Disc_DVD_MinusRW::GetOperationProgress()
{
    if (meas != 0)
    {
	return meas->getProgress();
    }
    return Disc::GetOperationProgress();
}

uint32 Disc_DVD_MinusRW::GetDiscSize()
{
    if (fmt.IsFormatted())
	return fmt.GetMaxCapacity();
    return Disc_DVD_MinusR::GetDiscSize();
}

