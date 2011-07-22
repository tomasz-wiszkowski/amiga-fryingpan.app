#include "Headers.h"
#include "Disc_DVD_PlusR.h"
#include "SCSI/scsi_GetConfiguration.h"

Disc_DVD_PlusR::Disc_DVD_PlusR(Drive &d) : Disc_DVD_ROM(d)
{
};

Disc_DVD_PlusR::~Disc_DVD_PlusR(void)
{
};

DriveStatus& Disc_DVD_PlusR::CloseDisc(DRT_Close type, int lTrackNo)
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
	cl.setType(cmd_Close::Close_DVDPlusR_Track, lTrackNo);
    }
    else if (type == DRT_Close_Session) 
    {
	Notify(result(DRT_Operation_Write_CloseSession));
	cl.setType(cmd_Close::Close_DVDPlusR_LastSession, 0);
    }
    else if (type == DRT_Close_Finalize) 
    {
	Notify(result(DRT_Operation_Write_CloseDisc));
	cl.setType(cmd_Close::Close_DVDPlusR_FinalizeNominal, 0);
    }

    cl.setImmediate(true);
    cl.Go();
    WaitOpComplete();

    RequestUpdate();
    return result;
}

DriveStatus& Disc_DVD_PlusR::OnChangeWriteMethod()
{
    DRT_WriteMethod dwm = GetWriteMethod();            // works exactly same as -R/RW version

    if (dwm == DRT_WriteMethod_Default)
    {
	return SetWriteMethod(DRT_WriteMethod_Packet);  // a small loopback
    }
    else if (dwm != DRT_WriteMethod_Packet)
    {
	SetWriteMethod(DRT_WriteMethod_Packet);         // reject
	return result.Complete(ODE_IllegalType);
    }

    Page<Page_Write> &pw = drive.GetWritePage();
    if (!pw.IsValid()) 
	return result.Complete(ODE_NoModePage);
    _D(Lvl_Debug, "Setting up DVD+R/+RW/-RAM parameters: packet writing enabled");
    pw->SetSessionFormat(Page_Write::Session_Data);
    pw->SetPacketSize(16);
    pw->SetDataMode(Page_Write::DataMode_Mode1);
    pw->SetWriteType(Page_Write::WriteType_Packet);
    pw->SetTrackMode(Page_Write::TrackMode_Data_Incremental);
    pw->SetMultisession(1);
    pw->SetLinkSize(0);
    return drive.SetPage(pw);
}

int Disc_DVD_PlusR::DiscSubType()
{
    Feat_DVD_PlusR_DualLayer *x;

    x = drive.GetFeatures().GetFeature<Feat_DVD_PlusR_DualLayer>();
    if (x == 0)
    {
	return DRT_SubType_Unknown;
    }

    if (x->IsCurrent()) {
	_D(Lvl_Debug, "Got DualLayer DVD+R media! Yuppie!");
	return DRT_SubType_DVD_DualLayer;
    } else {
	_D(Lvl_Debug, "Got SingleLayer DVD+R media..");
	return DRT_SubType_Unknown;
    }
}

bool Disc_DVD_PlusR::IsWritable()
{
    if (GetNextWritableTrack(0))
	return true;
    return false;
}

DriveStatus& Disc_DVD_PlusR::CheckItemData(const IOptItem *pDI)
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
	    _D(Lvl_Error, "DVD discs can not be masterized");
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
	    _D(Lvl_Error, "Only Data/Mode1 tracks allowed on DVD media");
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

    return Disc::CheckItemData(pDI);
}

