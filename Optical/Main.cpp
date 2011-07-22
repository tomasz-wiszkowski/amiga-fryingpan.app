#include <Generic/LibrarySpool.h>
#include <libclass/intuition.h>
#include "Headers.h"
#include "Drive.h"
#include "Main.h"
#include <exec/memory.h>
#include "IOptItem.h"
#include "OptDisc.h"
#include "OptTrack.h"
#include "IDriveClient.h"

#include <libclass/utility.h>
#include <PlugLib/PlugLib.h>
#include <dos/dos.h>
#include <dos/dos.h>

#include <Generic/HashT.h>

uint32 StartupFlags = 0;
PlugLibIFace* plg = 0;

using namespace GenNS;
static char programname[64];
static BPTR out;

#if 0
iptr DoLibMethodA(iptr *parm)
{
    request("Info", "Sorry, interface has changed.", "Ok", 0);
    return 0;
}
{
   LONG           rc  = 0;
   Drive         *drv = 0;

   if (!parm) return 0;
   drv = (Drive*)parm[1];

   switch (*parm) {
      case DRV_ScanDevice:       return Drive::ScanDevice((char*)parm[1]);
      case DRV_FreeScanResults:  return Drive::FreeScanResults((ScanData*)parm[1]);
      case DRV_NewDrive:         return (iptr)DriveSpool::GetDriveClient((char*)parm[1], parm[2]);
      case DRV_CloneDrive:       return (iptr)DriveSpool::CloneDrive(drv);
      case DRV_EndDrive:         DriveSpool::FreeDriveClient(drv); return 0;
      case DRV_GetAttr:          return drv->GetDriveAttrs(parm[2]);
      case DRV_GetAttrs:         return drv->GetDriveAttrs((struct TagItem*)&parm[2]);
      default:                   return drv->SendMessage(parm);
   }
   return rc;
}
      
class IOptItem* OptCreateDisc()
{
   return new OptDisc();
}
#endif
  
struct args 
{
    char    *drive;
    int     *unit;
    int      show_contents;

    int      quick_erase;
    int      complete_erase;
    int      quick_format;
    int      complete_format;
    int      prepare_disc;

    int      check_writable;
    int      check_erasable;
    int      check_formattable;
    int      check_overwritable;
    int	     check_formatted;

    int      start;
    int      stop;
    int      load;
    int      eject;
    int      idle;
    int      standby;
    int      sleep;
    int      write_image;
    int     *read_track;
    char    *read_to;
    int      layout_track;
    int     *writekbps;
    int     *readkbps;
    int      testmode;
    int      closetrack;
    int      closesession;
    int      closedisc;
    int      getwritabletrks;
    int      writedao;
    char    *data;
    char   **audio;
    int      wait_forever;
    int      close_tracks;
    int     *write_protect;

    const char* GetTemplate(void)
    {
	return "DRIVE/A,UNIT/A/K/N,SHOWCONTENTS=SC/S,"
		"QUICKERASE=QERA/S,COMPLETEERASE=CERA/S,QUICKFORMAT=QFMT/S,COMPLETEFORMAT=CFMT/S,PREPAREDISC=PREP/S,"
		"CHECKWRITABLE=ISWRT/S,CHECKERASABLE=ISERA/S,CHECKFORMATABLE=ISFMT/S,CHECKOVERWRITABLE=ISOVW/S,CHECKFORMATTED/S"
		"START/S,STOP/S,LOAD/S,EJECT/S,IDLE/S,STANDBY/S,SLEEP/S,"
	    "WRITEDISC/S,READTRACK/K/N,TO/K,LAYOUTDISC/S,WRITEKBPS/N,READKBPS/N,TESTMODE/S,"
	    "CLOSETRACK/K/N,CLOSESESSION/S,CLOSEDISC/S,GETWRITABLETRACKS/S,DAOMODE/S,DATATRACK/K,AUDIOTRACKS/K/M,WAITFOREVER/S,CLOSETRACKS/S,WRITEPROTECT/K/N";
    };

    void Init()
    {
	drive                = 0;
	unit                 = 0;
	show_contents        = 0;
	quick_erase          = 0;
	complete_erase       = 0;
	prepare_disc         = 0;
	check_writable       = 0;
	check_erasable       = 0;
	check_formattable    = 0;
	quick_format         = 0;
	complete_format      = 0;
	start                = 0;
	stop                 = 0;
	load                 = 0;
	eject                = 0;
	idle                 = 0;
	standby              = 0;
	sleep                = 0;
	check_overwritable   = 0;
	write_image          = 0;
	read_track           = 0;
	read_to              = 0;
	layout_track         = 0;
	readkbps             = 0;
	writekbps            = 0;
	testmode             = 0;
	closetrack           = 0;
	closesession         = 0;
	getwritabletrks      = 0;
	writedao             = 0;
	data                 = 0;
	audio                = 0;
	wait_forever         = 0;
	closedisc            = 0;
	write_protect        = 0;
	close_tracks         = 0;
    };
};

#define HASH_INSERT(hash, val) (hash).Add(val, #val)

static TagItem our_tags[] =
{
    { PLO_NameSpace,      (iptr)"FryingPan"	},
    { PLO_PluginName,     (iptr)Optical_Name	},
    { PLO_MinVersion,     Optical_Version	},
    { PLO_MinRevision,    Optical_Revision	},
    { 0,		0			}
};

class Main
{
    args     arg;
    RDArgs  *rda;
    int      rc;
    Call2T<void, Main, IDriveClient&, const IDriveStatus&> call;
    OpticalPlugin& opt;
    HashT <int32, const char*> operations;
    HashT <int32, const char*> opstatuses;


protected:
    void buildHashes()
    {
	{
	    /// drive main operations ///////
	    HASH_INSERT(operations, DRT_Operation_Unknown);
	    HASH_INSERT(operations, DRT_Operation_Ready);
	    HASH_INSERT(operations, DRT_Operation_OpeningTray);
	    HASH_INSERT(operations, DRT_Operation_ClosingTray);
	    HASH_INSERT(operations, DRT_Operation_IdleNoDisc);

	    ///***************************************** playback control
	    HASH_INSERT(operations, DRT_Operation_Play);
	    HASH_INSERT(operations, DRT_Operation_Pause);
	    HASH_INSERT(operations, DRT_Operation_Read);
	    /// other operations ////////////

	    ///***************************************** drive control
	    HASH_INSERT(operations, DRT_Operation_Control_General);
	    HASH_INSERT(operations, DRT_Operation_Control_Eject);
	    HASH_INSERT(operations, DRT_Operation_Control_Load);
	    HASH_INSERT(operations, DRT_Operation_Control_SpinUp);
	    HASH_INSERT(operations, DRT_Operation_Control_SpinDown);
	    HASH_INSERT(operations, DRT_Operation_Control_Standby);
	    HASH_INSERT(operations, DRT_Operation_Control_Lock);
	    HASH_INSERT(operations, DRT_Operation_Control_Unlock);
	    ///***************************************** other types may follow.

	    ///***************************************** drive & disc analysis
	    HASH_INSERT(operations, DRT_Operation_Analyse_General);
	    HASH_INSERT(operations, DRT_Operation_Analyze_Drive);
	    HASH_INSERT(operations, DRT_Operation_Analyse_Tracks);
	    HASH_INSERT(operations, DRT_Operation_Analyse_Indices);
	    HASH_INSERT(operations, DRT_Operation_Analyse_DataType);
	    HASH_INSERT(operations, DRT_Operation_Analyse_CDText);
	    HASH_INSERT(operations, DRT_Operation_Analyse_FreeDB);
	    HASH_INSERT(operations, DRT_Operation_Analyse_DiscID);
	    HASH_INSERT(operations, DRT_Operation_Analyse_ControlBlocks);
	    HASH_INSERT(operations, DRT_Operation_Analyse_Speeds);
	    HASH_INSERT(operations, DRT_Operation_Analyse_Layout);
	    ///***************************************** other types may follow.

	    ///***************************************** different types of write
	    HASH_INSERT(operations, DRT_Operation_Write_General);
	    HASH_INSERT(operations, DRT_Operation_Write_Calibrate);
	    HASH_INSERT(operations, DRT_Operation_Write_Allocate);
	    HASH_INSERT(operations, DRT_Operation_Write_CDText);
	    HASH_INSERT(operations, DRT_Operation_Write_Data);
	    HASH_INSERT(operations, DRT_Operation_Write_Synchronize);
	    HASH_INSERT(operations, DRT_Operation_Write_CloseTrack);
	    HASH_INSERT(operations, DRT_Operation_Write_CloseSession);
	    HASH_INSERT(operations, DRT_Operation_Write_CloseDisc);
	    HASH_INSERT(operations, DRT_Operation_Write_Repair);
	    ///***************************************** other types may follow.

	    ///***************************************** different types of erase
	    HASH_INSERT(operations, DRT_Operation_Erase_General);
	    HASH_INSERT(operations, DRT_Operation_Erase_BlankComplete);
	    HASH_INSERT(operations, DRT_Operation_Erase_BlankFast);
	    HASH_INSERT(operations, DRT_Operation_Erase_BlankSession);
	    HASH_INSERT(operations, DRT_Operation_Erase_FormatComplete);
	    HASH_INSERT(operations, DRT_Operation_Erase_FormatFast);
	    HASH_INSERT(operations, DRT_Operation_Erase_FormatSession);
	};

	{
	    HASH_INSERT(opstatuses, DRT_OpStatus_Ready);
	    HASH_INSERT(opstatuses, DRT_OpStatus_InProgress);
	    HASH_INSERT(opstatuses, DRT_OpStatus_Completed);
	    HASH_INSERT(opstatuses, DRT_OpStatus_Aborted);
	};
    }

public:
    Main() :
	rda(0),
	call(this, &Main::notified),
	opt(*((OpticalPlugin*)plg->OpenPlugin(our_tags))),
	operations("*** Unknown ***"),
	opstatuses("*** Unknown ***")
    {
	/*
	** here's the funny thing:
	** we still want to work even if another application is running.
	** therefore we cannot simply call our own code, sorry...
	*/
	arg.Init();

	buildHashes();

	if (opt.IsValid())
	{
	    rda = DOS->ReadArgs(const_cast<char*>(arg.GetTemplate()), (void**)&arg, 0);
	    if (rda != NULL) 
	    {
		rc = execute(opt, arg);
	    } 
	    else
	    {
		DOS->PrintFault(ERROR_REQUIRED_ARG_MISSING, programname);
	    }
	}
	else
	{
	    DOS->PrintFault(ERROR_OBJECT_NOT_FOUND, programname);
	    request("Error", "You cannot start me from another directory, sorry.\nI must at least be able to lock onto myself.", "Ok", 0);
	}
    }

    ~Main()
    {
	if (rda)
	    DOS->FreeArgs(rda);

	opt.Dispose();
    }

    void notified(IDriveClient& cl, const IDriveStatus& op)
    {
	/*
	** when this happens, we are NOT ALLOWED to take much time to process
	** we are called from the context of FryingPan here. it is not delegated
	*/
	const char* oname = operations.GetVal(op.operation);
	const char* sname = opstatuses.GetVal(op.status);

	DOS->VFPrintf(out, "Drive status update:\n"
		    "	Status     : %lx (%s)\n"
		    "	Operation  : %lx (%s)\n"
		    "	Progress   : %ld.%ld\n"
		    "	Error      : %ld (%lx)\n"
		    "	Description: %s\n",
		    ARRAY(op.status, (uint32)sname,
			  op.operation, (uint32)oname,
			  op.progress_major, op.progress_minor, 
			  op.error, op.scsi_error, 
			  (iptr)op.description));
    }

    bool waitDiscPresent(IDriveClient *cl)
    {
	char tmp[8];
	DOS->PutStr("Insert disc and press <enter>: ");
	DOS->FGets(DOS->Input(), tmp, sizeof(tmp));
	for (int i=0; i<10; i++)
	{
	    if (cl->isDiscPresent())
	    {
		DOS->VPrintf("got it!\n", 0);
		return true;
	    }
	    DOS->PutStr(".");
	    DOS->Flush(DOS->Output());
	    DOS->Delay(50);
	}
	DOS->VPrintf("no disc in drive.\n", 0);
	return false;
    }

    int execute(OpticalPlugin& opt, args& a)
    {
	IDriveClient* dcl = opt->OpenDrive(a.drive, *a.unit);

	dcl->setNotifyCallback(&call);

	if (a.eject)
	    dcl->eject();
	else if (a.load)
	    dcl->load();
	else if (a.show_contents)
	    showContents(dcl);
	else if (a.quick_erase)
	{
	    if (waitDiscPresent(dcl))
		dcl->blank(DRT_Blank_Erase_Fast);
	}
	else if (a.complete_erase)
	{
	    if (waitDiscPresent(dcl))
		dcl->blank(DRT_Blank_Erase_Complete);
	}
	else if (a.quick_format)
	{
	    if (waitDiscPresent(dcl))
		dcl->blank(DRT_Blank_Format_Fast);
	}
	else if (a.complete_format)
	{
	    if (waitDiscPresent(dcl))
		dcl->blank(DRT_Blank_Format_Complete);
	}
	else if (a.prepare_disc)
	{
	    if (waitDiscPresent(dcl))
		dcl->blank(DRT_Blank_Default);
	}
	else if (a.check_writable)
	{
	    if (waitDiscPresent(dcl))
		DOS->VPrintf("Disc %s wirtable\n", ARRAY((iptr)(dcl->isWritable() ? "is" : "is not")));
	}
	else if (a.check_erasable)
	{
	    if (waitDiscPresent(dcl))
		DOS->VPrintf("Disc %s erasable\n", ARRAY((iptr)(dcl->isErasable() ? "is" : "is not")));
	}
	else if (a.check_formattable)
	{
	    if (waitDiscPresent(dcl))
		DOS->VPrintf("Disc %s formattable\n", ARRAY((iptr)(dcl->isFormattable() ? "is" : "is not")));
	}
	else if (a.check_overwritable)
	{
	    if (waitDiscPresent(dcl))
		DOS->VPrintf("Disc %s overwritable\n", ARRAY((iptr)(dcl->isOverwritable() ? "is" : "is not")));
	}
	else if (a.check_formatted)
	{
	    if (waitDiscPresent(dcl))
		DOS->VPrintf("Disc %s formatted\n", ARRAY((iptr)(dcl->isFormatted() ? "is" : "is not")));
	}


	dcl->waitComplete();
	dcl->dispose();
	return 0;
    }

    void showContents(IDriveClient* dcl)
    {
	const IOptItem* item = 0;

	if (!waitDiscPresent(dcl))
	    return;
	
	item = dcl->getDiscContents();
	if (item)
	{
	    printTrackInfo(item, true);
	    item->release();
	}
    }

    void printTrackInfo(const IOptItem* item, bool recursive)
    {
	DOS->VPrintf("%s %ld\n", ARRAY(
				       item->getItemType() == Item_Disc ? (int)"Disc" :
				       item->getItemType() == Item_Session ? (int)"Session" :
				       item->getItemType() == Item_Track ? (int)"Track" :
				       item->getItemType() == Item_Index ? (int)"Index" : (int)"Unknown",
				       item->getItemNumber()
				      ));

	DOS->VPrintf("\tLocation   : %ld - %ld (%ld blocks)\n", ARRAY(item->getStartAddress(), item->getEndAddress(), item->getBlockCount()));
	DOS->VPrintf("\tType       : %s\n", ARRAY(item->getDataType() == Data_Unknown   ? (int)"Unknown" :
						  item->getDataType() == Data_Audio     ? (int)"Audio" :
						  item->getDataType() == Data_Mode1     ? (int)"Data, Mode 1" :
						  item->getDataType() == Data_Mode2     ? (int)"Data, Mode 2" :
						  item->getDataType() == Data_Mode2Form1? (int)"Data, Mode 2, Form 1" :
						  item->getDataType() == Data_Mode2Form2? (int)"Data, Mode 2, Form 2" :
						  (int)"Illegal track type."));
	DOS->VPrintf("\tSec Size   : %ld bytes\n", ARRAY(item->getSectorSize()));
	DOS->VPrintf("\tBlank      : %s\n", ARRAY(item->isBlank()       ? (int)"Yes"         : (int)"No"));
	DOS->VPrintf("\tIncomplete : %s\n", ARRAY(item->isComplete()    ? (int)"No"          : (int)"Yes"));
	DOS->VPrintf("\tIncremental: %s\n", ARRAY(item->isIncremental() ? (iptr)"Yes"        : (iptr)"No"));
	DOS->VPrintf("\tCDText     : %s\n", ARRAY(item->hasCDText()     ? (int)"Available"   : (int)"Unavailable"));
	if (item->hasCDText()) 
	{
	    DOS->VPrintf("\t- Artist   : %s\n", ARRAY((int)item->getCDTArtist()));
	    DOS->VPrintf("\t- Title    : %s\n", ARRAY((int)item->getCDTTitle()));
	    DOS->VPrintf("\t- Message  : %s\n", ARRAY((int)item->getCDTMessage()));
	    DOS->VPrintf("\t- Lyrics   : %s\n", ARRAY((int)item->getCDTLyrics()));
	    DOS->VPrintf("\t- Composer : %s\n", ARRAY((int)item->getCDTComposer()));
	    DOS->VPrintf("\t- Director : %s\n", ARRAY((int)item->getCDTDirector()));
	}

	if (recursive)
	{
	    for (int i=0; i<item->getChildCount(); i++)
	    {
		printTrackInfo(item->getChild(i), true);
	    }
	}
    }
};

int main()
{ 
    Exec = ExecIFace::GetInstance(SysBase);
    DOS = DOSIFace::GetInstance(0);
    plg = PlugLibIFace::GetInstance(0);

    DOS->GetProgramName(programname, sizeof(programname));
    out = DOS->Output();

    if (plg != 0)
    {
	delete new Main;
	plg->FreeInstance();
    }
    else
    {
	DOS->PrintFault(ERROR_OBJECT_NOT_FOUND, programname);
	request("Error", "If you want to execute me from shell,\nmake sure i can find plug.library.", "Ok", 0);
    }

    DOS->FreeInstance();
    Exec->FreeInstance();
}

#if 0
{

   
   if (!dcl)
   {
      DOS->VPrintf("Unable to open drive. Exiting.\n", 0);
      DOS->FreeArgs(rda); 
      CleanUp();
      return 20;
   }

   DOS->VPrintf("Please insert disc in drive (unless already inserted)\n", 0);

   
   if ((iptr)DoLibMethodA(ARRAY(DRV_WaitForDisc, dcl, arg.wait_forever ? 0xffffffff : 30)) == (iptr)ODE_NoDisc)
   {
      DOS->VPrintf("No disc inserted.\n", 0);
      DoLibMethodA(ARRAY(DRV_EndDrive, dcl));
      DOS->FreeArgs(rda);
      CleanUp();
      return 20;
   }

   int spd_read=0,
       spd_write=0;

   if (arg.readkbps)  spd_read  = *arg.readkbps;
   if (arg.writekbps) spd_write = *arg.writekbps;

   DOS->VPrintf("Selected read speed:  %ld\n", ARRAY(spd_read));
   DOS->VPrintf("Selected write speed: %ld\n", ARRAY(spd_write));

   DoLibMethodA(ARRAY(DRV_SetAttr, dcl, DRA_Drive_TestMode, arg.testmode));

   else if (arg.show_contents) 
   {
      int         num;
      IOptItem   *di;

      DOS->VPrintf("Please insert a disc (unless it is already inserted).\n", 0);

      DoLibMethodA(ARRAY( DRV_GetAttrs,           dcl,
                        DRA_Disc_NumTracks,     (int)&num,
                        DRA_Disc_Contents,      (int)&di,
                        TAG_DONE,               0));

      DOS->VPrintf("Number of tracks: %ld\n", ARRAY(num));

      showDetails(di);
      for (int i=0; i<di->getChildCount(); i++)
      {
         const IOptItem *sess = di->getChild(i);
         showDetails(sess);
         
         for (int j=0; j<sess->getChildCount(); j++)
         {
            const IOptItem *trak = sess->getChild(j);
            showDetails(trak);

            for (int k=0; k<trak->getChildCount(); k++)
            {
               const IOptItem *indx = trak->getChild(k);
               showDetails(indx);
            }
         }
      }
   }
   else if ((arg.layout_track) || (arg.write_image))
   {
      int         num;

      num = DoLibMethodA(ARRAY(DRV_GetAttr, dcl, DRA_Disc_IsWritable));

      if (!num) 
      {
         DOS->VPrintf("Disc is not writable!!!!\n", 0);
      } 
      else 
      {
         IOptItem      *disc, *sess, *trak;
         IOptItem     **tbl;
         FileInfoBlock  fib;
         BPTR           fh;

         DOS->VPrintf("Got writable disc!\n", 0);

         int d=0, a=0, t=0;
         
         if (arg.data)
            d = 1;
         if (arg.audio)
            while (arg.audio[a++]);
         if (a) a--;
         t = d + a;                    // total number of tracks;   
      
         DOS->VPrintf("Got %ld data and %ld audio tracks (total: %ld tracks)\n", ARRAY(d, a, t));
     
         /*
          * create disc and session items.
          */
         disc = OptCreateDisc();
         sess = disc->addChild();
         tbl  = new IOptItem*[t];

         /*
          * create data track
          */
         if (0 != d)
         {
            DOS->VPrintf("Accessing file %s\n", ARRAY((ULONG)arg.data));
            fh = DOS->Open(arg.data, MODE_OLDFILE);
            if (0 != fh)
            {
               if (DOS->ExamineFH(fh, &fib))
               {
                  trak = sess->addChild();
                  trak->setDataType(Data_Mode1);
                  trak->setDataBlockCount(fib.fib_Size >> 11);
                  tbl[0] = trak;
               }
               DOS->Close(fh);
            }
         }

         /*
          * create tracks.
          */
         for (int i=0; i<a; i++)
         {
            DOS->VPrintf("Accessing file %s\n", ARRAY((ULONG)arg.audio[i]));
            fh = DOS->Open(arg.audio[i], MODE_OLDFILE);
            if (0 != fh)
            {
               if (0 != DOS->ExamineFH(fh, &fib))
               {
                  trak = sess->addChild();
                  trak->setDataType(Data_Audio);
                  trak->setDataBlockCount(fib.fib_Size / 2352);
                  tbl[i+d] = trak;
               }
               DOS->Close(fh);
            }
         }
  
         /*
          * set disc flags
          */
         disc->setFlags((arg.writedao     ? DIF_Disc_MasterizeCD  : 0) |
                        (arg.closesession ? DIF_Disc_CloseSession : 
                         arg.closedisc    ? DIF_Disc_CloseDisc    : 0));
            
         DOS->VPrintf("Lying tracks...\n", 0);
   
         num = DoLibMethodA(ARRAY(DRV_LayoutTracks, dcl, (int)disc));
   
         DOS->VPrintf("Layout returned: %ld\n", ARRAY(num));
   
         if (arg.write_image)
         {
            DOS->VPrintf("Preparing for upload...\n", 0);
            num = DoLibMethodA(ARRAY(DRV_UploadLayout, dcl, (int)disc));
            if (!num) 
            {
               for (int i=0; i<t; i++)
               {
                  int   sec = 0;
                  int   count = 0;
                  int   size = 0;
                  int   rem = 0;
                  uint8*buf = 0;
                  
                  trak  = tbl[i];
                  count = trak->getPacketSize();
                  rem   = trak->getDataBlockCount();
                  size  = count * trak->getSectorSize();
                  buf   = new uint8[size];
                     
                  if ((d) && (!i))
                  {
                     DOS->VPrintf("Opening file: %s\n", ARRAY((ULONG)arg.data));
                     fh = DOS->Open(arg.data, MODE_OLDFILE);\
                  }
                  else                     
                  {
                     DOS->VPrintf("Opening file: %s\n", ARRAY((ULONG)arg.audio[i-d]));
                     fh = DOS->Open(arg.audio[i-d], MODE_OLDFILE);
                  }
                     
                  while (rem) 
                  {
                     DOS->VPrintf("Writing sector %ld, remaining: %ld\n", ARRAY(sec, rem));
                     size  = rem > count ? count : rem;
                     DOS->Read(fh, buf, size * trak->getSectorSize());

                     num = DoLibMethodA(ARRAY(DRV_WriteSequential, dcl, (int)buf, size));
                     sec += size;
                     rem -= size;
                     if (num) break;
                  }
                  if (num) 
                  {
                     DOS->VPrintf("Error while uploading track data: %ld\n", ARRAY(num));
                  }
                  DOS->Close(fh); 
                  delete [] buf;
               }
            } 
            else 
            {
               DOS->VPrintf("Layout upload failed %ld. Aborting.\n", ARRAY(num));
            }
         }
   
         /*
          * and this is meant to dispose all items: disc, session and tracks.
          */
         disc->dispose();
         delete [] tbl;
      }
   }
   else if (arg.read_track) 
   {
      int               num;
      IOptItem         *di;
      const IOptItem   *trak = 0;

      if ((arg.read_to) && (*arg.read_track>0)) 
      {
         BPTR  fh;

         fh = DOS->Open(arg.read_to, MODE_NEWFILE);

         if (fh) 
         {
            DOS->VPrintf("Please insert a disc (unless it is already inserted).\n", 0);
            DoLibMethodA(ARRAY( DRV_GetAttrs,           dcl,
                              DRA_Disc_NumTracks,     (int)&num,
                              DRA_Disc_Contents,      (int)&di,
                              TAG_DONE,               0));
            if (num >= *arg.read_track) 
            {
               for (int i=0; i<di->getChildCount(); i++)
               {
                  for (int j=0; j<di->getChild(i)->getChildCount(); j++)
                  {
                     if (di->getChild(i)->getChild(j)->getItemNumber() == *arg.read_track)
                     {
                        trak = di->getChild(i)->getChild(j);
                        break;
                     }
                  }
                  if (0 != trak)
                     break;
               }

               if (0 != trak) 
               {
                  trak->claim();
                  uint8 *buff = new uint8[32*trak->getSectorSize()];

                  if (0 != buff) 
                  {
                     int csec, rsec;

                     csec = 0;
                     rsec = trak->getDataBlockCount();

                     while (rsec) 
                     {
                        int len = (rsec > 32) ? 32 : rsec;

                        DoLibMethodA(ARRAY( DRV_ReadTrackRelative, dcl, (int)trak, csec, len, (int)buff));
                        DOS->Write(fh, buff, len*trak->getSectorSize());

                        rsec -= len;
                        csec += len;
                     }
                     delete [] buff;
                  }  

                  trak->dispose();
               }
            }
            DOS->Close(fh);
         }
      }
   }
   else if (arg.getwritabletrks) 
   {
      IOptItem *di = 0;

      DOS->VPrintf("Please insert a disc (unless it is already inserted).\n", 0);

      for (DoLibMethodA(ARRAY(DRV_GetAttrs, dcl, DRA_Disc_NextWritableTrack, (int)&di, TAG_DONE, 0)); di;
         DoLibMethodA(ARRAY(DRV_GetAttrs, dcl, DRA_Disc_NextWritableTrack, (int)&di, TAG_DONE, 0)))
      {
         DOS->VPrintf("Track %ld\n", ARRAY(di->getItemNumber()));
         if (di->isIncremental())
         {
            DOS->VPrintf("\tLocation   : %ld - %ld (%ld blocks total, random writable)\n", ARRAY(di->getStartAddress(), di->getEndAddress(), di->getBlockCount()));
         }
         else
         {
            DOS->VPrintf("\tLocation   : %ld - %ld (%ld blocks written, %ld blocks total)\n", ARRAY(di->getStartAddress(), di->getEndAddress(), 0, di->getBlockCount()));
         }
         DOS->VPrintf("\tType       : %s\n", ARRAY(  di->getDataType() == Data_Unknown     ? (int)"Unknown" :
                                                di->getDataType() == Data_Audio       ? (int)"Audio" :
                                                di->getDataType() == Data_Mode1       ? (int)"Data, Mode 1" :
                                                di->getDataType() == Data_Mode2       ? (int)"Data, Mode 2" :
                                                di->getDataType() == Data_Mode2Form1  ? (int)"Data, Mode 2, Form 1" :
                                                di->getDataType() == Data_Mode2Form2  ? (int)"Data, Mode 2, Form 2" :
                                                                                        (int)"Illegal track type."));
         DOS->VPrintf("\tSec Size   : %ld bytes\n", ARRAY(di->getSectorSize()));
         DOS->VPrintf("\tBlank      : %s\n", ARRAY(di->isBlank() == 1 ? (int)"Yes" : (int)"No"));
      }
   }
   else if (arg.start) 
   {
      DoLibMethodA(ARRAY(DRV_ControlUnit, dcl, DRT_Unit_Start));
   }
   else if (arg.stop) 
   {
      DoLibMethodA(ARRAY(DRV_ControlUnit, dcl, DRT_Unit_Stop));
   }
   else if (arg.idle) 
   {
      DoLibMethodA(ARRAY(DRV_ControlUnit, dcl, DRT_Unit_Idle));
   }
   else if (arg.standby) 
   {
      DoLibMethodA(ARRAY(DRV_ControlUnit, dcl, DRT_Unit_Standby));
   }
   else if (arg.closetrack) 
   {
      DOS->VPrintf("Close track... \n", 0);
      rc = DoLibMethodA(ARRAY(DRV_CloseDisc, dcl, DRT_Close_Track, *((int*)arg.closetrack)));
      DOS->VPrintf("Close track result: %ld\n", ARRAY(rc));
   }
   else if (arg.closesession) 
   {
      DOS->VPrintf("Close session... \n", 0);
      rc = DoLibMethodA(ARRAY(DRV_CloseDisc, dcl, DRT_Close_Session));
      DOS->VPrintf("Close session result: %ld\n", ARRAY(rc));
   }
   else if (arg.closedisc) 
   {
      DOS->VPrintf("Close disc... \n", 0);
      rc = DoLibMethodA(ARRAY(DRV_CloseDisc, dcl, DRT_Close_Finalize));
      DOS->VPrintf("Close disc result: %ld\n", ARRAY(rc));
   }
   else if (arg.sleep) 
   {
      DoLibMethodA(ARRAY(DRV_ControlUnit, dcl, DRT_Unit_Sleep));
   }
   else if (arg.close_tracks)
   {
      int         num;
      IOptItem   *di;

      DOS->VPrintf("Please insert a disc (unless it is already inserted).\n", 0);

      DoLibMethodA(ARRAY( DRV_GetAttrs,           dcl,
                        DRA_Disc_NumTracks,     (int)&num,
                        DRA_Disc_Contents,      (int)&di,
                        TAG_DONE,               0));

      DOS->VPrintf("Number of tracks: %ld\n", ARRAY(num));

      showDetails(di);
      for (int i=0; i<di->getChildCount(); i++)
      {
         const IOptItem *sess = di->getChild(i);
         showDetails(sess);
         
         for (int j=0; j<sess->getChildCount(); j++)
         {
            const IOptItem *trak = sess->getChild(j);
            showDetails(trak);
            if (!trak->isComplete())
               DoLibMethodA(ARRAY(DRV_CloseDisc, dcl, DRT_Close_Track, trak->getItemNumber()));
         }
      }
   }
   else if (arg.write_protect)
   {
      int         num;
      DOS->VPrintf("Setting write protection status to %s\n", ARRAY((iptr)(*arg.write_protect ? "enabled" : "disabled")));
      DoLibMethodA(ARRAY( DRV_SetAttr, dcl, DRA_Disc_WriteProtect, *arg.write_protect, 0));
      DOS->VPrintf("Done. Checking new write protection status\n", 0);
      DoLibMethodA(ARRAY( DRV_GetAttrs, dcl, DRA_Disc_WriteProtect, (iptr)&num, 0));
      DOS->VPrintf("Done. New write protection status: %s\n", ARRAY((iptr)(num ? "on" : "off")));
   }

   do 
   {
      DOS->VPrintf("%08lx: Current drive status: %ld\n", ARRAY((ULONG)dcl, DoLibMethodA(ARRAY(DRV_GetAttr, dcl, DRA_Drive_Status))));
      DOS->Delay(50);
   } while (!(Exec->SetSignal(0, 0) & SIGBREAKF_CTRL_C));

   if (rda) DOS->FreeArgs(rda);

   DoLibMethodA(ARRAY(DRV_EndDrive, dcl));
   CleanUp();
}
#endif
