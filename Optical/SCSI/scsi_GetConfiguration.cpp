#include "scsi_GetConfiguration.h"

scsi_GetConfiguration::scsi_GetConfiguration(DriveIO & dio, DriveStatus& r) : SCSICommand(dio, r)
{
    feature_data   = (_feature*)new char[65536];
    scsi_Data      = (uint16*)feature_data;
    dump_data      = 1;
    read_media     = 0;
    write_media    = 0;
    scsi_Flags    |= SCSIF_READ;
    need_probe     = 1;
    cmdname        = "Get Configuration";
};

scsi_GetConfiguration::~scsi_GetConfiguration(void)
{
    if (feature_data)
	delete [] feature_data;
};

bool scsi_GetConfiguration::onInit()
{
    cmd[0]         = 0x46;
    cmd[7]         = 0x00;
    cmd[8]         = 0x04;
    scsi_CmdLength = 10;
    scsi_Flags    |= SCSIF_READ;
    scsi_Length    = 4;

    return true;
};

bool scsi_GetConfiguration::onProbe(bool c, uint32 s)
{
    uint32 v;

    if (!SCSICommand::onProbe(c, s)) {
	delete [] feature_data;
	feature_data = 0;
	return false;
    }

    v = feature_data->length + 4;   // length indicates number of bytes following.

    cmd[0]         = 0x46;
    cmd[7]         = (v>>8);            // assuming it's 16 bit
    cmd[8]         = v&0xff;
    scsi_CmdLength = 10;
    scsi_Flags    |= SCSIF_READ;
    scsi_Length    = v&0xffff;

    return true;
};

int32 scsi_GetConfiguration::GetCurrentProfile()
{
    if (!feature_data) return DRT_Profile_Unknown;

    switch (feature_data->current_profile) {
	case 8:  return DRT_Profile_CD_ROM;
	case 9:  return DRT_Profile_CD_R;
	case 10: return DRT_Profile_CD_RW;
	case 16: return DRT_Profile_DVD_ROM;
	case 17: return DRT_Profile_DVD_MinusR;
	case 18: return DRT_Profile_DVD_RAM;
	case 19: return DRT_Profile_DVD_MinusRW_Restricted;
	case 20: return DRT_Profile_DVD_MinusRW_Sequential;
	case 26: return DRT_Profile_DVD_PlusRW;
	case 27: return DRT_Profile_DVD_PlusR;
	case 43: return DRT_Profile_DVD_PlusR;
	case 64: return DRT_Profile_BD_ROM;
	case 65: return DRT_Profile_BD_R_Sequential;
	case 66: return DRT_Profile_BD_R_RandomWrite;
	case 67: return DRT_Profile_BD_RW;
	default: return DRT_Profile_Unknown;
    }
}

Feature* scsi_GetConfiguration::GetNextFeature(Feature *f)
{
    if (!feature_data) 
	return 0;

    if (f == 0)
	return (Feature*)&feature_data->profiles;
  
    {
	f = f->Next();
	iptr p = ((uint8*)f) - (uint8*)(feature_data->profiles);
	iptr m = feature_data->length - 4;
	if (p >= m)
	    return 0;
	return f;
    }
}

Feature *scsi_GetConfiguration::getFeature(Feat_ID num)
{
    uint8    *f;
    uint32     max;
    uint32     pos=0;
    Feature  *feat;

    f = (uint8*)feature_data;
    if (!f) return 0;

    pos = 8;       // skip _feature
    max = feature_data->length+4;

    while (pos < max) {
	feat = (Feature*)&f[pos];
	if (feat->GetId() == num) return feat;
	pos += feat->GetLength();
    }
    return 0;
}

uint32 scsi_GetConfiguration::GetMediaReadSupport(void)
{
    uint32             supp=0;
    int32               total;
    Feat_ProfileList *prof;


    if (read_media)      return read_media;
    if (!feature_data)   return 0;

    prof  = GetFeature<Feat_ProfileList>();
    if (!prof) return supp;    // no profiles, no support.

    total = (prof->GetLength()-1)>>2;   // get length...

    while (total--) {
	switch (prof->items[total].profile) {
	    case 8:  supp |= DRT_Media_CD_ROM;        break;
	    case 9:  supp |= DRT_Media_CD_R;          break;
	    case 10: supp |= DRT_Media_CD_RW;         break;
	    case 16: supp |= DRT_Media_DVD_ROM;       break;
	    case 17: supp |= DRT_Media_DVD_MinusR;    break;
	    case 18: supp |= DRT_Media_DVD_RAM;       break;
	    case 19: supp |= DRT_Media_DVD_MinusRW;   break;
	    case 20: supp |= DRT_Media_DVD_MinusRW;   break;
	    case 26: supp |= DRT_Media_DVD_PlusRW;    break;
	    case 27: supp |= DRT_Media_DVD_PlusR;     break;
	    case 43: supp |= DRT_Media_DVD_PlusR_DL;  break;
	    case 64: supp |= DRT_Media_BD_ROM;        break;
	    case 65: supp |= DRT_Media_BD_R;          break;
	    case 66: supp |= DRT_Media_BD_R;          break;
	    case 67: supp |= DRT_Media_BD_RW;         break;
	}
    }
    read_media = supp;
    return supp;
}

uint32 scsi_GetConfiguration::GetMediaWriteSupport(void)
{
    uint32                supp=0;
    Feat_CD_TrackAtOnce	     *cd;
    Feat_CD_SessionAtOnce    *sao;
    uint32                    read=GetMediaReadSupport();

    if (write_media)   return write_media;
    if (!feature_data) return 0;

    if (read & DRT_Media_CD_R) 
    {
	cd = GetFeature<Feat_CD_TrackAtOnce>();
	if (cd) 
	{
	    supp |= DRT_Media_CD_R;
	    if ((cd->conf.isReWritable()) && (read & DRT_Media_CD_RW)) 
	    {
		supp |= DRT_Media_CD_RW;
	    }
	}

	sao = GetFeature<Feat_CD_SessionAtOnce>();
	if (sao) 
	{
	    supp |= DRT_Media_CD_R;
	    if ((sao->conf.isReWritable()) && (read & DRT_Media_CD_RW)) 
	    {
		supp |= DRT_Media_CD_RW;
	    }
	}
    }

    if (read & DRT_Media_CD_RW) 
    {
	if (GetFeature<Feat_CD_RW_MediaWriteSupp>())   
	    supp |= DRT_Media_CD_RW;
    }

    if (read & (DRT_Media_BD_R | DRT_Media_BD_RW)) 
    {
	if (GetFeature<Feat_BD_Write>())
	    supp |= DRT_Media_BD_R | DRT_Media_BD_RW;
    }

    if (read & (DRT_Media_DVD_MinusR | DRT_Media_DVD_MinusRW)) 
    {
	Feat_DVD_MinusR_RW_Write *minus = GetFeature<Feat_DVD_MinusR_RW_Write>();
	if (minus) 
	{
	    supp |= DRT_Media_DVD_MinusR;

	    if ((read & DRT_Media_DVD_MinusRW) && (minus->conf.isReWritable())) 
	    {
		supp |= DRT_Media_DVD_MinusRW;
	    }
	}
    }


    if (read & DRT_Media_DVD_PlusRW) 
    {
	Feat_DVD_PlusRW *plus = GetFeature<Feat_DVD_PlusRW>();
	if (plus) 
	{
	    if (plus->conf.doesWrite()) 
		supp |= DRT_Media_DVD_PlusRW;
	}
    }

    if (read & DRT_Media_DVD_PlusR) 
    {
	Feat_DVD_PlusR *plus = GetFeature<Feat_DVD_PlusR>();
	if (plus) 
	{
	    if (plus->conf.isWritable()) 
		supp |= DRT_Media_DVD_PlusR;
	}
    }

    if (read & DRT_Media_DVD_PlusR_DL) 
    {
	Feat_DVD_PlusR_DualLayer *plus = GetFeature<Feat_DVD_PlusR_DualLayer>();
	
	if (plus) 
	{
	    if (plus->conf.doesWriteDualLayer()) 
		supp |= DRT_Media_DVD_PlusR_DL;
	}
    }

    if (read & DRT_Media_DVD_RAM) 
    {
	if (GetFeature<Feat_RandomWritable>())
	    supp |= DRT_Media_DVD_RAM;
    }

    write_media = supp;
    return supp;
}

uint32 scsi_GetConfiguration::GetDriveCapabilities(void)
{
    uint32 feats = 0;
    Feature *f;
    if (!feature_data) return 0;

    f = GetFeature<Feat_WriteProtect>();
    if (f != 0)
	feats |= DRT_Can_Do_WriteProtect;

    return feats;
}



