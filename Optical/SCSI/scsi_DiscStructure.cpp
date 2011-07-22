#include "scsi_DiscStructure.h"

scsi_DiscStructure::scsi_DiscStructure(DriveIO & dio, DriveStatus& r) : SCSICommand(dio, r)
{
    dump_data      = 1;
};

scsi_DiscStructure::~scsi_DiscStructure()
{
};

bool scsi_DiscStructure::onProbe(bool c, uint32 s)
{
    uint32 v;

    if (!SCSICommand::onProbe(c, s))
    {
	delete data;
	data = 0;
	return false;
    }

    v = data->Length();
    delete data;
    data = (DiscStructure*)new char[v];   

    cmd[8]         = (v>>8);         // assuming it's 16 bit
    cmd[9]         = v&0xff;
    scsi_Length    = v&0xffff;
    scsi_Data      = (uint16*)data;  //

    return true;
};

bool scsi_DiscStructure::onInit()
{
    return true;
}

/*
 * read structure
 */
DiscStructure*	scsi_DiscStructure::readStructure(uint32 type, uint32 lba, uint8 layer, MediaType mediatype, uint32 agid)
{
    cmd[0]      = 0xAD;
    cmd[1]	= mediatype;
    cmd[2]      = (lba>>24)&255;
    cmd[3]      = (lba>>16)&255;
    cmd[4]      = (lba>>8)&255;
    cmd[5]      = lba&255;
    cmd[6]      = layer;
    cmd[7]      = type;
    cmd[8]	= 0;
    cmd[9]      = sizeof(DiscStructure);
    cmd[10]     = (agid & 3) << 6;
    cmd[11]     = 0;
    data	= new DiscStructure(4);
    scsi_Length = sizeof(DiscStructure);
    need_probe  = 1;
    scsi_Flags &= ~(SCSIF_WRITE | SCSIF_READ);
    scsi_Flags |= SCSIF_READ;
    scsi_Data   = (uint16*)data;
    scsi_CmdLength = 12;

    if (Go())
	return data;

    if (data != 0)
	delete data;
    data = 0;
    return 0;
}

/*
 * send structure
 */
bool scsi_DiscStructure::sendStructure(uint32 type, MediaType mediatype, DiscStructure* dsc, uint32 agid)
{
    cmd[0]      = 0xBF;
    cmd[1]	= mediatype;
    cmd[2]      = 0;
    cmd[3]      = 0;
    cmd[4]      = 0;
    cmd[5]      = 0;
    cmd[6]      = 0;
    cmd[7]      = type;
    cmd[8]	= dsc->Length() >> 8; 
    cmd[9]      = dsc->Length() & 255;
    cmd[10]     = (agid & 3) << 6;
    cmd[11]     = 0;
    scsi_Length = dsc->Length();
    need_probe  = 0;
    scsi_Flags &= ~(SCSIF_WRITE | SCSIF_READ);
    scsi_Flags |= SCSIF_WRITE;
    scsi_Data   = (uint16*)data;
    scsi_CmdLength = 12;

    return Go();
}

/*
		data[0] = len-2;
		data[1] = 0;
		data[2] = 0;
		data[3] = 0;
		data[4] = 0x3230;
		data[5] = 0x3035; // 2005
		data[6] = 0x3031; // 01
		data[7] = 0x3031; // 01
		data[8] = 0x3030; // 00
		data[9] = 0x3030; // 00
		data[10]= 0x3030; // 00
*/
