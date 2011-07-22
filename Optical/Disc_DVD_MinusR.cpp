#include "Headers.h"
#include "Disc_DVD_MinusR.h"

Disc_DVD_MinusR::Disc_DVD_MinusR(Drive &d) : Disc_DVD_ROM(d)
{
};

bool Disc_DVD_MinusR::Init(void)
{
   if (false == Disc_DVD_ROM::Init())
      return false;

   Notify(result(DRT_Operation_Analyse_ControlBlocks));
   information = rds.ReadDVDStructure<DVD_PreRecordedLeadIn>(0, 0);

   return true;
}

Disc_DVD_MinusR::~Disc_DVD_MinusR(void)
{
};

DriveStatus& Disc_DVD_MinusR::OnChangeWriteMethod()
{
   DRT_WriteMethod dwm = GetWriteMethod();

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
   _D(Lvl_Debug, "Setting up DVD-R/RW parameters: packet writing enabled");
   pw->SetSessionFormat(Page_Write::Session_Data);
   pw->SetPacketSize(16);
   pw->SetDataMode(Page_Write::DataMode_Mode1);
   pw->SetWriteType(Page_Write::WriteType_Packet);
   pw->SetTrackMode(Page_Write::TrackMode_Data_Incremental);
   pw->SetMultisession(1);
   pw->SetLinkSize(0);
   return drive.SetPage(pw);
}

DriveStatus& Disc_DVD_MinusR::CloseDisc(DRT_Close type, int lTrackNo)
{
   cmd_Close cl(dio, result);

   _D(Lvl_Info, "Closing DVD-R media");
   
   Notify(result(DRT_Operation_Write_Synchronize));
   cl.setType(cmd_Close::Close_FlushBuffers, 0);
   /* do not let immediate go up here. immediate flush causes problems. */
   cl.Go();
   WaitOpComplete();

   if (type == DRT_Close_Track) 
   {
      Notify(result(DRT_Operation_Write_CloseTrack));
      cl.setType(cmd_Close::Close_DVDMinusR_Track, lTrackNo);
   }
   else if (type == DRT_Close_Session) 
   {
      Notify(result(DRT_Operation_Write_CloseSession));
      cl.setType(cmd_Close::Close_DVDMinusR_LastSession, 0);
   }
   else if (type == DRT_Close_Finalize) 
   {
      Notify(result(DRT_Operation_Write_CloseDisc));
      cl.setType(cmd_Close::Close_DVDMinusR_LastSession, 0);
   }

   cl.setImmediate(true);
   cl.Go();
   WaitOpComplete();
   
   RequestUpdate();
   return result;
}

bool Disc_DVD_MinusR::IsWritable()
{ 
   if (GetNextWritableTrack(0))
      return true;
   return false;
}

DriveStatus& Disc_DVD_MinusR::CheckItemData(const IOptItem *pDI)
{
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
   if (pDI->getPreGapSize())
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
