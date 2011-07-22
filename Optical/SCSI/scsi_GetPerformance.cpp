#include "scsi_GetPerformance.h"

/*
** at most we will check 10 speeds.
*/
#define MAX_ITEMS 10

bool	Perf_PerformanceData::getPerfItem(int index, uint32 &sblk, uint32 &sspd, uint32 &eblk, uint32 &espd)
{
    ASSERTS(index < getPerfCount(), "Attempt to address performance item outside bounds");

    sblk = descriptors[index].start_lba;
    sspd = descriptors[index].start_performance;
    eblk = descriptors[index].end_lba;
    espd = descriptors[index].end_performance;

    return true;
}

int32   Perf_PerformanceData::getPerfCount()
{
    /*
    ** note length does not include length field itself
    ** so it's not the whole page header.
    */
    return ((length - 4) >> 4) <? MAX_ITEMS;
}

bool	Perf_WriteSpeeds::getSpeedItem(int index, uint32& rspd, uint32& wspd) 
{
    ASSERTS(index < getSpeedCount(), "Attempt to address performance item outside bounds");

    rspd = descriptors[index].read_speed;
    wspd = descriptors[index].write_speed;

    return true;
}

int32   Perf_WriteSpeeds::getSpeedCount()
{
    /*
    ** note length does not include length field itself
    ** so it's not the whole page header.
    */
    return ((length - 4) >> 4) <? MAX_ITEMS;
}


scsi_GetPerformance::scsi_GetPerformance(DriveIO & dio, DriveStatus& r) : SCSICommand(dio, r)
{
    perform_data   = 0;
    dump_data      = 1;
    scsi_Flags    |= SCSIF_READ;
    need_probe     = 0;
    cmdname        = "Get Performance";
};

scsi_GetPerformance::~scsi_GetPerformance()
{
};

bool scsi_GetPerformance::onInit()
{
    /*
    ** problem here identified:
    ** with too many speeds instead of receiving valid data
    ** we read junk. this is completely undesired
    ** could be system specific, i don't care. 
    ** right now we don't want more than MAX_ITEMS speeds.
    */
    perform_data    = (Perf_Header*)new char[MAX_ITEMS * 16 + 8];
    cmd[0]	    = 0xAC;
    cmd[1]	    = (type == PerfType_PerformanceData) ? (write ? 0x14 : 0x10) : 0;
    cmd[9]	    = MAX_ITEMS; 
    cmd[10]	    = type;
    scsi_CmdLength  = 12;
    scsi_Flags     |= SCSIF_READ;
    scsi_Length     = MAX_ITEMS * 16 + 8;
    scsi_Data	    = (uint16*)perform_data;

    return true;
};

Perf_Header* scsi_GetPerformance::readPerformance(scsi_GetPerformance::Perf_Type type, bool write)
{
    this->type = type;
    this->write = write;
    if (true != Go())
    {
	delete [] perform_data;
	perform_data = 0;
    }

    return perform_data;
}
    
Perf_PerformanceData* scsi_GetPerformance::readPerformanceData(bool write)
{
    return (Perf_PerformanceData*)(readPerformance(PerfType_PerformanceData, write));
}

Perf_WriteSpeeds* scsi_GetPerformance::readWriteSpeeds()
{
    return (Perf_WriteSpeeds*)(readPerformance(PerfType_WriteSpeeds, false));
}

