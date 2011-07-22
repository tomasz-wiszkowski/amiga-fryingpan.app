/*
 * FryingPan - Amiga CD/DVD Recording Software (User Intnerface and supporting Libraries only)
 * Copyright (C) 2001-2008 Tomasz Wiszkowski Tomasz.Wiszkowski at gmail.com
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#include "JobUpload.h"
#include <libdata/Optical/IOptItem.h>
#include <libclass/Optical.h>

JobUpload::JobUpload(Globals &glb, iptr drive, RWSyncT< VectorT<IData*> > &trks, bool master, bool closedisc) :
    Job(glb, drive),
    tracks(trks),
    vec(tracks.ObtainRead())
{
    disc     = g.Optical->CreateDisc();
    session  = disc->addChild();

    operation = "Recording Disc";

    finalize    = closedisc;
    masterize   = master;

    _dx(Lvl_Info, "Recording disc with flags: fin=%ld mas=%ld", finalize, masterize);

    for (uint32 i=0; i<vec.Count(); i++)
    {
	IData*t = vec[i];

	IOptItem   *o = session->addChild();
	if (!t->fillOptItem(o))
	{
	    _dx(Lvl_Info, "Failed to add track %ld; track refuses to fill structure.", i+1);
	    session->remChild(o);
	    continue;
	}

	_dx(Lvl_Info, "Added track %ld", i+1);
	Pair p = { t, o };
	items << p;
    }

    _dx(Lvl_Info, "Setting disc flags..");
    disc->setFlags((closedisc ? DIF_Disc_CloseDisc   : DIF_Disc_CloseSession) |
	    (masterize ? DIF_Disc_MasterizeCD : 0));
    session->setFlags(0);
    currTrack = 0;
    _dx(Lvl_Info, "Job init done");
}

JobUpload::~JobUpload()
{
    _dx(Lvl_Info, "Disposing job");
   disc->dispose();
   tracks.Release();
   _dx(Lvl_Info, "Done");
}

void JobUpload::execute()
{
    iptr res = 0;

    currTrack = 0;

    _dx(Lvl_Info, "Calculating layout");
    res = g.Optical->DoMethodA(ARRAY(DRV_LayoutTracks, Drive, (int)disc));

    if (res == ODE_OK)
    {
	_dx(Lvl_Info, "Pre-configuring tracks...");
	for (uint32 i=0; i<items.Count(); i++)
	{
	    // how to pass the damn structure here?
	    _dx(Lvl_Info, "Initializing track %ld", i+1);
	    items[i].fr->setUp(items[i].oi);
	}

	_dx(Lvl_Info, "Preparing for write");
	numBlocks = disc->getDataBlockCount();
	currBlock = 0;

	_dx(Lvl_Info, "Locking disc");
	g.Optical->DoMethodA(ARRAY(DRV_LockDrive, Drive, DRT_LockDrive_Write));

	_dx(Lvl_Info, "Uploading layout");
	res = g.Optical->DoMethodA(ARRAY(DRV_UploadLayout, Drive, (int)disc));

	_dx(Lvl_Info, "Commencing");
	for (currTrack=0; currTrack<items.Count(); currTrack++)
	{
	    IOptItem   *item    = items[currTrack].oi;
	    IData*	trak    = items[currTrack].fr;
	    iptr 	    secsize  = item->getSectorSize();
	    iptr 	    pktsize  = item->getPacketSize();
	    iptr	    seccnt   = item->getDataBlockCount();

	    if (item->getItemType() != Item_Track)
		continue;

	    _dx(Lvl_Info, "Track %ld will record %ld blocks of %ld size with fixed packet length=%ld", currTrack+1, seccnt, secsize, pktsize);
	    if (pktsize == 0)
		pktsize = 16;

	    char *memblk = new char[(secsize * pktsize)];

	    ASSERT(memblk != 0);

	    if (memblk != 0)
	    {
		_dx(Lvl_Info, "Memory allocated.");
		while (seccnt != 0)
		{
		    iptr xfer = seccnt <? pktsize;

		    _dx(Lvl_Info, "Reading data from track");
		    res = trak->readData(memblk, xfer);

		    if (!res)
		    {
			_dx(Lvl_Info, "Reader aborted");
			break;
		    }

		    _dx(Lvl_Info, "Writing data to disc");
		    res = (EOpticalError)g.Optical->DoMethodA(ARRAY(DRV_WriteSequential, Drive, (iptr)memblk, xfer));
		    if (res != ODE_OK)
			break;

		    currBlock += xfer;
		    seccnt -= xfer;
		}

		_dx(Lvl_Info, "Disposing memory");
		delete [] memblk;
	    }
	    else
	    {
		request("ERROR", "Unable to allocate memory (%ld x %ld = %ld bytes)!\nOperation aborted.", "Ok", ARRAY(secsize, pktsize, secsize*pktsize));
		break;
	    }

	    if (res != 0)
		break;
	}      

	_dx(Lvl_Info, "Cleaning up items");
	for (uint32 i=0; i<items.Count(); i++)
	{
	    _dx(Lvl_Info, "Cleanup item %ld", i+1);
	    items[i].fr->cleanUp();
	}

	_dx(Lvl_Info, "Closing disc...");
	g.Optical->DoMethodA(ARRAY(DRV_CloseDisc, Drive, finalize ? DRT_Close_Finalize : DRT_Close_Session));
	_dx(Lvl_Info, "Unlocking drive");
	g.Optical->DoMethodA(ARRAY(DRV_LockDrive, Drive, DRT_LockDrive_Unlock));

	if (res != 0)
	{
	    request("Error", "Error during write process. Operation aborted.", "Proceed", 0);
	}
    }
    else
    {
	request("Information", "Track layout failed (%ld). Disc will not be written to.", "Proceed", ARRAY(res));
    }
    _dx(Lvl_Info, "Done");
}

void JobUpload::onError(EOpticalError ret)
{
   switch (ret)
   {
      case ODE_OK:
         break;

      case ODE_NoMemory:           
         operation = "Not enough memory to complete.";
         break;

      case ODE_NoHandler:
         operation = "No device handler.";
         break;

      case ODE_NoDevice:
         operation = "Device not opened.";
         break;

      case ODE_InitError:
         operation = "Initialization failure.";
         break;

      case ODE_BadDriveType:
         operation = "Illegal drive type.";
         break;

      case ODE_MediumError:
         operation = "Medium failure, disc cannot be written to.";
         break;
         
      case ODE_IllegalType:
         operation = "Illegal disc type.";
         break;

      case ODE_NoDevOpened:
         operation = "No device opened.";
         break;

      case ODE_CommandError:
         operation = "Command failed. Disk cannot be written to.";
         break;

      case ODE_NoFormatDescriptors:
         operation = "No format descriptors found. Disc cannot be formatted.";
         break;

      case ODE_NoModePage:
         operation = "Unable to set device page, device refused to cooperate.";
         break;

      case ODE_DeviceBusy:
         operation = "Device is busy.";
         break;

      case ODE_IllegalCommand:
         operation = "Illegal command. Please try again.";
         break;

      case ODE_TooManyTracks:
         operation = "Too many tracks.";
         break;

      case ODE_TooManySessions:
         operation = "Too many sessions.";
         break;

      case ODE_IllegalParameter:
         operation = "Illegal parameter, device refused to accept settings.";
         break;

      case ODE_NoDisc:
         operation = "Disc has been replaced.";
         break;

      case ODE_NotEnoughSpace:
         operation = "Not enough free space.";
         break;

      case ODE_DiscNotEmpty:
         operation = "Disc not empty.";
         break;

      case ODE_BadSessionNumber:
         operation = "Bad session number.";
         break;

      case ODE_BadTrackNumber:
         operation = "Bad track number.";
         break;

      case ODE_BadIndexNumber:
         operation = "Bad index number.";
         break;

      case ODE_BadLayout:
         operation = "Bad layout.";
         break;

      case ODE_DiscFull:
         operation = "Disc is full.";
         break;

      case ODE_DiscReserved:
         operation = "Disc has been reserved.";
         break;

      case ODE_DiscNotFormatted:
         operation = "Disc is not formatted.";
         break;

      case ODE_DeviceNotReady:
         operation = "Disc is not ready.";
         break;

      case ODE_BadTrackMode:
         operation = "Bad track type.";
         break;

      case ODE_BadBlockNumber:
         operation = "Bad block number.";
         break;

      case ODE_Exiting:
         operation = "Device rejected command because it is now cleaning up.";
         break;

      case ODE_DriveLockedForWrite:
         operation = "Device locked for write.";
         break;
         
      case ODE_DriveLockedForRead:
         operation = "Device locked for read.";
         break;

      case ODE_BadTransferSize:
         operation = "Bad transfer size.";
         break;

      case ODE_NoDataReturned:
         operation = "No data returned by drive.";
         break;
    }

   _dx(Lvl_Info, "Operation failed with result %ld (%s)", ret, (iptr)operation.Data());

   if (ret != ODE_OK)
   {
      operation += "\nwhile writing disc";
      request("Error during write", operation.Data(), "Abort", 0);
   }
}


uint32 JobUpload::getProgress()
{
   uint64 s1, s2;

   s2 = numBlocks;
   s1 = currBlock;
   while (s2 >= 16384)
   {
      s2 >>= 1;
      s1 >>= 1;
   }

   return ((iptr)s1 * 65535) / (iptr)s2;
}

const char *JobUpload::getActionName()
{
   return operation.Data();
}

bool JobUpload::inhibitDOS()
{
   return true;
}
