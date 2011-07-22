#include "scsi_Format.h"
#include <LibC/LibC.h>

scsi_Format::scsi_Format(DriveIO & dio, DriveStatus& r) : SCSICommand(dio, r)
{
    caps	    = 0;
    dump_data	    = 1;
    ffrmt	    = false;
    fblks	    = 0;
    ftype	    = Format_FullFormat;
    fparm	    = 0;
    fimmed	    = false;
};

scsi_Format::~scsi_Format()
{
    if (caps)
	delete [] caps;
};

bool scsi_Format::onProbe(bool c, uint32 s)
{
    uint32 v;

    if (!SCSICommand::onProbe(c, s))
    {
	delete [] caps;
	caps = 0;                     // moving this above will cause error on getting amount of data.
	return false;
    }

    v = caps->length;                // let's pick the first one.
    delete [] caps;

    if (v&7)
    {
	Result().Complete(ODE_CommandError, 0);
	return false;
    }

    v+=4;

    caps  = (scsi_Format::_readcaps*)new uint8[v];

    cmd[7]         = (v>>8);
    cmd[8]         = v&0xff;
    scsi_Length    = v&0xffff;
    scsi_Data      = (uint16*)caps;

    return true;
};

bool scsi_Format::IsFormatted(void)
{
    if (!caps) 
	return false;

    /*
     * fun facts:
     *  0 never happens
     *  1 means that medium is random writable(!!), so formatted?
     *  2 means that medium is either sequential or random writable
     *  3 means that medium is sequential writable
     *
     * this is actually sad because there is no way of telling...
     */

#warning IsFormatted() will never work good - MMC5 says that drive could be formatted for either random or sequential write.
    return (caps->capacities[0].conf.getType2() == 2) ? true : false;
}

uint32 scsi_Format::GetMaxCapacity(void)
{
    unsigned char i;
    unsigned long lBlocks = 0;

    if (!caps) return 0;

    for (i=0; i<(caps->length>>3); i++)
    {
	if (caps->capacities[i].conf.getType2() == 0x03)
	    continue;

	if ((caps->capacities[i].conf.getType1() == 0x00) ||
		(caps->capacities[i].conf.getType1() == 0x10) ||
		(caps->capacities[i].conf.getType1() == 0x26))
	{
	    if (lBlocks < caps->capacities[i].num_blocks)
		lBlocks = caps->capacities[i].num_blocks;
	}
    }
    return lBlocks;
};

uint32 scsi_Format::GetMaxPacketLength(void)
{
    unsigned char i;

    if (!caps) return 0;

    for (i=1; i<((caps->length>>3)); i++)
    {
	if (caps->capacities[i].conf.getType1() == 0x10)
	{
	    return caps->capacities[i].conf.getParam();
	}
    }
    return 16;
};

void scsi_Format::SetType(bool format, FormatType t, uint32 b, int32 param)
{
    ftype = t;
    fblks = b;
    fparm = param;
    ffrmt = format;
    need_probe  = !format;
}

void scsi_Format::SetImmediate(bool imm)
{
    fimmed = imm;
}

bool scsi_Format::onInit()
{
    if (ffrmt)
    {
	memset(&fd, 0, sizeof(fd));
	cmdname     = "Format";

	fd.type.setType(ftype);
	fd.size    = fblks;
	fd.type.setParam(fparm);
	fd.length  = 8;
	if (fimmed)
	    fd.flags.setImmediate();

	cmd[0]	    = 0x4;
	cmd[1]	    = 0x11;
	cmd[2]	    = 0x00;
	cmd[3]	    = 0x00;
	cmd[4]	    = 0x00;
	cmd[5]	    = 0x00;
	scsi_CmdLength  = 6;
	scsi_Data	= (uint16*)&fd;
	scsi_Length	= sizeof(fd);
	scsi_Flags     &= ~(SCSIF_READ | SCSIF_WRITE);
	scsi_Flags     |= SCSIF_WRITE;
    }
    else
    {
	need_probe  = 1;
	cmdname	    = "Read Format Capabilities";

	if (caps)
	    delete [] caps;
	caps	    = (scsi_Format::_readcaps*)new char[4];
	cmd[0]      = 0x23;
	cmd[1]      = 0x00;
	cmd[2]	    = 0x00;
	cmd[3]	    = 0x00;
	cmd[4]	    = 0x00;
	cmd[5]	    = 0x00;
	cmd[6]	    = 0x00;
	cmd[7]      = 0x00;
	cmd[8]      = 0x04;
	cmd[9]      = 0x00;
	scsi_CmdLength  = 10;
	scsi_Length = 4;
	scsi_Data   = (uint16*)caps;
	scsi_Flags &= ~(SCSIF_READ | SCSIF_WRITE);
	scsi_Flags |= SCSIF_READ;
    }

    return true;
}

bool scsi_Format::ReadFormats()
{
    ffrmt = false;
    return Go();
}


