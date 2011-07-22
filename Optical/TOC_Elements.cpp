#include "Headers.h"
#include "Commands.h"

TOC_Entry *TOC_FullTOC::FindTOCEntry(int32 item)
{
   int32 i;
   int32 size;

   size = (length-2)/11;      // yup...
   for (i=0; i<size; i++) {
      if (elem[i].GetTrackNumber() == item) return &elem[i];
   }

   return 0;
};

int32 TOC_FullTOC::GetNumTracks(void)
{
   int32 i;

   for (i=1; i<100; i++) {
      if (!FindTOCEntry(i)) break;
   }
   return i-1;
};

void TOC_FullTOC::FillInDiscItem(IOptItem *di, int32 track)
{
   TOC_Entry *e, *f;

   e = FindTOCEntry(track);
   f = FindTOCEntry(track+1);
   if (!e) 
   {
      di->setDataType(Data_Unknown);
   }
   else
   {
      di->setItemNumber(e->GetTrackNumber());
      di->setDataType(e->GetTrackType());
      di->setStartAddress(e->GetTrackPositionLBA());
      di->setEndAddress(f ? f->GetTrackPositionLBA()-1 : GetDiscSize()-1);
      di->setSectorSize(e->GetTrackSectorSize());
      di->setProtected(e->IsProtected());
      di->setPreemphasized(e->IsPreemphasized());
      di->setPacketSize(16);
   }
}

void TrackInfo::FillInDiscItem(IOptItem *di)
{
   di->setItemNumber(GetTrackNumber());
   di->setDataType(GetTrackType());
   di->setStartAddress(GetStartAddress());
   di->setEndAddress(GetEndAddress());
//   di->setActualSize(GetActualSize());
   di->setSectorSize(GetSectorSize());
   di->setProtected(IsProtected());
   di->setPreemphasized(IsPreemphasized());
   di->setIncremental(IsIncremental());
   di->setBlank(IsBlank());
   di->setPacketSize(GetPacketSize());
   di->setComplete(!((IsBlank() == 0) && (IsIncremental()) && (GetFreeBlocks() != 0)));
}

void TOC_PrimitiveTOC::FillInDiscItem(IOptItem *di, int32 track)
{
   TOC_PrimitiveEntry *e, *f;

   e = FindTOCEntry(track);
   f = FindTOCEntry(track+1);
   if (!e) 
   {
      di->setDataType(Data_Unknown);

   } 
   else 
   {
      di->setItemNumber(e->GetTrackNumber());
      di->setDataType(e->GetTrackType());
      di->setStartAddress(e->GetTrackPositionLBA());
      di->setEndAddress(f ? f->GetTrackPositionLBA()-1 : GetDiscSize()-1);
      di->setSectorSize(e->GetTrackSectorSize());
      di->setProtected(e->IsProtected());
      di->setPreemphasized(e->IsPreemphasized());
      di->setPacketSize(16);
   }
}

TOC_PrimitiveEntry *TOC_PrimitiveTOC::FindTOCEntry(int32 item)
{
   int32 i;
   int32 size;

   size = GetNumElements();
   for (i=0; i<size; i++) {
      if (elem[i].GetTrackNumber() == item) return &elem[i];
   }

   return 0;
};

int32 TOC_PrimitiveTOC::GetNumTracks(void)
{
   int32 i;

   for (i=1; i<100; i++) {
      if (!FindTOCEntry(i)) break;
   }
   return i-1;
};

int32 TOC_FullTOC::GetDiscSize(void)
{  
   TOC_Entry *pTOC = FindTOCEntry(0xA2);
   if (!pTOC)
      return 0;
   return pTOC->GetTrackPositionLBA();
}



VectorT<String> *TOC_CDText::MergeBlocks(int32 id)
{
   int32            i;
   int32            count;
   char          *block;
   VectorT<String>*pArr = new VectorT<String>;

   for (count=0, i=0; i<NumCDTextBlocks(); i++) {
      if (items[i].block_id == id) count++;
   }

   if (!count) return pArr;
   block = new char[12*count+1]; 
   count = 0;

   for (i=0; i<NumCDTextBlocks(); i++) {
      if (items[i].block_id == id) {
         Exec->CopyMem(&items[i].cdtextinfo, &block[count*12], 12);
         count++;
      }
   }

   for (i=0; i<(count*12); i++) {
      *(pArr) << String(&block[i]);
      while (block[i]!=0) i++;
   }

   delete [] block;
   return pArr;
}

VectorT<String> *TOC_CDText::GetTitles(void)
{
   return MergeBlocks(0x80);
}

VectorT<String> *TOC_CDText::GetPerformers(void)
{
   return MergeBlocks(0x81);
}

VectorT<String> *TOC_CDText::GetSongWriters(void)
{
   return MergeBlocks(0x82);
}

VectorT<String> *TOC_CDText::GetComposers(void)
{
   return MergeBlocks(0x83);
}

VectorT<String> *TOC_CDText::GetArrangers(void)
{
   return MergeBlocks(0x84);
}

VectorT<String> *TOC_CDText::GetMessages(void)
{
   return MergeBlocks(0x85);
}
