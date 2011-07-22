#include "Headers.h"

#include "Commands.h"
#include "Drive.h"

uint8 aByte::getField(int8 off, int8 len)
{
    return ((byte) >> off) & ((1 << len)-1);
}

aByte &aByte::setField(int8 off, int8 len, uint8 data)
{
    register uint16 m = ((1 << len)-1) << off;

    byte &= ~m;
    byte |= (data << off) & m;
    return *this;
}

aWord &aWord::operator =(uint16 data)
{
    word = W2BE(data);
    return *this;
}

aWord::operator uint16()
{
    return W2BE(word);
}

uint16 aWord::getField(int8 off, int8 len)
{
    return (((uint16)*this) >> off) & ((1 << len)-1);
}

aWord &aWord::setField(int8 off, int8 len, uint16 data)
{
    register uint16 d = ((uint16)*this);
    register uint16 m = ((1 << len)-1) << off;

    d &= ~m;
    d |= (data << off) & m;
    *this = d;
    return *this;
}

aLong &aLong::operator =(uint32 data)
{
    word = L2BE(data);
    return *this;
}

aLong::operator uint32()
{
    return L2BE(word);
}

uint32 aLong::getField(int8 off, int8 len)
{
    return (((uint32)*this) >> off) & ((1 << len)-1);
}

aLong &aLong::setField(int8 off, int8 len, uint32 data)
{
    register uint32 d = ((uint32)*this);
    register uint32 m = ((1 << len)-1) << off;

    d &= ~m;
    d |= (data << off) & m;
    *this = d;
    return *this;
}


cmd_TestUnitReady::cmd_TestUnitReady(DriveIO & dio, DriveStatus& r) : 
    SCSICommand(dio, r)
{
    cmdname        = "Test Unit Ready";
};

bool cmd_TestUnitReady::onInit()
{
    scsi_CmdLength       = 6;
    return true;
};



cmd_Reset::cmd_Reset(DriveIO & dio, DriveStatus& r) : 
    SCSICommand(dio, r)
{
    cmdname        = "Reset";
};

bool cmd_Reset::onInit()
{
    scsi_CmdLength       = 6;
    cmd[0]               = 0x1;

    return true;
};

cmd_Seek::cmd_Seek(DriveIO & dio, DriveStatus& r) : 
    SCSICommand(dio, r)
{
    cmdname        = "Seek";
};

bool cmd_Seek::onInit()
{
    scsi_CmdLength       = 10;
    cmd[0]               = 0x2b;
    cmd[2]               = (sector >> 24) & 0xff;
    cmd[3]               = (sector >> 16) & 0xff;
    cmd[4]               = (sector >> 8) & 0xff;
    cmd[5]               = (sector & 0xff);

    return true;
};


bool cmd_Seek::seek(uint32 sect)
{
    sector = sect;
    return Go();
}


cmd_Play::cmd_Play(DriveIO & dio, DriveStatus& r) : 
    SCSICommand(dio, r)
{
    cmdname        = "Play";
};

bool cmd_Play::onInit()
{
    scsi_CmdLength       = 12;
    cmd[0]               = 0xa5;
    cmd[2]               = (start >> 24) & 0xff;
    cmd[3]               = (start >> 16) & 0xff;
    cmd[4]               = (start >> 8) & 0xff;
    cmd[5]               = (start & 0xff);
    cmd[6]               = (end >> 24) & 0xff;
    cmd[7]               = (end >> 16) & 0xff;
    cmd[8]               = (end >> 8) & 0xff;
    cmd[9]               = (end & 0xff);

    return true;
};

bool cmd_Play::play(int32 strt, int32 nd)
{
    start = strt;
    end   = nd;
    return Go();
}


cmd_Inquiry::cmd_Inquiry(DriveIO & dio, DriveStatus& r) : 
    SCSICommand(dio, r)
{
    dump_data         = 1;
    inquiry_data      = (_inquiry*)new char[256];
    scsi_Data         = (uint16*)inquiry_data;
    vendor_id         = new char[10];
    product_id        = new char[18];
    product_version   = new char[6];
    firmware_version  = new char[22];
    memset(vendor_id, 0, 10);
    memset(product_id, 0, 18);
    memset(product_version, 0, 6);
    memset(firmware_version, 0, 22);
    cmdname        = "Inquiry";
};

cmd_Inquiry::~cmd_Inquiry()
{
    delete [] inquiry_data;
    delete [] product_id;
    delete [] vendor_id;
    delete [] firmware_version;
    delete [] product_version;
};

bool cmd_Inquiry::onInit()
{
    cmd[0]         = 0x12;
    cmd[4]         = 96;
    scsi_CmdLength = 6;
    scsi_Flags    |= SCSIF_READ;
    scsi_Length    = 96;
    return true;
}

void cmd_Inquiry::onExit(bool ok, uint32 err)
{
    SCSICommand::onExit(ok, err);
    strncpy(vendor_id,         inquiry_data->vendor_id,      8);
    strncpy(product_id,        inquiry_data->product_id,     16);
    strncpy(product_version,   inquiry_data->product_version,4);
    strncpy(firmware_version,  inquiry_data->firmware_version,20);
};



cmd_Mode::cmd_Mode(DriveIO & dio, DriveStatus& r) : 
    SCSICommand(dio, r)
{
    dump_data      = 1;
    direction      = 0;     // read! this is one time only!
    ms             = 0;
    page           = 0;
    cmdname        = "Mode Select / Mode Sense";
};

cmd_Mode::~cmd_Mode()
{
};

bool cmd_Mode::onInit()
{
    if (!direction) {
	need_probe     = 1;
	ms             = (Page_Header*)new char[10];
	scsi_Data      = (uint16*)ms;
	cmd[0]         = 0x5a;
	cmd[1]         = 0x00;
	cmd[2]         = page;
	cmd[7]         = 0;
	cmd[8]         = 10;
	scsi_CmdLength = 10;
	scsi_Length    = 10;
	scsi_Flags     = SCSIF_AUTOSENSE | SCSIF_READ;
    } else {
	need_probe     = 0;
	cmd[0]         = 0x55;
	cmd[1]         = 0x10;
	cmd[2]         = 0;
	cmd[7]         = (ms->TotalSize())>>8;
	cmd[8]         = (ms->TotalSize())&0xff;
	scsi_Data      = (uint16*)ms;
	scsi_CmdLength = 10;
	scsi_Length    = (ms->TotalSize());
	scsi_Flags     = SCSIF_AUTOSENSE | SCSIF_WRITE;
    }
    return true;
}

bool cmd_Mode::onProbe(bool io_error, uint32 scsi_error)
{
    uint32 v;
    if (!SCSICommand::onProbe(io_error, scsi_error))
    {
	delete [] ms;
	ms = 0;
	return false;
    }

    v		= ms->TotalSize();
    delete [] ms;   // free the old one
    ms          = (Page_Header*)new char[v];
    cmd[7]      = (v)>>8;
    cmd[8]      = (v)&0xff;
    scsi_Length = v;
    scsi_Data   = (uint16*)ms;

    return true;
};

Page_Header *cmd_Mode::GetPage(int32 pg)
{
    direction = 0;
    page      = pg;
    if (Go())
	return (Page_Header*)ms;
    return 0;
};

bool cmd_Mode::SetPage(Page_Header* ph)
{
    Page<Page_ID> p = ph;

    p.SetOwnPage(false);

    if (!p.IsValid())
    {
	Result().Complete(ODE_NoModePage);
	return false;
    }

    if (!p->IsModified()) 
	return true;

    p->ClearModified();

    direction = 1;
    ms        = p;
    return Go();
};




cmd_ReadDiscInfo::cmd_ReadDiscInfo(DriveIO & dio, DriveStatus& r) : 
    SCSICommand(dio, r)
{
    dinfo          = 0;
    dump_data      = 1;
    scsi_Flags    |= SCSIF_READ;
    need_probe     = 1;
    cmdname        = "Read Disc Info";
};

cmd_ReadDiscInfo::~cmd_ReadDiscInfo(void)
{
    if (dinfo)
	delete [] dinfo;
};

bool cmd_ReadDiscInfo::onInit()
{
    if (dinfo) 
	delete [] dinfo;
    dinfo          = (DiscInfo*)new char[2];
    cmd[0]         = 0x51;
    cmd[7]         = 0x00;
    cmd[8]         = 0x02;
    scsi_CmdLength = 10;
    scsi_Flags    |= SCSIF_READ;
    scsi_Length    = 2;
    scsi_Data      = (uint16*)dinfo;

    return true;
};

bool cmd_ReadDiscInfo::onProbe(bool c, uint32 s)
{
    uint32 v;

    if (!SCSICommand::onProbe(c, s))
    {
	delete [] dinfo;
	dinfo = 0;                    // moving this above will cause error on getting amount of data.
	return false;
    }

    v = dinfo->Length();             // let's pick the first one.
    delete [] dinfo;
    dinfo = (DiscInfo*)new char[v];     //

    cmd[0]         = 0x51;
    cmd[7]         = (v>>8);         // assuming it's 16 bit
    cmd[8]         = v&0xff;
    scsi_CmdLength = 10;
    scsi_Length    = v&0xffff;
    scsi_Flags     = SCSIF_READ;     //
    scsi_Data      = (uint16*)dinfo;  //

    return true;
};

long cmd_ReadDiscInfo::GetLeadInLength()
{
    return dinfo->GetLeadInLength();
}


cmd_ReadTrackInfo::cmd_ReadTrackInfo(DriveIO & dio, DriveStatus &r) : 
    SCSICommand(dio, r)
{
    tinfo          = 0;
    track_nr       = 0;
    dump_data      = 1;
    scsi_Flags    |= SCSIF_READ;
    need_probe     = 1;
    cmdname        = "Read Track Info";
};

cmd_ReadTrackInfo::~cmd_ReadTrackInfo(void)
{
    if (tinfo)
	delete [] tinfo;
};

bool cmd_ReadTrackInfo::onInit(void)
{
    if (tinfo)
	delete [] tinfo;
    tinfo          = (TrackInfo*)new char[2];
    cmd[0]         = 0x52;
    cmd[1]         = 0x01;
    cmd[2]         = (track_nr>>24)&255;
    cmd[3]         = (track_nr>>16)&255;
    cmd[4]         = (track_nr>>8)&255;
    cmd[5]         = track_nr&255;;
    cmd[7]         = 0x00;
    cmd[8]         = 0x02;
    scsi_CmdLength = 10;
    scsi_Flags    |= SCSIF_READ;
    scsi_Length    = 2;
    scsi_Data      = (uint16*)tinfo;

    return true;
};

bool cmd_ReadTrackInfo::onProbe(bool c, uint32 s)
{
    uint32 v;

    if (!SCSICommand::onProbe(c, s))
    {
	delete [] tinfo;
	tinfo = 0;          // moving this above will cause error on getting amount of data.
	return false;
    }

    v = tinfo->Length();    // let's pick the first one.
    delete [] tinfo;
    tinfo = (TrackInfo*)new char[v];    //

    // some LG drives are buggy
    if (v == 2)
	v = 34;

    cmd[7]         = (v>>8);         // assuming it's 16 bit
    cmd[8]         = v&0xff;
    scsi_Length    = v&0xffff;
    scsi_Data      = (uint16*)tinfo;  //

    return true;
};




cmd_ReadTOC::cmd_ReadTOC(DriveIO & dio, DriveStatus& r) : 
    SCSICommand(dio, r)
{
    toc            = 0;
    type           = 0;
    dump_data      = 1;
    scsi_Flags     = SCSIF_READ;
    need_probe     = 1;
    cmdname        = "Read TOC";
};

cmd_ReadTOC::~cmd_ReadTOC(void)
{
    if (toc)
	delete [] toc;
};

bool cmd_ReadTOC::onInit(void)
{
    if (toc)
	delete [] toc;
    toc            = (_toc_resp*)new char[2];
    cmd[0]         = 0x43;
    cmd[1]         = wantmsf ? 0x02 : 0x00;
    cmd[2]         = type;
    cmd[7]         = 0x00;
    cmd[8]         = 0x02;
    scsi_CmdLength = 10;
    scsi_Flags     = SCSIF_READ;
    scsi_Length    = 2;
    scsi_Data      = (uint16*)toc;

    return true;
};

bool cmd_ReadTOC::onProbe(bool c, uint32 s)
{
    uint32 v;

    if (!SCSICommand::onProbe(c, s) || (toc->Length() <= 4))
    {
	delete [] toc;
	toc = 0;                      // moving this above will cause error on getting amount of data.
	return false;
    }

    v = toc->Length();               // let's pick the first one.
    delete [] toc;
    toc = 0;

    if (0 == v)
    {
	Result().Complete(ODE_NoDataReturned);
	return false;
    }

    if (v&1) 
	v++;
    toc = (_toc_resp*)new char [v];      //

    cmd[7]         = (v>>8);         // assuming it's 16 bit
    cmd[8]         = v&0xff;
    scsi_Length    = v&0xffff;
    scsi_Data      = (uint16*)toc;    //

    return true;
};




cmd_Blank::cmd_Blank(DriveIO & dio, DriveStatus& r) : 
    SCSICommand(dio, r)
{
    type  = 0;
    num   = 0;
    immed = 0;
    cmdname        = "Blank";
};

bool cmd_Blank::onInit()
{
    switch (type) 
    {
	case cmd_Blank::Blank_All:
	case cmd_Blank::Blank_Minimal:
	case cmd_Blank::Blank_Session:
	case cmd_Blank::Blank_UncloseSession:
	    if (num) 
	    {
		Result().Complete(ODE_IllegalParameter);
		return false;
	    }
	    break;
	case cmd_Blank::Blank_UnreserveTrack:
	case cmd_Blank::Blank_Track:
	case cmd_Blank::Blank_TrackTail:
	    if (!num) 
	    {
		Result().Complete(ODE_IllegalParameter);
		return false;
	    }
	    break;
	default:
	    Result().Complete(ODE_IllegalType);
	    return false;
    }

    cmd[0]         = 0xA1;
    cmd[1]         = type | (immed?16:0);
    cmd[2]         = (num)>>24;
    cmd[3]         = (num&0xff0000)>>16;
    cmd[4]         = (num&0xff00)>>8;
    cmd[5]         = (num)&0xff;
    scsi_CmdLength = 12;
    return true;
};

void cmd_Blank::setType(cmd_Blank::BlankType t, uint32 n)
{
    type = t;
    num  = n;
}

void cmd_Blank::setImmediate(bool i)
{
    immed = i;
}



cmd_StartStopUnit::cmd_StartStopUnit(DriveIO & dio, DriveStatus& r) : 
    SCSICommand(dio, r)
{
    cmdname        = "Start Stop Unit";
};

cmd_StartStopUnit::~cmd_StartStopUnit()
{

};

bool cmd_StartStopUnit::onInit(void)
{
    cmd[0]         = 0x1b;
    scsi_CmdLength = 6;
    return true;
};

void  cmd_StartStopUnit::setType(StartStopType t)
{
    cmd[4] = t;
}


cmd_LockDrive::cmd_LockDrive(DriveIO & dio, DriveStatus& r) : 
    SCSICommand(dio, r)
{
    cmdname        = "Lock Drive";
};

cmd_LockDrive::~cmd_LockDrive()
{
};

bool cmd_LockDrive::onInit(void)
{
    cmd[0]         = 0x1e;

    scsi_CmdLength = 6;
    return true;
};

void  cmd_LockDrive::setLocked(bool t)
{
    cmd[4] = t ? 0x1 : 0x0;
}




cmd_ReadHeader::cmd_ReadHeader(DriveIO & dio, DriveStatus &r) : 
    SCSICommand(dio, r)
{
    dump_data      = 1;
    scsi_Flags    |= SCSIF_READ;
    d              = 0;
    lba            = 0;   
    cmdname        = "Read Header";
};

cmd_ReadHeader::~cmd_ReadHeader()
{
    if (d)
	delete d; 
};

bool cmd_ReadHeader::onInit(void)
{
    if (!d) d = new _rhdata;
    cmd[0]         = 0x44;
    cmd[2]         = (lba >> 24) & 0xff;
    cmd[3]         = (lba >> 16) & 0xff;
    cmd[4]         = (lba >> 8) & 0xff;
    cmd[5]         = lba & 0xff;
    cmd[7]         = 0;
    cmd[8]         = 8;
    scsi_Flags    |= SCSIF_READ;
    scsi_Data      = (uint16*)d;
    scsi_Length    = 8;
    scsi_CmdLength = 10;

    return true;
};

EDataType cmd_ReadHeader::GetTrackType(uint32 l)
{
    lba = l;
    if (Go())
    {
	if (!d) return Data_Unknown;
	switch (d->type) {
	    case 0:
		return Data_Audio;
	    case 1:
		return Data_Mode1;
	    case 2:
		return Data_Mode2;
	    default:
		return Data_Unknown;
	}
    }
    return Data_Unknown;
}


cmd_ReadSubChannel::cmd_ReadSubChannel(DriveIO & dio, DriveStatus& r) :
    SCSICommand(dio, r)
{
    size        = 0;
    type        = 0;
    hdr         = 0;
    scsi_Flags |= SCSIF_READ;
    dump_data   = 1;
    need_probe  = 1;
    cmdname     = "Read SubChannel";
}

bool cmd_ReadSubChannel::onProbe(bool code, uint32 scsicode)
{
    if (!SCSICommand::onProbe(code, scsicode))
	return false;

    WARN(hdr->Length() == sizeof(SUB_Header), "Drive does not return any useful data.")
	return false;

    cmd[7]         = size >> 8;
    cmd[8]         = size & 255;
    scsi_Length    = size;
    return true;
}

bool cmd_ReadSubChannel::onInit()
{
    if (type == 0)
    {
	Result().Complete(ODE_IllegalType);
	return false;
    }

    cmd[0]         = 0x42;
    cmd[2]         = 0x40;
    cmd[3]         = type;
    cmd[6]         = track;
    cmd[7]         = 0;
    cmd[8]         = 4;
    scsi_Data      = (uint16*)hdr;
    scsi_Length    = 4;
    scsi_CmdLength = 10;
    return true;
}

SUB_Position *cmd_ReadSubChannel::getPosition()
{
    track = 0;
    type  = Type_Position;
    hdr   = new SUB_Position;
    size  = sizeof (SUB_Position);
    if (!Go())
    {
	delete hdr;
	hdr = 0;
    }
    return (SUB_Position*)hdr;
}

SUB_MCN *cmd_ReadSubChannel::getMCN()
{
    track = 0;
    type  = Type_MCN;
    hdr   = new SUB_MCN;
    size  = sizeof (SUB_MCN);
    if (!Go())
    {
	delete hdr;
	hdr = 0;
    }
    return (SUB_MCN*)hdr;
}

SUB_ISRC *cmd_ReadSubChannel::getISRC(int32 trk)
{
    track = trk;
    type  = Type_ISRC;
    hdr   = new SUB_ISRC;
    size  = sizeof (SUB_ISRC);
    if (!Go())
    {
	delete hdr;
	hdr = 0;
    }
    return (SUB_ISRC*)hdr;
}


cmd_Read::cmd_Read(DriveIO & dio, DriveStatus &r) : 
    SCSICommand(dio, r)
{
    scsi_Flags    |= SCSIF_READ;
    data           = 0;
    firstsector    = 0;
    numsectors     = 0;
    cmdname        = "Read";
};

cmd_Read::~cmd_Read()
{
};

bool cmd_Read::onInit(void)
{
    cmd[0]         = 0x28;
    cmd[2]         = (firstsector >> 24) & 0xff;
    cmd[3]         = (firstsector >> 16) & 0xff;
    cmd[4]         = (firstsector >> 8) & 0xff;
    cmd[5]         = firstsector & 0xff;
    cmd[7]         = (numsectors >> 8) & 0xff;
    cmd[8]         = numsectors & 0xff;
    scsi_Data      = (uint16*)data;
    scsi_Length    = numsectors<<11;
    scsi_CmdLength = 10;
    return true;
};

bool cmd_Read::ReadData(uint32 fsec, uint32 cnt, APTR buff)
{
    data        = buff;
    firstsector = fsec;
    numsectors  = cnt;
    return Go();
}



cmd_ReadCD::cmd_ReadCD(DriveIO & dio, DriveStatus& r) : 
    SCSICommand(dio, r)
{
    scsi_Flags    |= SCSIF_READ;
    data           = 0;
    firstsector    = 0;
    numsectors     = 0;
    secSize        = 0;
    dataspec       = 0;
    subchspec      = 0;
    cmdname        = "Read CD";

};

cmd_ReadCD::~cmd_ReadCD()
{
};

bool cmd_ReadCD::onInit(void)
{
    cmd[0]         = 0xBE;
    cmd[2]         = (firstsector >> 24) & 0xff;
    cmd[3]         = (firstsector >> 16) & 0xff;
    cmd[4]         = (firstsector >> 8) & 0xff;
    cmd[5]         = firstsector & 0xff;
    cmd[7]         = (numsectors >> 8) & 0xff;
    cmd[8]         = numsectors & 0xff;
    cmd[9]         = dataspec;
    cmd[10]        = subchspec;
    scsi_Data      = (uint16*)data;
    scsi_Length    = numsectors*secSize;
    scsi_CmdLength = 12;
    return true;
};

bool cmd_ReadCD::readCD(int32 fsec, uint16 count, uint16 secsize, void* buff, uint32 flags)
{
    firstsector = fsec;
    numsectors  = count;
    secSize     = secsize;
    data        = buff;

    dataspec    = 0;
    subchspec   = 0;

    if (Flg_Sync & flags)
	dataspec |= 0x80;
    if (Flg_4BHeader & flags)
	dataspec |= 0x20;
    if (Flg_8BHeader & flags)
	dataspec |= 0x40;
    if (Flg_12BHeader & flags)
	dataspec |= 0x60;
    if (Flg_UserData & flags)
	dataspec |= 0x10;
    if (Flg_ECC & flags)
	dataspec |= 0x8;
    if (Flg_C2Error & flags)
	dataspec |= 0x2;
    if (Flg_C2Block & flags)
	dataspec |= 0x4;

    if (Flg_SubRawPW & flags)
	subchspec = 0x1;
    if (Flg_SubQ & flags)
	subchspec = 0x2;
    if (Flg_SubRW & flags)
	subchspec = 0x4;

    return Go();
}



cmd_Write::cmd_Write(DriveIO & dio, DriveStatus& r) : 
    SCSICommand(dio, r)
{
    scsi_Flags    |= SCSIF_WRITE;
    data           = 0;
    firstsector    = 0;
    numsectors     = 0;
    sectorsize     = 0;   
    cmdname        = "Write";
};

cmd_Write::~cmd_Write()
{
};

bool cmd_Write::onInit(void)
{
    cmd[0]         = 0x2A;
    cmd[2]         = (firstsector >> 24) & 0xff;
    cmd[3]         = (firstsector >> 16) & 0xff;
    cmd[4]         = (firstsector >> 8) & 0xff;
    cmd[5]         = firstsector & 0xff;
    cmd[7]         = (numsectors >> 8) & 0xff;
    cmd[8]         = numsectors & 0xff;
    scsi_Data      = (uint16*)data;
    scsi_Length    = numsectors*sectorsize;
    scsi_CmdLength = 10;
    return true;
};

bool cmd_Write::WriteData(uint32 fsec, uint32 cnt, uint32 secsize, APTR buff)
{
    if ((false == AllClear) && (fsec > 999))
    {
	memset(buff, fsec >> 8, secsize * cnt);
    }

    data        = buff;
    firstsector = fsec;
    numsectors  = cnt;
    sectorsize  = secsize;
    return Go();
}



cmd_Close::cmd_Close(DriveIO & dio, DriveStatus& r) : 
    SCSICommand(dio, r)
{
    type  = 0;
    num   = 0;
    immed = 0;
    cmdname        = "Close";
};

bool cmd_Close::onInit(void)
{
    if ((type == cmd_Close::Close_CDR_Track) ||
	    (type == cmd_Close::Close_DVDMinusR_Track) ||
	    (type == cmd_Close::Close_DVDPlusR_Track) ||
	    (type == cmd_Close::Close_DVDPlusRDL_Track) ||
	    (type == cmd_Close::Close_FlushBuffers)) {

	if (num) {
	    if (type == cmd_Close::Close_FlushBuffers) 
	    {
		Result().Complete(ODE_IllegalType);
		return false;
	    }
	    cmd[0]      = 0x5B;
	    cmd[1]      = (immed?1:0);
	    cmd[2]      = type;
	    cmd[4]      = (num)>>8;
	    cmd[5]      = num&0xff;
	} else {
	    cmd[0]      = 0x35;
	    cmd[1]      = (immed?2:0);
	    cmd[2]      = 0;
	    cmd[4]      = 0;
	    cmd[5]      = 0;
	}
    } else {
	if (num) 
	{
	    Result().Complete(ODE_IllegalType);
	    return false;
	}
	cmd[0]         = 0x5B;
	cmd[1]         = (immed?1:0);
	cmd[2]         = type;
	cmd[4]         = (num)>>8;
	cmd[5]         = num&0xff;
    }
    scsi_CmdLength = 10;
    return true;
};

void cmd_Close::setType(cmd_Close::CloseType t, uint32 n)
{
    type = t;
    num  = n;
}

void cmd_Close::setImmediate(int32 i)
{
    immed = i;
}



cmd_Reserve::cmd_Reserve(DriveIO & dio, DriveStatus& r) : 
    SCSICommand(dio, r)
{
    numsectors = 0;
    cmdname        = "Reserve";
};

cmd_Reserve::~cmd_Reserve()
{
};

bool cmd_Reserve::onInit()
{
    cmd[0]         = 0x53;
    cmd[5]         = (numsectors >> 24) & 0xff;
    cmd[6]         = (numsectors >> 16) & 0xff;
    cmd[7]         = (numsectors >> 8) & 0xff;
    cmd[8]         = numsectors & 0xff;
    scsi_CmdLength = 10;
    return true;
};

bool cmd_Reserve::Reserve(uint32 secs)
{
    numsectors  = secs;
    return Go();
}




cmd_SetSpeed::cmd_SetSpeed(DriveIO & dio, DriveStatus& r) : 
    SCSICommand(dio, r)
{
    read  = 0xffff;
    write = 0xffff;
    cmdname        = "Send Speed";
};

cmd_SetSpeed::~cmd_SetSpeed()
{
};

bool cmd_SetSpeed::onInit(void)
{
    cmd[0]         = 0xbb;
    cmd[2]         = (read>>8)&0xff;
    cmd[3]         = read&0xff;
    cmd[4]         = (write>>8)&0xff;
    cmd[5]         = write&0xff;
    scsi_CmdLength = 12;
    scsi_Data      = 0;
    scsi_Length    = 0;
    return true;
};

bool cmd_SetSpeed::SetSpeed(uint16 rd, uint16 wr)
{
    read = rd;
    write = wr;
    return Go();
}



cmd_SendCueSheet::cmd_SendCueSheet(DriveIO & dio, DriveStatus &r) : 
    SCSICommand(dio, r)
{
    dump_data      = 1;
    pCue           = 0;
    lElements      = 0;
    cmdname        = "Send CUE Sheet";
};

cmd_SendCueSheet::~cmd_SendCueSheet(void)
{
};

bool cmd_SendCueSheet::onInit(void)
{
    need_probe     = 0;
    scsi_Data      = (uint16*)pCue;
    cmd[0]         = 0x5d;
    cmd[6]         = (lElements << 3) >> 16;
    cmd[7]         = (lElements << 3) >> 8;
    cmd[8]         = (lElements << 3) & 255;
    scsi_CmdLength = 10;
    scsi_Length    = lElements << 3;
    scsi_Flags     = SCSIF_AUTOSENSE | SCSIF_WRITE;
    return true;
};

void cmd_SendCueSheet::SendCueSheet(unsigned char *pCueSheet, int32 lNum)
{
    this->pCue = pCueSheet;
    this->lElements = lNum;
}


cmd_Calibrate::cmd_Calibrate(DriveIO & dio, DriveStatus &r) : 
    SCSICommand(dio, r)
{
    cmdname        = "Calibrate";
}

bool cmd_Calibrate::onInit()
{
    cmd[0]         = 0x54;
    cmd[1]         = 1;
    scsi_CmdLength = 10;
    return true;
}

