#include "Headers.h"
#include "Disc_BD_R.h"
#include "SCSI/scsi_GetConfiguration.h"

Disc_BD_R::Disc_BD_R(Drive &d) : Disc_BD_ROM(d)
{
    isRRM = false;
    isPOW = false;
};

Disc_BD_R::~Disc_BD_R(void)
{
};

bool Disc_BD_R::Init(void)
{
    Feat_BD_R_PseudoOverwrite *featPOW;
    Feat_WriteOnce *featWO;

    if (false == Disc_DVD_ROM::Init())
	return false;

    Notify(result(DRT_Operation_Analyse_ControlBlocks));

    featPOW = drive.GetFeatures().GetFeature<Feat_BD_R_PseudoOverwrite>();
    if (featPOW != 0)
    {
	if (featPOW->IsCurrent())
	    isPOW = true;
    }

    featWO = drive.GetFeatures().GetFeature<Feat_WriteOnce>();
    if (featWO != 0)
    {
	if (featWO->IsCurrent())
	    isRRM = true;
    }
    return true;
}

DriveStatus& Disc_BD_R::CloseDisc(DRT_Close type, int lTrackNo)
{
    cmd_Close cl(dio, result);

    Notify(result(DRT_Operation_Write_Synchronize));
    /* do not let immediate go up here. immediate flush causes problems. */
    cl.setType(cmd_Close::Close_FlushBuffers, 0);
    cl.Go();
    WaitOpComplete();

    if (type == DRT_Close_Track) 
    {
	Notify(result(DRT_Operation_Write_CloseTrack));
	cl.setType(cmd_Close::Close_BDR_Track, lTrackNo);
    }
    else if (type == DRT_Close_Session) 
    {
	Notify(result(DRT_Operation_Write_CloseSession));
	cl.setType(cmd_Close::Close_BDR_Session, 0);
    }
    else if (type == DRT_Close_Finalize) 
    {
	Notify(result(DRT_Operation_Write_CloseDisc));
	cl.setType(cmd_Close::Close_BDR_Finalize, 0);
    }

    cl.setImmediate(true);
    cl.Go();
    WaitOpComplete();

    RequestUpdate();
    return result;
}

DriveStatus& Disc_BD_R::OnChangeWriteMethod()
{
    DRT_WriteMethod dwm = GetWriteMethod();            // works exactly same as -R/RW version

    if (dwm == DRT_WriteMethod_Default)
    {
	return result = SetWriteMethod(DRT_WriteMethod_Packet);  // a small loopback
    }
    else if (dwm != DRT_WriteMethod_Packet)
    {
	SetWriteMethod(DRT_WriteMethod_Packet);         // reject
	return result.Complete(ODE_IllegalType);
    }

    Page<Page_Write> &pw = drive.GetWritePage();
    if (!pw.IsValid()) 
	return result.Complete(ODE_NoModePage);
    _D(Lvl_Debug, "Setting up BD-R parameters: packet writing enabled");
    pw->SetSessionFormat(Page_Write::Session_Data);
    pw->SetPacketSize(16);
    pw->SetDataMode(Page_Write::DataMode_Mode1);
    pw->SetWriteType(Page_Write::WriteType_Packet);
    pw->SetTrackMode(Page_Write::TrackMode_Data_Incremental);
    pw->SetMultisession(1);
    pw->SetLinkSize(0);
    return drive.SetPage(pw);
}

int Disc_BD_R::DiscSubType()
{
    /*
     * i have absolutely no idea how to determine disc sub type.
     * at least not yet.
     */
    return DRT_SubType_Unknown;
}

bool Disc_BD_R::IsWritable()
{
    if (GetNextWritableTrack(0))
	return true;
    return false;
}

DriveStatus& Disc_BD_R::CheckItemData(const IOptItem *pDI)
{
#warning INCREMENTAL TRACKS PASSED ON

    const_cast<IOptItem*>(pDI)->setIncremental(true);

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
    if (pDI->getPreGapSize() != 0)
    {
	_D(Lvl_Error, "Pad size other than 0 not allowed");
	return result.Complete(ODE_BadLayout);
    }

    if (pDI->getItemType() == Item_Disc)
    {
	if (pDI->getFlags() & DIF_Disc_MasterizeCD)
	{
	    _D(Lvl_Error, "BDR discs can not be masterized");
	    return result.Complete(ODE_BadLayout);
	}
	const_cast<IOptItem*>(pDI)->setSectorSize(2048);
    }
    else if (pDI->getItemType() == Item_Session)
    {
	const_cast<IOptItem*>(pDI)->setSectorSize(2048);
    }
    else if (pDI->getItemType() == Item_Track)
    {
	if (pDI->getDataType() != Data_Mode1)
	{
	    _D(Lvl_Error, "Only Data/Mode1 tracks allowed on blu ray media");
	    return result.Complete(ODE_BadTrackMode);
	}
	const_cast<IOptItem*>(pDI)->setSectorSize(2048);
	const_cast<IOptItem*>(pDI)->setIncremental(true);
    }
    else if (pDI->getItemType() == Item_Index)
    {
	_D(Lvl_Error, "Indices allowed only on CD media");
	return result.Complete(ODE_BadLayout);
    }

    return result = Disc::CheckItemData(pDI);
}
   
int Disc_BD_R::DiscType()
{
    if (isRRM)
	return DRT_Profile_BD_R_RandomWrite;
    if (isPOW)
	return DRT_Profile_BD_R_PseudoOverwrite;
    return DRT_Profile_BD_R_Sequential;
}

