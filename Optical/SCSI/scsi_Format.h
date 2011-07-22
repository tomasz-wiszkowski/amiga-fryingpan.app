#ifndef __SCSI_FORMAT_H
#define __SCSI_FORMAT_H

#include "SCSICommand.h"
#include "uniform.h"

enum FormatType 
{
    Format_FullFormat             = 0x00,
    Format_SpareAreaExpansion     = 0x01,
    Format_ZoneReformat           = 0x04,
    Format_ZoneFormat             = 0x05,
    Format_CD_DVD_FullFormat      = 0x10,
    Format_CD_DVD_GrowSession     = 0x11,
    Format_CD_DVD_AddSession      = 0x12,
    Format_CD_DVD_QuickGrowSession= 0x13,
    Format_DVDM_QuickAddSession   = 0x14,
    Format_DVDM_QuickFormat       = 0x15,
    Format_MRW_Quick_Obsolete     = 0x20,
    Format_MRW_FullFormat         = 0x24,
    Format_DVDP_FullFormat        = 0x26
};

class scsi_Format : public SCSICommand
{
    FormatType	ftype;
    uint32	fblks;
    int32	fparm;
    bool	fimmed;
    bool	ffrmt;

    struct _fmt_capacity
    {
	aLong    num_blocks;
	struct _1 : protected aLong
	{
	    uint8  getType1() { return getField(26, 6); }
	    uint8  getType2() { return getField(24, 2); }
	    uint32 getParam() { return getField(0, 24); }
	} conf;
    };

    struct _readcaps 
    {
	uint8                                     pad0[3];
	uint8                                     length;
	scsi_Format::_fmt_capacity   capacities[0];
    };

private:
    struct  // fd
    {
	uint8    resvd0;
	struct _1 : protected aByte
	{
	    void setOptionsValid()  { setField(7, 1, 1); }
	    void setDisablePrimary(){ setField(6, 1, 1); }
	    void setCertificates()  { setField(5, 1, 1); }
	    void setStopFormat()    { setField(4, 1, 1); }
	    void setInitPattern()   { setField(3, 1, 1); }
	    void setTryOut()        { setField(2, 1, 1); }
	    void setImmediate()     { setField(1, 1, 1); }
	    void setVendorSpecific(){ setField(0, 1, 1); }
	} flags __attribute__((packed));

	aWord    length;     // 8
	aLong    size;

	struct _2 : protected aLong
	{
	    void setType(uint8 t)   { setField(26, 6, t); }
	    void setParam(uint32 t) { setField(0, 24, t); }
	} type __attribute__((packed));
    } fd;

    _readcaps  *caps;

public:

    scsi_Format(DriveIO &, DriveStatus&);
    ~scsi_Format();
    bool onInit();
    bool onProbe(bool, uint32);

    bool IsFormatted();
    void SetType(bool format, FormatType type=Format_FullFormat, uint32 blocks=0, int32 param=0);
    void SetImmediate(bool);

    /* alternative */
    bool ReadFormats();
    uint32 GetMaxCapacity();
    uint32 GetMaxPacketLength();
};

#endif

