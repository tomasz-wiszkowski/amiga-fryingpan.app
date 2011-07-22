#include "Headers.h"
#include "Disc_CD_ROM.h"

Disc_CD_ROM::Disc_CD_ROM(Drive &d) : 
    Disc(d),
    readCD(dio, result),
    readSub(dio, result)
{
   mcn         = 0;
   subq        = new SUB_Q;
};

Disc_CD_ROM::~Disc_CD_ROM()
{
   if (mcn)
      delete mcn;
   if (subq)
      delete subq;
};

bool Disc_CD_ROM::Init()
{
    IOptItem* items = const_cast<IOptItem*>(GetContents());

    /*
    ** set sector size for entire disc
    */
    items->setSectorSize(2352);

    probeCDText();
    probeDataType();
    probeSubChannel();
    probeFreeDBID();

    for (int i=0; i<items->getChildCount(); i++)
    {
	/*
	** set sector size for entire sessions
	*/
	const_cast<IOptItem*>(items->getChild(i))->setSectorSize(2352);

	/*
	** set pre-gap size for first tracks in session
	** this may be not true for multi-session discs, i don't know
	*/
#warning this may be not true for multisession discs
	const_cast<IOptItem*>(items->getChild(i)->getChild(0))->setPreGapSize(150);
    }
    items->release();

    Notify(result(DRT_Operation_Analyse_DiscID, DRT_OpStatus_InProgress));
    mcn = readSub.getMCN();
    Notify(result(DRT_Operation_Analyse_DiscID, DRT_OpStatus_Completed));

    return true;
};

bool Disc_CD_ROM::probeCDText()
{
    cmd_ReadTOC	    rtc(dio, result); 
    TOC_CDText          *toc = 0;
    VectorT<String>     *sa  = 0;
    const IOptItem      *di  = 0;

    toc = rtc.GetCDText();
    if (!toc)
    {
	_D(Lvl_Debug, "Disc does not contain CD-Text information");
	return false;
    }

    Notify(result(DRT_Operation_Analyse_CDText, DRT_OpStatus_InProgress));

    sa = toc->GetTitles();
    if (sa)
    {
	for (uint32 i=0; i<=GetNumTracks(); i++)
	{
	    if (i < sa->Count())
	    {
		String &s = (*sa)[i];
		if (s.Length())
		{
		    if (i)
			di = FindTrack(i);        // only first session may contain cd audio!
		    else
			di = FindSession(1);
		    if (di)
		    {
			const_cast<IOptItem*>(di)->setCDTTitle(s.Data());
		    }
		}
	    }
	}
	delete sa;
    }

    sa = toc->GetPerformers();
    if (sa)
    {
	for (uint32 i=0; i<=GetNumTracks(); i++)
	{
	    if (i < sa->Count())
	    {
		String &s = (*sa)[i];
		if (s.Length())
		{
		    if (i)
			di = FindTrack(i);        // only first session may contain cd audio!
		    else
			di = FindSession(1);
		    if (di)
		    {
			const_cast<IOptItem*>(di)->setCDTArtist(s.Data());
		    }
		}
	    }
	}
	delete sa;
    }

    sa = toc->GetSongWriters();
    if (sa)
    {
	for (uint32 i=0; i<=GetNumTracks(); i++)
	{
	    if (i < sa->Count())
	    {
		String &s = (*sa)[i];
		if (s.Length())
		{
		    if (i)
			di = FindTrack(i);        // only first session may contain cd audio!
		    else
			di = FindSession(1);
		    if (di)
		    {
			const_cast<IOptItem*>(di)->setCDTLyrics(s.Data());
		    }
		}
	    }
	}
	delete sa;
    }

    sa = toc->GetComposers();
    if (sa)
    {
	for (uint32 i=0; i<=GetNumTracks(); i++)
	{
	    if (i < sa->Count())
	    {
		String &s = (*sa)[i];
		if (s.Length())
		{
		    if (i)
			di = FindTrack(i);        // only first session may contain cd audio!
		    else
			di = FindSession(1);
		    if (di)
		    {
			const_cast<IOptItem*>(di)->setCDTComposer(s.Data());
		    }
		}
	    }
	}
	delete sa;
    }

    sa = toc->GetArrangers();
    if (sa)
    {
	for (uint32 i=0; i<=GetNumTracks(); i++)
	{
	    if (i < sa->Count())
	    {
		String &s = (*sa)[i];
		if (s.Length())
		{
		    if (i)
			di = FindTrack(i);        // only first session may contain cd audio!
		    else
			di = FindSession(1);
		    if (di)
		    {
			const_cast<IOptItem*>(di)->setCDTDirector(s.Data());
		    }
		}
	    }
	}
	delete sa;
    }

    sa = toc->GetMessages();
    if (sa)
    {
	for (uint32 i=0; i<=GetNumTracks(); i++)
	{
	    if (i < sa->Count())
	    {
		String &s = (*sa)[i];
		if (s.Length())
		{
		    if (i)
			di = FindTrack(i);        // only first session may contain cd audio!
		    else
			di = FindSession(1);
		    if (di)
		    {
			const_cast<IOptItem*>(di)->setCDTMessage(s.Data());
		    }
		}
	    }
	}
	delete sa;
    }

    Notify(result(DRT_Operation_Analyse_CDText, DRT_OpStatus_Completed));
    return true;
};

bool Disc_CD_ROM::probeDataType()
{
   /*
    * disabled for now
    */
    Notify(result(DRT_Operation_Analyse_DataType, DRT_OpStatus_InProgress));
#warning data probing disabled
#if 0
   cmd_ReadHeader   rh(dio, result);
   const IOptItem*  item;
   //SUB_ISRC            *isrc;


   for (int i=1; i; i++)
   {
      item = FindTrack(i);
      if (0 == item)
         break;

      /*
       * warning: ReadISRC REQUIRES USER SETTING
       * as it is hell slow on drives that don't support it
       */
      /*
      isrc = pReadSub->getISRC(item->getItemNumber());

      if (isrc)
      {
         if (isrc->isValid())
         {
            _D(Lvl_Debug, "Got valid ISRC!");
         }
         delete isrc;
      }
      */

      if (item->getDataType() == Data_Mode1)
      {
         EDataType t = rh->GetTrackType(item->getStartAddress());
         if (rh->Result() == ODE_OK)
         {
            _D(Lvl_Info, "Track %ld - track mode %ld", item->getItemNumber(), t);
            const_cast<IOptItem*>(item)->setDataType(t);
         }
      }
   }

   _D(Lvl_Info, "%s: Finished track mode detection", (iptr)__PRETTY_FUNCTION__);
   delete rh;
   _D(Lvl_Info, "%s: all done", (iptr)__PRETTY_FUNCTION__);
#endif
    Notify(result(DRT_Operation_Analyse_DataType, DRT_OpStatus_Completed));
   return true;
}

bool Disc_CD_ROM::getSubChannelInfo(int32 pos, uint8 &trk, uint8 &idx)
{
   bool res;
   res = readCD.readCD(pos, 1, sizeof(SUB_Q), subq, cmd_ReadCD::Flg_AllSubQ);

   if (res)
   {
      trk = subq->getTrack();
      idx = subq->getIndex();
      _D(Lvl_Info, "%s: trk %ld, idx %ld, ctl %ld, adr %ld, rel %06lx, abs %06lx",
            (int)__PRETTY_FUNCTION__,
            subq->getTrack(),
            subq->getIndex(),
            subq->getCtl(),
            subq->getAdr(),
            subq->getRelPos(),
            subq->getAbsPos());
   }
   return res;
}

/*
int Disc_CD_ROM::probeSubChannel()
{
   if (GetContents()->getChildCount() == 0)
      return 0;

#warning speed this shit up

   cmd_ReadCD         rcd(dio);
   SUB_Q             *subq = new SUB_Q;
   int32              cpos = 0;     // this is a fixed value, tho most CD's won't be able to seek that far :P
   uint8              cidx = 0;
   uint8              ctrk = 0;

   int32              lpos = 0;  // defaults :P
   uint8              lidx = 0;     //
   uint8              ltrk = 1;     //

   int32              dist = 0;
   bool               fnd;

   const IOptItem    *trk;
   IOptItem          *idx;

   trk = FindTrack(1);
   if (trk->isBlank())
   {
      delete subq;
      return 0;
   }

   do
   {
      cpos = GetContents()->getEndAddress();
      dist = cpos - lpos;
      if (!dist)
         break;

      rcd.readCD(lpos, 1, sizeof(SUB_Q), subq, cmd_ReadCD::Flg_AllSubQ);
      ltrk = subq->getTrack();
      lidx = subq->getIndex();

      fnd = false;

      do
      {
         dist = (dist+1)>>1;
         if (!dist)
            break;

         if (cpos > GetContents()->getEndAddress())
         {
            cpos = GetContents()->getEndAddress();
            if (fnd && dist == 1)
               break;
         }
         if (cpos < lpos)
            cpos = lpos;

         rcd.readCD(cpos, 1, sizeof(SUB_Q), subq, cmd_ReadCD::Flg_AllSubQ);
         ctrk = subq->getTrack();
         cidx = subq->getIndex();

         if ((ltrk != ctrk) || (lidx != cidx))
         {
            if (fnd)
               break;
            cpos -= dist;
         }
         else
         {
            if (dist == 1)
               fnd = true;
            cpos += dist;
         }
      }
      while (true);

      trk = FindTrack(ltrk);
      if (trk != 0)
      {
         idx = const_cast<IOptItem*>(trk)->addChild();
         idx->setStartAddress(lpos);
         idx->setEndAddress(cpos-1);
         idx->setItemNumber(lidx);
      }

      lpos = cpos;
   }
   while (lpos != GetContents()->getEndAddress());

   delete subq;
   return 0;
}
*/

bool Disc_CD_ROM::probeSubChannel()
{
    const IOptItem* items = GetContents();

    Notify(result(DRT_Operation_Analyse_Indices, DRT_OpStatus_InProgress));
    for (int sn=0; sn<items->getChildCount(); sn++)
    {
	for (int tn=0; tn<items->getChild(sn)->getChildCount(); tn++)
	{
	    const IOptItem   *trak = items->getChild(sn)->getChild(tn);
	    IOptItem         *indx;
	    uint8             ctn, cin, ltn, lin;
	    int32             spos, cpos, epos, dlta;
	    bool              done;

	    // don't read subchannel data for blanks. it won't work.
	    if (trak->isBlank())
		continue;

	    // this is where we begin
	    spos = trak->getStartAddress();

	    _D(Lvl_Info, "%s: Analysing track %ld (%ld - %ld, %ld blocks long)",
	       (int)__PRETTY_FUNCTION__,
	       trak->getItemNumber(),
	       trak->getStartAddress(),
	       trak->getEndAddress(),
	       trak->getBlockCount());

	    while (spos < trak->getEndAddress())
	    {
		// if this fails, we may be either reading wrong sector type or
		// cd does not support command or maybe reading subq fails. whatever.
		if (ODE_OK != getSubChannelInfo(spos, ltn, lin))
		    break;

		// we always check at most the last sector.
		epos = trak->getEndAddress();
		cpos = epos;
		dlta = cpos - spos;
		done = false;

		// typical loop inside to find further indices.
		while (ODE_OK == getSubChannelInfo(cpos, ctn, cin))
		{
		    if (dlta == 1)
			done = true;
		    dlta = (dlta + 1) >> 1;

		    if (ctn != ltn)                     // track numbers do not match
			break;                           // means we got shit man
		    if ((cin == lin) && (cpos == epos))
			break;
		    if ((cin == lin) && (done))
			break;
		    else if (cin == lin)
			cpos += dlta;
		    else
			cpos -= dlta;
		    cpos = spos >? cpos;
		    cpos = epos <? cpos;
		}

		_D(Lvl_Info, "%s: Adding index %ld to track %ld from sector %ld to %ld", (int)__PRETTY_FUNCTION__,
		   trak->getItemNumber(),
		   lin,
		   spos,
		   cpos);
		indx = const_cast<IOptItem*>(trak)->addChild();
		indx->setStartAddress(spos);
		indx->setEndAddress(cpos);
		indx->setItemNumber(lin);
		spos = cpos + 1;
	    }

	    // this is true for very very special type of cd
	    if (trak->getChildCount() == 0)
	    {
		indx = const_cast<IOptItem*>(trak)->addChild();
		indx->setFlags(DIF_Common_RelSize);
		indx->setStartAddress(0);
		indx->setDataBlockCount(-1);
		indx->setItemNumber(1);
		_D(Lvl_Info, "%s: Track empty or no indices read", (int)__PRETTY_FUNCTION__);
		continue;
	    }

	}
    }
    items->release();
    Notify(result(DRT_Operation_Analyse_Indices, DRT_OpStatus_Completed));
    return true;
}

DriveStatus& Disc_CD_ROM::RandomRead(const IOptItem *i, int32 first, int32 count, void* buff)
{
    uint16 flags = 0;

    if (0 == i)
    {
	_D(Lvl_Warning, "No track passed to function");
	return result.Complete(ODE_IllegalParameter);
    }

    if (!buff)
    {
	_D(Lvl_Warning, "No memory passed.");
	return result.Complete(ODE_NoMemory);
    }
    if ((first) < (i->getStartAddress()))
    {
	_D(Lvl_Warning, "First sector before starting address: wanted: %ld, min: %ld", first, i->getStartAddress());
	return result.Complete(ODE_BadBlockNumber);
    }
    if ((first) > (i->getEndAddress()))
    {
	_D(Lvl_Warning, "First sector beyond ending address: wanted: %ld, max: %ld", first, i->getEndAddress());
	return result.Complete(ODE_BadBlockNumber);
    }
    if ((first+count) > (i->getEndAddress()+1))  // first sector = last block in track, count = 1.
    {
	_D(Lvl_Warning, "Last sector beyond ending address: wanted: %ld, size %ld; max: %ld", first, count, i->getEndAddress());
	return result.Complete(ODE_BadBlockNumber);
    }

    switch (i->getItemType())
    {
	case Item_Disc:
	case Item_Session:
	    flags = cmd_ReadCD::Flg_Sync | cmd_ReadCD::Flg_12BHeader | cmd_ReadCD::Flg_ECC;
	default:
	    flags |= cmd_ReadCD::Flg_UserData;
	    break;
    }

    /*
    ** note: it shares result
    */
    readCD.readCD(first, count, i->getSectorSize(), buff, flags);
    return result;
}

void Disc_CD_ROM::FillDiscSpeed(DiscSpeed &spd)
{
   spd.begin_f = 0;
   spd.begin_i = ((int32)10*spd.begin_kbps+882)/1764; // we need to round up the speed. 882 = 1764/2, so it's a halve.
   spd.end_f = 0;
   spd.end_i = ((int32)10*spd.end_kbps+882)/1764; // we need to round up the speed. 882 = 1764/2, so it's a halve.
}

uint32 Disc_CD_ROM::probeFreeDBID()
{
   cmd_ReadTOC          rtc(dio, result);
   TOC_PrimitiveTOC    *toc = 0;
   TOC_PrimitiveEntry  *te1 = 0;
   TOC_PrimitiveEntry  *te2 = 0;
   uint32               id1 = 0;
   uint32               id2 = 0;
   uint32               dbid= 0;

   freedbid = 0;

   toc = rtc.GetTOC(true);
   if (0 == toc)
      return 0;

    Notify(result(DRT_Operation_Analyse_FreeDB, DRT_OpStatus_InProgress));

   for (int i=1; i<=toc->GetNumTracks(); i++)
   {
      te1   = toc->FindTOCEntry(i);
      id1  += sumDigits(te1->GetMin()*60 + te1->GetSec());
   }

   te1   = toc->FindTOCEntry(1);
   te2   = toc->FindTOCEntry(0xAA);
   id2   = (60 * te2->GetMin() + te2->GetSec()) - (60 * te1->GetMin() + te1->GetSec());
   dbid  = ((id1 & 0xff) << 24) | ((id2 & 0xffff) << 8) | (toc->GetNumTracks());

   freedbid = dbid;

    Notify(result(DRT_Operation_Analyse_FreeDB, DRT_OpStatus_Completed));
   return dbid;
};

int Disc_CD_ROM::sumDigits(int32 val)
{
   int32 res = 0;

   while (val > 0)
   {
      res += (val % 10);
      val /= 10;
   }

   return res;
}

