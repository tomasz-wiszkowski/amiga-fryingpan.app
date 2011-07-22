#include "Headers.h"
#include "Disc_Generic.h"
#include "Drive.h"
#include "Config.h"
#include "OptTrack.h"
#include "OptSession.h"

#include "Disc_CD_ROM.h"
#include "Disc_CD_R.h"
#include "Disc_CD_RW.h"
#include "Disc_DVD_ROM.h"
#include "Disc_DVD_PlusR.h"
#include "Disc_DVD_PlusRW.h"
#include "Disc_DVD_MinusR.h"
#include "Disc_DVD_MinusRW.h"
#include "Disc_DVD_RAM.h"
#include "Disc_BD_ROM.h"
#include "Disc_BD_R.h"
#include "SCSI/scsi_GetPerformance.h"


// TODO: przerobiæ ca³y ten ba³agan.

Disc* Disc::GetDisc(Drive & drv, DRT_Profile type)
{
    Disc *d = 0;
    switch (type) 
    {
	case DRT_Profile_DDCD_ROM:
	case DRT_Profile_CD_ROM:                    d = new Disc_CD_ROM      (drv);   break;
	case DRT_Profile_DDCD_R:
	case DRT_Profile_CD_R:                      d = new Disc_CD_R        (drv);   break;
	case DRT_Profile_DDCD_RW:
	case DRT_Profile_CD_MRW:
	case DRT_Profile_CD_RW:                     d = new Disc_CD_RW       (drv);   break;
	case DRT_Profile_DVD_ROM:                   d = new Disc_DVD_ROM     (drv);   break;
	case DRT_Profile_DVD_MinusR:                d = new Disc_DVD_MinusR  (drv);   break;
	case DRT_Profile_DVD_MinusRW_Sequential:
	case DRT_Profile_DVD_MinusRW_Restricted:    d = new Disc_DVD_MinusRW (drv);   break;
	case DRT_Profile_DVD_PlusR:                 d = new Disc_DVD_PlusR   (drv);   break;
	case DRT_Profile_DVD_PlusRW:                d = new Disc_DVD_PlusRW  (drv);   break;
	case DRT_Profile_DVD_RAM:                   d = new Disc_DVD_RAM     (drv);   break;
	case DRT_Profile_NoDisc:
	case DRT_Profile_Unknown:
	case DRT_Profile_BD_ROM:		    d = new Disc_BD_ROM      (drv);   break;
	case DRT_Profile_BD_R_Sequential:
	case DRT_Profile_BD_R_RandomWrite:
	case DRT_Profile_BD_R_PseudoOverwrite:
	case DRT_Profile_BD_RW:			    d = new Disc_BD_R        (drv);   break;
    }   

    /*
     * only if we had no better choice above, report no disc
     * otherwise leave everything to the task
     */
    if ((d != 0) && (!d->subinit()))
    {
	delete d;
	d = 0;
    }
    return d;
}

Disc::Disc(Drive &d) :
    drive(d),
    dio(d.GetDriveIO()),
    result(d.GetDriveStatus()),
    cRDI(dio, result)
{
    lSeqSector     = 0;
    lSeqLastSector = 0;
    pSeqStructure  = 0;
    pSeqCurrentTrack  = 0;
    read_speed     = 0xffff;
    write_speed    = 0xffff;
    write_method   = DRT_WriteMethod_Default;
    refresh_period = 0;
    pDiscInfo      = 0;
    bNeedUpdate    = false;
    SET_DEBUG_ENGINE(d.GetDebug());
    bNeedUpdate    = false;
    write_method   = DRT_WriteMethod_Default;   // do not use SetWriteMethod!!!
    pRead          = new cmd_Read(dio, result);
    pWrite         = new cmd_Write(dio, result);

    optcontent     = new OptDisc();
};

Disc::~Disc()
{
    optcontent->release();

    delete pRead;
    delete pWrite;
};

bool Disc::subinit()
{
    bool res = false;

    if (cRDI.Go())
    {
	pDiscInfo = cRDI.GetData();
	_D(Lvl_Info, "Got DiscInfo data");
    }
    else
    {
	_D(Lvl_Info, "No DiscInfo data");
	pDiscInfo = 0;  
    }

    /*
     * tell the parent component that we want drive to be refreshed
     * every 3 seconds
     */
    SetRefreshPeriod(150);

    /*
     * find a way to extract disc details
     */ 
    SetWriteMethod(DRT_WriteMethod_Default);   // assure no change is made

    read_speed  = 0;
    write_speed = 0;

    ReadSpeeds();

    res = ReadTracks();
    if (res)
	res = Init();

    return res;
}

const IOptItem *Disc::GetContents()
{
    return optcontent->acquire();
}


bool Disc::ReadTracks_TrackInfo()
{
    IOptItem            *osess = 0;
    IOptItem            *otrak = 0;
    int                  current_track;
    TrackInfo           *i;
    int                  current_session = -1;
    cmd_ReadTrackInfo    rti(dio, result);

    if (!pDiscInfo)
    {
	_D(Lvl_Info, "No DiscInfo structure");
	return false;
    }
    if (!drive.MediaWriteCapabilities()) 
    {
	_D(Lvl_Info, "No write capabilities");
	return false;                          // in such case drive is unable to provide us the TrackInfos
    }

    for (current_track = 1; current_track <= pDiscInfo->GetNumTracks(); current_track++)
    {
	rti.SelectTrack(current_track);
	rti.Go();
	if (rti.Result()) break;

	i = rti.GetData();

	if (i->GetSessionNumber() != current_session)
	{
	    if (osess)
		osess->printReport(drive.GetDebug());

	    osess = optcontent->addChild();
	    osess->setItemNumber(i->GetSessionNumber());
	    osess->setComplete(true);
	    current_session = i->GetSessionNumber();
	}

	otrak = osess->addChild();
	i->FillInDiscItem(otrak);

	otrak->printReport(drive.GetDebug());
    }

    /* if read track info is available, read disc info is available, too. */

    if (osess)
    {
	osess->setComplete(!pDiscInfo->IsLastSessionIncomplete());
	osess->setBlank(pDiscInfo->IsLastSessionEmpty());
    }
    if (optcontent)
    {
	optcontent->setComplete(!pDiscInfo->IsDiscIncomplete());
	optcontent->setBlank(pDiscInfo->IsDiscEmpty());
    }

    if (osess)
	osess->printReport(drive.GetDebug());
    optcontent->printReport(drive.GetDebug());
    return true;
};

bool Disc::ReadTracks_TOC()
{
    int                  current_track;
    cmd_ReadTOC          rtc(dio, result);
    TOC_FullTOC         *toc = 0;
    int                  current_session=0;
    IOptItem            *osess = 0;
    IOptItem            *otrak = 0;

    toc = rtc.GetFullTOC();
    if (0 == toc) 
    {
	_D(Lvl_Info, "No full TOC");
	return false;
    }

    for (current_track=1; current_track<=toc->GetNumTracks(); current_track++) 
    {
	if (toc->FindTOCEntry(current_track)->GetTrackSession() != current_session) 
	{
	    if (osess)
		osess->printReport(drive.GetDebug());

	    current_session = toc->FindTOCEntry(current_track)->GetTrackSession();

	    osess = optcontent->addChild();
	    osess->setItemNumber(current_session);
	}

	otrak = osess->addChild();
	toc->FillInDiscItem(otrak, current_track);

	otrak->printReport(drive.GetDebug());
    }

    if (osess)
	osess->printReport(drive.GetDebug());
    optcontent->printReport(drive.GetDebug());
    return true;
};

bool Disc::ReadTracks_PrimitiveTOC()
{
    int                  current_track;
    cmd_ReadTOC          rtc(dio, result);
    TOC_PrimitiveTOC    *toc = 0;
    int                  current_session=0;
    IOptItem            *osess = 0;
    IOptItem            *otrak = 0;

    toc = rtc.GetTOC(false);
    if (!toc) 
    {
	_D(Lvl_Info, "No TOC");
	return false;
    }

    for (current_track=1; current_track<=toc->GetNumTracks(); current_track++) 
    {
	if (toc->FindTOCEntry(current_track)->GetTrackSession() != current_session) 
	{
	    if (osess)
		osess->printReport(drive.GetDebug());

	    current_session = toc->FindTOCEntry(current_track)->GetTrackSession();

	    osess = optcontent->addChild();
	    osess->setItemNumber(current_session);
	}

	otrak = osess->addChild();
	toc->FillInDiscItem(otrak, current_track);

	otrak->printReport(drive.GetDebug());
    }

    if (osess)
	osess->printReport(drive.GetDebug());
    optcontent->printReport(drive.GetDebug());
    return true;
};

bool Disc::ReadTracks()
{
    bool res = false;

    Notify(result(DRT_Operation_Analyse_Tracks, DRT_OpStatus_InProgress));
    res = ReadTracks_TrackInfo();
    if (!res)
	res = ReadTracks_TOC();
    if (!res)
	res = ReadTracks_PrimitiveTOC();
    Notify(result(DRT_Operation_Analyse_Tracks, DRT_OpStatus_Completed));
    return res;
}


DriveStatus& Disc::WriteData(const IOptItem *di)
{
    return result.Complete(ODE_CommandError);
}

void Disc::SetRefreshPeriod(UBYTE fiftieths)
{
    if (fiftieths > 250) {
	_D(Lvl_Error, "Extreme refresh period truncated.");
	fiftieths = 250;
    }

    if (fiftieths < 10) {
	_D(Lvl_Warning, "Refresh period set too small, correcting BUT IT IS STILL SHORT ANYWAYS.");
	fiftieths = 10;
    }
    refresh_period = fiftieths;
}

const UBYTE Disc::GetRefreshPeriod(void)
{
    return refresh_period;
};

DriveStatus& Disc::RandomRead(const IOptItem *i, int32 first, int32 count, void* buff)
{
    if (0 == i)
	i = optcontent;

    if (first < i->getStartAddress())
	return result.Complete(ODE_BadBlockNumber);
    if ((first+count) > (i->getEndAddress()+1))
	return result.Complete(ODE_BadBlockNumber);
    if (!buff)                          
	return result.Complete(ODE_NoMemory);

    pRead->ReadData(first, count, buff);
    return result;
}

DriveStatus& Disc::RandomWrite(const IOptItem *i, int32 first, int32 count, void* buff)
{
    if (0 == i)
	i = optcontent;

    if (first < i->getStartAddress())
	return result.Complete(ODE_BadBlockNumber);
    if ((first + count) > i->getEndAddress())
	return result.Complete(ODE_BadBlockNumber);
    if (!buff)                          
	return result.Complete(ODE_NoMemory);

    return WriteDisc(first, count, i->getSectorSize(), buff);
}

DriveStatus& Disc::WriteDisc(int lBlock, unsigned short lLength, unsigned short lSectorSize, APTR pBuffer)
{
    for (;;) {
	pWrite->WriteData(lBlock, lLength, lSectorSize, pBuffer);

	if (!result) 
	    break;

	if ((result.SCSIError() & 0xff0000) != 0x20000) 
	    break;

	DOS->Delay(20);
    }
    return result;
}

/*
 * close disc (return error) (updated)
 * generic implementation for -ROM discs
 */
DriveStatus& Disc::CloseDisc(DRT_Close, int)
{
    return result.Complete(ODE_IllegalCommand);
};

DriveStatus& Disc::UploadLayout(const IOptItem *pDI)
{
    if (pDI == 0)
	return result.Complete(ODE_BadLayout);
    if (pDI->getItemType() != Item_Disc)               // regular hierarchy
	return result.Complete(ODE_BadLayout);
    if (pDI->getChildCount() != 1)                     // with just one session (we don't burn more)
	return result.Complete(ODE_BadLayout);
    if (pDI->getChild(0)->getChildCount() == 0)        // and at least one track
	return result.Complete(ODE_BadLayout);

    pSeqStructure     = pDI;
    pSeqCurrentTrack  = pDI->getChild(0)->getChild(0);

    lSeqSector        = pSeqCurrentTrack->getStartAddress();
    lSeqLastSector    = pSeqCurrentTrack->getEndAddress();

    _D(Lvl_Info, "The following structure will be recorded:");
    pDI->printReport(DEBUG_ENGINE);
    for (int s=0; s<pDI->getChildCount(); s++)
    {
	pDI->getChild(s)->printReport(DEBUG_ENGINE);
	for (int t=0; t<pDI->getChild(s)->getChildCount(); t++)
	{
	    pDI->getChild(s)->getChild(t)->printReport(DEBUG_ENGINE);
	}
    }

    cmd_Calibrate cal(dio, result);
    cal.Go();
    return result;
};

uint32 Disc::GetNumTracks()
{
    const IOptItem *i;

    if (optcontent->getChildCount() == 0)
	return 0;
    i = optcontent->getChild(optcontent->getChildCount() - 1);

    ASSERT(i->getChildCount() != 0);

    return i->getChild(i->getChildCount() - 1)->getItemNumber();
};

uint32 Disc::GetNumSessions()
{
    if (optcontent->getChildCount() == 0)
	return 0;
    return (optcontent->getChild(optcontent->getChildCount() - 1)->getItemNumber());
};

DriveStatus& Disc::SetReadSpeed(uint16 rd)
{
    if (read_speed == rd) 
	return result.Complete();

    cmd_SetSpeed            ss(dio, result);
    cmd_Mode                md(dio, result);
    Page<Page_Capabilities> cap;

    ss.SetSpeed(rd, write_speed);
    if (result == ODE_OK)
    {
	cap = md.GetPage(cmd_Mode::Id_Page_Capabilities);
	read_speed = cap->GetCurrentReadSpeed();
	write_speed = cap->GetCurrentWriteSpeed();
    }

    return result;
}

DriveStatus& Disc::SetWriteSpeed(uint16 wr)
{
    if (write_speed == wr) 
	return result.Complete();

    if (!(IsWritable() || IsErasable() || IsFormattable() || IsOverwritable()))
	return result.Complete(ODE_IllegalCommand);

    cmd_SetSpeed            ss(dio, result);
    cmd_Mode                md(dio, result);
    Page<Page_Capabilities> cap;

    ss.SetSpeed(read_speed, wr);
    if (result == ODE_OK)
    {
	cap = md.GetPage(cmd_Mode::Id_Page_Capabilities);
	read_speed = cap->GetCurrentReadSpeed();
	write_speed = cap->GetCurrentWriteSpeed();
    }

    return result;
}

uint16 Disc::GetWriteSpeed()
{
    return write_speed;
}

uint16 Disc::GetReadSpeed()
{
    return read_speed;
}

const char *Disc::DiscVendor()
{
    return Cfg->Vendors()->getVendor(0, 0, 0);
}

const IOptItem *Disc::GetNextWritableTrack(const IOptItem *di)
{
    bool fnd = false;
    _D(Lvl_Info, "Querying next writable track after %08lx", (int)di);

    if (0 == di)
	fnd = true;

    for (int i=0; i<optcontent->getChildCount(); i++)
    {
	const IOptItem *sess = optcontent->getChild(i);

	for (int j=0; j<sess->getChildCount(); j++)
	{
	    const IOptItem *trak = sess->getChild(j);

	    if ((!fnd) && (trak == di))
		fnd = true;
	    else if ((fnd) && (trak->isBlank()))
		return trak;
	    else if ((fnd) && (IsOverwritable()) && (trak->isIncremental()))
		return trak;
	}
    } 

    return 0;
}

DriveStatus& Disc::SetWriteMethod(DRT_WriteMethod wm)
{
    _D(Lvl_Info, "Setting write method to %ld", wm);
    write_method = wm;
    return OnChangeWriteMethod();
};

DRT_WriteMethod Disc::GetWriteMethod()
{
    return write_method;
};

DriveStatus& Disc::OnChangeWriteMethod()
{
    return result.Complete(ODE_IllegalCommand);
}

/*
 * perform track layout (updated) 
 */
DriveStatus& Disc::LayoutTracks(const IOptItem *dit)
{
    const IOptItem *pTrack = 0;

    /* tell what we're doing. */
    Notify(result(DRT_Operation_Analyse_Layout));

    pTrack = GetNextWritableTrack(0);      // we start with *first*, not *last* writable track

    if (pTrack == 0) 
    {
	_D(Lvl_Info, "Unable to find writable track.");
	return result.Complete(ODE_NotEnoughSpace);
    }                            
    if (dit == 0)
    {
	_D(Lvl_Warning, "No disc structure passed");
	return result.Complete(ODE_BadLayout);
    }
    if (dit->getChildCount() != 1)
    {
	_D(Lvl_Warning, "No sessions defined in disc structure");
	return result.Complete(ODE_BadLayout);
    }
    if (dit->getChild(0)->getChildCount() == 0)
    {
	_D(Lvl_Warning, "No tracks defined in disc/session structure");
	return result.Complete(ODE_BadLayout);
    }

    _D(Lvl_Info, "Provided structure");
    dit->printReport(drive.GetDebug());
    result = CheckItemData(dit);
    if (result != ODE_OK)
	return result;

    /* based on the settings resulting from disc specification we calculate the following */

    layCurrentBlock= pTrack->getStartAddress() + LayoutAdjustment();
    layFirstTrack  = pTrack->getItemNumber();
    layFirstSession= pTrack->getParent()->getItemNumber();
    laySessions    = 0;
    layTracks      = 0;

    /* now we can perform layout */

    for (int sn=0; sn<dit->getChildCount(); sn++)
    {
	const IOptItem *sess = dit->getChild(sn);

	_D(Lvl_Info, "Provided structure");
	sess->printReport(drive.GetDebug());
	result = CheckItemData(sess);
	if (result != ODE_OK)
	    return result;

	for (int tn=0; tn<sess->getChildCount(); tn++)
	{
	    const IOptItem *trak = sess->getChild(tn);

	    _D(Lvl_Info, "Provided structure");
	    trak->printReport(drive.GetDebug());
	    result = CheckItemData(trak);
	    if (result != ODE_OK)
		return result;
	}
    }

    if (dit->getEndAddress() > pTrack->getEndAddress())
	return result.Complete(ODE_NotEnoughSpace);

    return result.Complete();
}

DriveStatus& Disc::CheckItemData(const IOptItem* dit)
{
    if (dit->isBlank()) 
    {
	_D(Lvl_Warning, "Blank tracks not allowed");
	return result.Complete(ODE_BadLayout);
    }

    const_cast<IOptItem*>(dit)->setPacketSize(16);

    switch (dit->getItemType())
    {
	case Item_Disc:
	    break;

	case Item_Session:
	    if ((laySessions != 0) && (!AllowMultiSessionLayout()))
	    {
		_D(Lvl_Warning, "Only one session allowed");
		return result.Complete(ODE_BadLayout);
	    }
	    else if (laySessions > 0)
	    {
		layCurrentBlock += SessionGapSize();
	    }
	    const_cast<IOptItem*>(dit)->setItemNumber(layFirstSession);
	    ++laySessions;
	    ++layFirstSession;
	    layTracks = 0;
	    break;

	case Item_Track:
	    if ((layTracks != 0) && (!AllowMultiTrackLayout()))
	    {
		_D(Lvl_Warning, "Only one track allowed");
		return result.Complete(ODE_BadLayout);
	    }
	    else if (layTracks > 0)
	    {
		layCurrentBlock += TrackGapSize();
	    }
	    const_cast<IOptItem*>(dit)->setItemNumber(layFirstTrack);
	    const_cast<IOptItem*>(dit)->setStartAddress(layCurrentBlock);
	    layCurrentBlock = dit->getEndAddress() + 1;
	    ++layTracks;
	    ++layFirstTrack;
	    break;

	case Item_Index:
	    _D(Lvl_Warning, "Indices not allowed.");
	    return result.Complete(ODE_BadLayout);
	    break;
    }

    _D(Lvl_Info, "Resulting structure:");
    dit->printReport(drive.GetDebug());

    return result.Complete();
}

const IOptItem *Disc::FindTrack(int lTrackNum)
{
    const IOptItem *sess;
    const IOptItem *trak;

    for (int i=0; i<optcontent->getChildCount(); i++)
    {
	sess = optcontent->getChild(i);
	ASSERT(sess->getChildCount() != 0);

	if ((sess->getChild(0)->getItemNumber() <= lTrackNum) && (sess->getChild(sess->getChildCount()-1)->getItemNumber() >= lTrackNum))
	{
	    trak = sess->getChild(0);
	    trak = sess->getChild(lTrackNum - trak->getItemNumber());
	    ASSERT(trak != 0);
	    ASSERT(trak->getItemNumber() == lTrackNum);
	    return trak;
	}
    }
    return 0;   
}

const IOptItem *Disc::FindSession(int lSessNum)
{
    const IOptItem *sess;

    sess = optcontent->getChild(lSessNum - 1);
    if (sess != 0)
    {
	ASSERT(sess->getItemNumber() == lSessNum);
    }

    return sess;
}

DriveStatus& Disc::SequentialWrite(APTR pMem, ULONG lBlocks)
{
    uint32 count;

    if (!pSeqCurrentTrack) 
	return result.Complete(ODE_IllegalCommand);

    if (0 != pSeqCurrentTrack->isIncremental())
    {
	if (0 != (lBlocks % pSeqCurrentTrack->getPacketSize()))
	    return result.Complete(ODE_BadTransferSize);
    }

    if (lSeqSector == pSeqCurrentTrack->getStartAddress())                     // so, the new track has just begun
    {
	BeginTrackWrite(pSeqCurrentTrack);                                      // tell that we're warming up the next track

	if (pSeqCurrentTrack->getPreGapSize())                                  // meanwhile check if we need to generate any data
	    WritePadData(pSeqCurrentTrack, pSeqCurrentTrack->getPreGapSize());   // write pre gap data
    }

    count = pSeqCurrentTrack->getDataEndAddress() - lSeqSector + 1;            // how many sectors can we actually handle?

    if (count > lBlocks) count = lBlocks;                                      // write no more than we can (track / available)
    lBlocks -= count;                                                          // reduce amount of blocks to be written...

    result = WriteDisc(lSeqSector, count, pSeqCurrentTrack->getSectorSize(), pMem);   
    if (result != ODE_OK)                                                      // oups...
	return result;                                                         // caught error -> return error.

    pMem = &((char*)pMem)[count * pSeqCurrentTrack->getSectorSize()];          // proceed to next memory location
    lSeqSector += count;                                                       // update counters

    if (lSeqSector == (pSeqCurrentTrack->getDataEndAddress()+1))               // last data sector has been written
    {
	WritePadData(pSeqCurrentTrack, pSeqCurrentTrack->getPostGapSize());     // write post gap data
	EndTrackWrite(pSeqCurrentTrack);                                        // perform cleanup operations
	CloseDisc(DRT_Close_Track, pSeqCurrentTrack->getItemNumber());          // close track

	for (int i=0; i<pSeqStructure->getChild(0)->getChildCount(); i++)       // browse a single session!
	{
	    const IOptItem *trk = pSeqStructure->getChild(0)->getChild(i);       // unless someone mangled the structure 
	    if (trk == pSeqCurrentTrack)                                         // everything will go well
	    {                                                                    //
		pSeqCurrentTrack = 0;                                             // if we found the previously finished track
		continue;                                                         // mark that we're waiting for next one
	    }                                                                    //
	    else if (pSeqCurrentTrack == 0)                                      // if we found that track in previous iteration
	    {                                                                    // just use the one we found in this one
		pSeqCurrentTrack = trk;                                           // and stop looping
		break;                                                            //
	    }                                                                    // otherwise, the pSeqCurrentTrack = 0
	}                                                                       // and operation stops;

	if (!pSeqCurrentTrack)                                                  // did we reach the end of structure?
	{                                                                       // seems so
	    if (pSeqStructure->getFlags() & DIF_Disc_CloseDisc)                  // do we want to close disc?
		CloseDisc(DRT_Close_Finalize, 0);                                 //
	    else if (pSeqStructure->getFlags() & DIF_Disc_CloseSession)          // or close session?
		CloseDisc(DRT_Close_Session, 0);                                  //
	    return result.Complete();                                            // finally, it's all done :)
	}
	else                                                                    //
	{
	    lSeqSector     = pSeqCurrentTrack->getStartAddress();
	    lSeqLastSector = pSeqCurrentTrack->getEndAddress();
	}
    }

    if (lBlocks <= 0) 
	return result.Complete();                                           // all blocks have been written
    return SequentialWrite(pMem, lBlocks);
}

DriveStatus& Disc::WritePadData(const IOptItem *pDI, int32 len)
{
    uint8* pBuf = new uint8[(pDI->getSectorSize() * pDI->getPacketSize())];
    int lLen;

    _D(Lvl_Debug, "Writing track pre-gap");

    while (len)
    {
	lLen = len <? pDI->getPacketSize();
	result = WriteDisc(lSeqSector, lLen, pDI->getSectorSize(), pBuf);
	if (result != ODE_OK)
	    break;
	lSeqSector += lLen;
	len -= lLen;
    }

    delete [] pBuf; 
    return result;
}

bool Disc::RequiresUpdate()
{
    return bNeedUpdate;
}

void Disc::RequestUpdate()
{
    bNeedUpdate = true;
}

int16 Disc::GetOperationProgress()
{
    return 0;
}

/*
** for some reason i have a feeling speed setting under linux does not exactly work
*/
/*
** GetPerformance based ReadSpeed
** totally preferred, resulting in a very accurate speeds and stuff
** we like it.
*/
bool Disc::ReadSpeeds_perfBased()
{
    DiscSpeed               spd;
    DiscSpeed		    oldspd;
    cmd_SetSpeed            ss(dio, result);
    scsi_GetPerformance	    perf(dio, result);
    Perf_WriteSpeeds*	    pwspds;
    Perf_PerformanceData*   pdata;
    uint32		    sblk, eblk, sspd, espd;

    /*
    ** collect write speeds. see if we have any.
    */
    pwspds = perf.readWriteSpeeds();
    if (0 == pwspds)
	return false;

    /*
    ** repeat for every write speed. automatically, *ignore* read speeds.
    */
    for (int sx = 0; sx < pwspds->getSpeedCount(); sx++)
    {
	/*
	** set speed for this round
	*/
	pwspds->getSpeedItem(sx, sspd, espd);
	_D(Lvl_Info, "Speed report: Read: %ld, Write: %ld", sspd, espd);
	ss.SetSpeed(sspd, espd);

	/*
	** see what we can read.
	*/
	pdata = perf.readPerformanceData(true);

	/*
	** note: the fact that we have performance data does not mean
	** that this data is actually reliable.
	** it could be, that perfdata only holds the 'default' ('best') recording speed details
	** in which case it's useless to us.
	*/
	if (pdata)
	{
	    int i;

	    /*
	    ** the performance data only returns performance for
	    ** (a) currently selected speed or
	    ** (b) best available speed
	    ** there seems to be no mixture of multiple perf descriptors here
	    */
	    for (i=0; i<pdata->getPerfCount(); i++)
	    {
		/*
		** collect performance speed details
		*/
		pdata->getPerfItem(i, sblk, sspd, eblk, espd);
		_D(Lvl_Info, "Performance Item: %ld - %ld / %ld - %ld", sblk, eblk, sspd, espd);

		/*
		** analyze speed descriptor and try to match the speed type
		*/
		if (sblk == 0)
		{
		    /*
		    ** if sblk = 0, then this is the first element
		    ** equal speeds suggest CLV (constant linear velo)
		    ** differing speeds suggest CAV (const angular velo)
		    */
		    if (espd == sspd)
			spd.type = DIF_Speed_CLV;
		    else
			spd.type = DIF_Speed_CAV;

		    /*
		    ** initialize this descriptor (in case it's the first/last/whatever
		    */
		    spd.begin_kbps = sspd;
		    spd.end_kbps = espd;
		}
		else
		{
		    /*
		    ** if sblk != 0, this must be a second or further speed descriptor in a row
		    ** usually this suggests a mixed type recording
		    */
		    ASSERTS(espd == sspd, "Write method cannot be fully determined");
		    if (espd == sspd)
		    {
			if (spd.type == DIF_Speed_CLV)
			    spd.type = DIF_Speed_ZCLV;
			else
			    spd.type = DIF_Speed_CAVCLV;
		    }

		    /*
		    ** fill in end speed in case it's the last descriptor
		    */
		    spd.end_kbps = espd;
		}

	    }

	    /*
	    ** queue last write speed if we had any descriptors
	    */
	    if ((spd.begin_kbps != oldspd.begin_kbps) ||
		(spd.end_kbps != oldspd.end_kbps))
	    {
		_D(Lvl_Info, "Appending performance element");
		FillDiscSpeed(spd);
		oldspd = spd;
		writeSpeeds << spd;
	    }
	    else
	    {
		/*
		** get read and write speeds
		*/
		pwspds->getSpeedItem(sx, sspd, espd);
		spd.begin_kbps = espd;
		spd.end_kbps = espd;
		spd.type = DIF_Speed_CLV;
		FillDiscSpeed(spd);
		writeSpeeds << spd;
	    }

	    pdata->Dispose();
	}
    }

    pwspds->Dispose();

    {
	Perf_PerformanceData* pdata;
	scsi_GetPerformance perf(dio, result);

    	pdata = perf.readPerformanceData(false);

	if (pdata)
	{
	    uint32 sblk, eblk, sspd, espd;
	    int i;

	    for (i=0; i<pdata->getPerfCount(); i++)
	    {
		/*
		** collect performance speed details
		*/
		pdata->getPerfItem(i, sblk, sspd, eblk, espd);
		_D(Lvl_Info, "Performance Item: %ld - %ld / %ld - %ld", sblk, eblk, sspd, espd);

		/*
		** if block is 0 and descriptor is != 0
		** then we are starting a new speed descriptor.
		** this order is necessary as we're going to kill some data soon
		*/
		if ((sblk == 0) && (i != 0))
		{
		    _D(Lvl_Info, "Appending performance element");
		    FillDiscSpeed(spd);
		    readSpeeds << spd;
		}

		/*
		** analyze speed descriptor and try to match the speed type
		*/
		if (sblk == 0)
		{
		    /*
		    ** if sblk = 0, then this is the first element
		    ** equal speeds suggest CLV (constant linear velo)
		    ** differing speeds suggest CAV (const angular velo)
		    */
		    if (espd == sspd)
			spd.type = DIF_Speed_CLV;
		    else
			spd.type = DIF_Speed_CAV;

		    /*
		    ** initialize this descriptor (in case it's the first/last/whatever
		    */
		    spd.begin_kbps = sspd;
		    spd.end_kbps = espd;
		}
		else
		{
		    /*
		    ** if sblk != 0, this must be a second or further speed descriptor in a row
		    ** usually this suggests a mixed type recording
		    */
		    ASSERTS(espd == sspd, "Write method cannot be fully determined");
		    if (espd == sspd)
		    {
			if (spd.type == DIF_Speed_CLV)
			    spd.type = DIF_Speed_ZCLV;
			else
			    spd.type = DIF_Speed_CAVCLV;
		    }

		    /*
		    ** fill in end speed in case it's the last descriptor
		    */
		    spd.end_kbps = espd;
		}

	    }

	    /*
	    ** queue last write speed if we had any descriptors
	    */
	    if (i != 0)
	    {
		_D(Lvl_Info, "Appending performance element");
		FillDiscSpeed(spd);
		readSpeeds << spd;
	    }

	    pdata->Dispose();
	}
    	
    }

    return true;
}

/*
** ModePage contents based ReadSpeed
** this theoretically would reduce amount 
** of calls to the drive to set and fetch speeds
** it's okay, worked for so long, so...
*/
bool Disc::ReadSpeeds_pageBased()
{
    DiscSpeed               spd;
    cmd_Mode                md(dio, result);
    Page<Page_Capabilities> caps;

    /*
    ** check if we have any speed descriptors available
    */
    caps = md.GetPage(cmd_Mode::Id_Page_Capabilities);
    if (caps->GetSpeedDescriptorsCount() == 0)
	return false;

    /*
    ** check what read speed was applied
    ** also, we have no idea about the rotational management
    ** so we assume all details on speed type are unknown.
    */
    spd.begin_kbps = caps->GetCurrentReadSpeed();
    spd.end_kbps = spd.begin_kbps;
    spd.type = DIF_Speed_CLV;
    FillDiscSpeed(spd);
    readSpeeds << spd;

    /*
    ** read all write descriptors for this drive.
    */
    for (int i=0; i<caps->GetSpeedDescriptorsCount(); i++)
    {
	/*
	** only set speeds. speed type is already preset to unknown.
	*/
	spd.begin_kbps = caps->GetSpeedDescriptor(i);
	spd.end_kbps = spd.begin_kbps;
	_DX(Lvl_Info, "Collected speed: %ld", spd.begin_kbps);

	FillDiscSpeed(spd);
	writeSpeeds << spd;
    }

    return true;
}

/*
** Probing. Totally lacks rotational control detection
** but then it's most reliable. if we set speed and drive says
** it has a new speed, it surely does.
*/
bool Disc::ReadSpeeds_selectBased()
{
    DiscSpeed               spd;
    cmd_SetSpeed            ss(dio, result);
    cmd_Mode                md(dio, result);
    Page<Page_Capabilities> caps;
    uint16		    lr,lw,cr,cw;

    scsi_GetPerformance	    perf(dio, result);
    Perf_PerformanceData*   pdata;

    cr = 0xfffe;
    cw = 0xfffe;
    lr = 0xfffe;
    lw = 0xfffe;

    /*
    ** right now, don't give a sh.t about CAV detection (= setting rot ctl to CAV)
    ** we only want non-pure-cav.
    */
    do
    {
	caps = md.GetPage(cmd_Mode::Id_Page_Capabilities);

	cr = caps->GetCurrentReadSpeed();
	cw = caps->GetCurrentWriteSpeed();
	_DX(Lvl_Info, "Collected speeds: %ld / %ld", cr, cw);

	if ((cr == lr) && (cw == lw))
	    break;

	pdata = perf.readPerformanceData(true);
	pdata->Dispose();


	if (cr != lr)
	{
	    spd.type = DIF_Speed_CLV;
	    spd.begin_kbps = cr;
	    spd.end_kbps = cr;
	    FillDiscSpeed(spd);
	    readSpeeds << spd;
	}
	
	if (cw != lw)
	{
	    spd.type = DIF_Speed_CLV;
	    spd.begin_kbps = cw;
	    spd.end_kbps = cw;
	    FillDiscSpeed(spd);
	    writeSpeeds << spd;
	}

	lr = cr;
	lw = cw;

	ss.SetSpeed(cr - 2, cw - 2);
    } while (1);

    return true;
}

/*
** Probing Old. Exact copy of the one above, except checks other fields.
*/
bool Disc::ReadSpeeds_selectOldBased()
{
    DiscSpeed               spd;
    cmd_SetSpeed            ss(dio, result);
    cmd_Mode                md(dio, result);
    Page<Page_Capabilities> caps;
    uint16		    lr,lw,cr,cw;

    scsi_GetPerformance	    perf(dio, result);
    Perf_PerformanceData*   pdata;

    cr = 0xfffe;
    cw = 0xfffe;
    lr = 0xfffe;
    lw = 0xfffe;

    /*
    ** right now, don't give a sh.t about CAV detection (= setting rot ctl to CAV)
    ** we only want non-pure-cav.
    */
    do
    {
	caps = md.GetPage(cmd_Mode::Id_Page_Capabilities);

	cr = caps->GetCurrentReadSpeed();
	cw = caps->GetCurrentWriteSpeedOld();
	_DX(Lvl_Info, "Collected speeds: %ld / %ld", cr, cw);

	if ((cr == lr) && (cw == lw))
	    break;

	pdata = perf.readPerformanceData(true);
	pdata->Dispose();


	if (cr != lr)
	{
	    spd.type = DIF_Speed_CLV;
	    spd.begin_kbps = cr;
	    spd.end_kbps = cr;
	    FillDiscSpeed(spd);
	    readSpeeds << spd;
	}
	
	if (cw != lw)
	{
	    spd.type = DIF_Speed_CLV;
	    spd.begin_kbps = cw;
	    spd.end_kbps = cw;
	    FillDiscSpeed(spd);
	    writeSpeeds << spd;
	}

	lr = cr;
	lw = cw;

	ss.SetSpeed(cr - 2, cw - 2);
    } while (1);

    return true;
}

void Disc::ReadSpeeds()
{
    DiscSpeed               spd;
    cmd_SetSpeed            ss(dio, result);
    cmd_Mode                md(dio, result);
    Page<Page_Capabilities> caps;

    Notify(result(DRT_Operation_Analyse_Speeds, DRT_OpStatus_InProgress));

    readSpeeds.Empty();
    writeSpeeds.Empty();

    /*
    ** TODO: We want to use DIF_SpeedProbe functionality here
    */
    ss.SetSpeed(0xfffe, 0xfffe);
    if (!ReadSpeeds_perfBased())
	if (!ReadSpeeds_pageBased())
	    if (!ReadSpeeds_selectBased())
		ReadSpeeds_selectOldBased();

    ss.SetSpeed(0xfffe, 0xfffe);

    spd.begin_kbps = 0;
    spd.begin_i    = 0;
    spd.begin_f    = 0;   
    spd.end_kbps   = 0;
    spd.end_i	   = 0;
    spd.end_f	   = 0;
    spd.type = 0;

    SetReadSpeed(0xfffe);
    SetWriteSpeed(0xfffe);

    readSpeeds << spd;
    writeSpeeds << spd;
    Notify(result(DRT_Operation_Analyse_Speeds, DRT_OpStatus_Completed));
}

const DiscSpeed *Disc::GetReadSpeeds()
{
    return readSpeeds.GetArray();
}

const DiscSpeed *Disc::GetWriteSpeeds()
{
    return writeSpeeds.GetArray();
}

/*
 * return disc size in 2k blocks
 */
uint32 Disc::GetDiscSize()
{
    return optcontent->getEndAddress() + 1;
}

/*
 * Calibrate laser (updated)
 */
DriveStatus& Disc::Calibrate()
{
    cmd_Calibrate cal(dio, result);

    Notify(result(DRT_Operation_Write_Calibrate, DRT_OpStatus_InProgress));
    cal.Go();
    Notify(result(DRT_Operation_Write_Calibrate, DRT_OpStatus_Completed));
    return result;
}

/*
 * Wait for long operation to complete (updated)
 * sends nop until drive returns 'ready'
 * true if okay to proceed, false if fail
 */
bool Disc::WaitOpComplete(iptr period)
{
    DriveStatus lr(DRT_Operation_Unknown);
    cmd_TestUnitReady tur(dio, lr);
    _D(Lvl_Debug, "Waiting for operation to complete...");

    do
    {
	DOS->Delay(period);
	if (!tur.Go())
	    continue;
	if (lr != ODE_OK)
	    continue;
	if (lr.SCSIError() == 0x20404)
	    continue;
	if (lr.SCSIError() == 0x20407)
	    continue;
	if (lr.SCSIError() == 0x20408)
	    continue;
	if (lr.SCSIError() == 0x20000)
	    continue;
	break;
    } while (true);

    _D(Lvl_Debug, "Operation completed.");

    return (lr == ODE_OK);
}

