#ifndef __SCSI_DISCSTRUCTURE_H
#define __SCSI_DISCSTRUCTURE_H

#include "SCSICommand.h"
#include "uniform.h"
#include <Generic/String.h>

using namespace GenNS;

class DiscStructure
{
private:
    aWord       len;
    aWord       resvd;

protected:
    DiscStructure()
    {
    }

public:
    DiscStructure(int length)
    {
	len = length;
	resvd = 0;
    }

    int32 Length()
    {
	return len+2;
    }

    static uint8 Type()
    {
	return 0xff;
    }
};

class DVD_Physical : public DiscStructure
{
public:
    enum LayerType
    {
	Layer_Embossed = 0,
	Layer_Recordable,
	Layer_Rewritable,
	Layer_Unknown
    };

    enum DiscType
    {
	Disc_DVD_ROM = 0,
	Disc_DVD_RAM,
	Disc_DVD_MinusR,
	Disc_DVD_MinusRW,

	Disc_DVD_PlusRW=9,
	Disc_DVD_PlusR,

	Disc_DVD_PlusRW_DL=13,
	Disc_DVD_PlusR_DL
    };
private:
    struct _1 : public aLong
    {
	uint8 getBookType()        { return getField(28, 4); }
	uint8 getPartVersion()     { return getField(24, 4); }
	uint8 getDiscSize()        { return getField(20, 4); }
	uint8 getMaxRate()         { return getField(16, 4); }
	uint8 getLayerCount()      { return getField(13, 2); }
	bool  isTrackPath()        { return getField(12, 1); }
	uint8 getLayerType()       { return getField(8, 4);  }
	uint8 getLinearDensity()   { return getField(4, 4);  }
	uint8 getTrackDensity()    { return getField(0, 4);  }
    } conf;

    aLong             start_physical_sector;
    aLong             end_physical_sector;
    aLong             end_physical_sector_layer0;

    struct _2 : public aByte
    {
	bool isBurstCuttingAreaPresent() { return getField(7, 1); }
    } bca __attribute__((packed));

public:
    int32 GetDiscSize()
    {
	return conf.getDiscSize();
    }

    int32 GetNumLayers()
    {
	return conf.getLayerCount();
    }

    LayerType GetLayerType()
    {
	return (LayerType)conf.getLayerType();
    };

    DiscType GetDiscType()
    {
	return (DiscType)conf.getBookType();
    }

    bool BCAPresent()
    {
	return bca.isBurstCuttingAreaPresent();
    }

    static uint8 Type()
    {
	return 0x0;
    }
};

class DVD_Copyright : public DiscStructure
{
    enum ProtectionType
    {
	Prot_None,
	Prot_CSS,
	Prot_CPRM,

	Prot_Unknown
    };

protected:
    uint8             protection_type;
    uint8             region_management_information;      // each bit for one of eight regions
    uint16            reserved;                           // where disc can be played

public:  
    ProtectionType GetProtectionType()
    {
	return (ProtectionType)protection_type;
    }

    uint8 RegionsAllowed()
    {
	return region_management_information;
    }

    static uint8 Type()
    {
	return 0x01;
    }
};

class DVD_DiscKey: public DiscStructure
{
protected:
    uint8       key[2048];

public:
    uint8 *GetKey()
    {
	return (uint8*)&key;
    }

    static uint8 Type()
    {
	return 0x02;
    }
};

class DVD_BurstCutArea : public DiscStructure
{
protected:
    uint8       bca[0];

public:
    uint8 *GetBCA()
    {
	return (uint8*)&bca;
    }

    static uint8 Type()
    {
	return 0x03;
    }
};

class DVD_Manufacturer : public DiscStructure
{
protected:
    uint8       manufacturing_info[];

public:
    uint8 *GetManufacturingInfo()
    {
	return (uint8*)&manufacturing_info;
    }

    static uint8 Type()
    {
	return 0x04;
    }
};

class DVD_CopyrightMgmt : public DiscStructure
{
protected:
    uint8       copyright_mgmt;
    uint8       resvd0[3];

public:
    // these fields make sense only with minus r(w) and rom media.
    bool IsCPM()        
    {
	return (copyright_mgmt & 0x80) ? true : false;
    }

    bool IsCPSector()
    {
	return (copyright_mgmt & 0x40) ? true : false;
    }

    bool UseCSS()
    {
	if (IsCPSector())
	{
	    return (copyright_mgmt & 7) == 0;
	}
	return false;
    }

    bool UseCPPM()
    {
	if (IsCPSector())
	{
	    return (copyright_mgmt & 7) == 1;
	}
	return false;
    }

    static uint8 Type()
    {
	return 0x05;
    }
};

class DVD_MediaIdentifier : public DiscStructure
{
protected:
    uint8       media_id_data[0];

public:
    uint8 *GetMediaID()
    {
	return (uint8*)&media_id_data;
    };

    static uint8 Type()
    {
	return 0x06;
    }
};

class DVD_MediaKeyBlock : public DiscStructure
{
protected:
    uint8       media_key_block[0];

public:
    uint8 *GetMediaKeyBlock()
    {
	return (uint8*)&media_key_block;
    }

    static uint8 Type()
    {
	return 0x07;
    }
};

class DVD_PreRecordedLeadIn : public DiscStructure
{
protected:
    uint8             field_id_1;
    uint8             disc_application_code;

    struct _1 : public aLong
    {
	uint8  getDiscPhysicalSize()        { return getField(24, 8); }
	uint32 getLastRecordableAddress()   { return getField(0, 24); }
    } size __attribute__((packed));

    struct _2 : public aWord
    {
	uint8  getPartVersion()             { return getField(12, 4);  }
	uint8  getExtensionCode()           { return getField(8, 4);   }
    } part __attribute__((packed));

    uint8             field_id_2;
    uint8             opc_suggested_code;
    uint8             wavelength_code;
    uint8             write_strategy_code1[4];
    uint8             reserved1;

    uint8             field_id_3;
    uint8             manufacturer1[6];
    uint8             reserved2;

    uint8             field_id_4;
    uint8             manufacturer2[6];
    uint8             reserved3;

    uint8             field_id_5;
    uint8             write_strategy_code2[6];
    uint8             reserved4;

public:

    String GetManufacturer()
    {
	String s;
	if (field_id_3 == 3)
	    s += (char*)&manufacturer1;
	if (field_id_4 == 4)
	    s += (char*)&manufacturer2;

	return s;
    }

    static uint8 Type()
    {
	return 0x0e;
    }
};

class DVD_DiscControlBlock : public DiscStructure
{
    aLong             id;
    aLong             unknown_content_actions;
    char              vendor[32];
public:
    uint32 GetID()
    {
	return id;
    }

    String GetVendor()
    {
	char x[33];
	String s;
	strncpy(x, vendor, 32);
	x[32] = 0;
	s = (char*)&x;
	return s;
    }

    static uint8 Type()
    {
	return 0x30;
    }
};

/*
 * disc
 */
class Disc_WriteProtection: public DiscStructure
{
    struct _1 : public aLong
    {
	bool isMediaSpecificWriteInhibition()  { return getField(27, 1); }
	bool isCartridgeWriteProtection()      { return getField(26, 1); }
	bool isPersistentWriteProtection()     { return getField(25, 1); }
	bool isSoftwareWriteProtection()       { return getField(24, 1); }
	void setMediaSpecificWriteInhibition(int x)  { setField(27, 1, x); }
	void setCartridgeWriteProtection(int x)	    { setField(26, 1, x); }
	void setPersistentWriteProtection(int x)	    { setField(25, 1, x); }
	void setSoftwareWriteProtection(int x)	    { setField(24, 1, x); }
    } wpstatus;
public:
    bool isMediaSpecificWriteInhibition()     { return wpstatus.isMediaSpecificWriteInhibition(); }
    bool isCartridgeWriteProtection()         { return wpstatus.isCartridgeWriteProtection();     }
    bool isPersistentWriteProtection()        { return wpstatus.isPersistentWriteProtection();    }
    bool isSoftwareWriteProtection()          { return wpstatus.isSoftwareWriteProtection();      }

    Disc_WriteProtection(bool prot) :
	DiscStructure(sizeof(Disc_WriteProtection))
    {
	static_cast<aLong>(wpstatus) = 0;
	wpstatus.setPersistentWriteProtection(prot);
    }

    static uint8 Type() 
    {
	return 0xc0;
    }
};

struct Disc_RecognizedLayers: public DiscStructure
{
    uint8 NumLayers;
    struct _1 : public aByte
    {
	uint8 dfltFormatLayer() { return getField(4, 2); }
	uint8 onlineFormatLayer() { return getField(0, 2); }
    } LayerFormat;
    aWord LayerFormatTypeCode[0];

    static int8 Type()
    {
	return 0x90;
    }
};

/*
 * bluray
 */
class BD_DiscInformation : public DiscStructure
{
    struct
    {
	aWord   identifier;
	aByte   format;
	aByte   num_di_units_in_block;
	uint8   resvd0;
	aByte   di_seq_number;
	aByte   num_bytes_in_this_di;
	aByte   resvd1;
	uint8   type[3];
	uint8   disc_size_class_version;
	uint8   disc_specific_info[88];
	uint8   manufacturer_id[6];
	uint8   media_id[3];
	aWord   timestamp[2];
	uint8   product_revision_number;
    } DIEntry[0];
public:
    static uint8 Type()
    {
	return 0x0;
    }
};

class BD_DiscDefinitionStructure : public DiscStructure
{
    struct 
    {
	// psn == physical sector number
	uint8	identifier[2];
	uint8	dds_format;
	uint8	resvd0;
	aLong	dds_update_count;
	uint32	resvd1[2];
	aLong	first_psn_of_drive;
	uint32	resvd2;
	aLong	first_psn_of_defect_list;
	uint32	resvd3;
	aLong	psn_of_sector0_of_user_data;
	aLong	last_lsn_of_user_data;
	aLong	isa0_size;
	aLong	osa_size;
	aLong	isa1_size;
	uint8	spare_area_full_flags;
	uint8	resvd4;
	uint8	disc_type_specific_field1;
	uint8	resvd5;
	aLong	disc_type_specific_field2;
	uint32	resvd6;
	struct
	{
	    aLong	status_bits_of_info1;
	    aLong	status_bits_of_info2;
	    aLong	status_bits_of_pac1;
	    aLong	status_bits_of_pac2;
	} layers[0];
    } Entry[0];
public:
    static uint8 Type()
    {
	return 0x08;
    }
};

class BD_SpareAreaInformation : public DiscStructure
{
    uint32  resvd;
    aLong   num_spare_blocks;
    aLong   num_allocated_blocks;
public:
    static uint8 Type()
    {
	return 0x0a;
    }
};

class BD_RawDefectList
{
    aLong   dfl_from_package[0];
public:
    static uint8 Type()
    {
	return 0x12;
    }
};



class scsi_DiscStructure : public SCSICommand
 {
public:
    enum BDStructureType
    {
	BD_DiscInformation = 0,
	BD_DiscDefinitionStructure = 8,
	BD_CartridgeStatus,
	BD_SpareAreaInformation,
	BD_RawDefectList = 0x12,
	BD_PhysicalAccessControl = 0x30,
    };

    enum DVDStructureType
    {
	DVD_Physical   =  0,          // TODO: unimplemented, layer=layer#
	DVD_Copyright,                // TODO: unimplemented, layer=layer#
	DVD_DiscKey,                  // TODO: unimplemented,
	DVD_BurstCutArea,             // TODO: unimplemented,
	DVD_Manufacturer,             // layer=layer#
	DVD_CopyrightMgmt,            // TODO: unimplemented, address=lba
	DVD_MediaIdentifier,          // TODO: unimplemented,
	DVD_MediaKeyBlock,            // TODO: unimplemented, address=pack#
	DVD_DiscDefinitionStructure,  // TODO: unimplemented, [DVD-RAM]
	DVD_MediumStatus,             // TODO: unimplemented, [DVD-RAM]
	DVD_SpareAreaInformation,     // TODO: unimplemented,

	DVD_RecordMgmtDataLast = 0x0c,// TODO: unimplemented, address=start field#
	DVD_RecordMgmtData,           // TODO: unimplemented, address=start sector#
	DVD_PreRecordedLeadIn,        // TODO: unimplemented,
	DVD_UniqueDiscID,             // TODO: unimplemented,
	DVD_PhysicalFormatInfo,       // TODO: unimplemented, layer=layer#

	DVD_DiscControlBlocks = 0x30, // TODO: unimplemented, address=content descriptor
    };

    enum CommonStructureType
    {
	Disc_AACSVolumeID = 0x80,
	Disc_AACSMediaSN,
	Disc_AACSMediaID,
	Disc_AACSMediaKeyBlock,
	Disc_AACSDataKeys,
	Disc_LBAEncryptionExtents,
	Disc_CPRMMediaKeyBlock,
	Disc_MediaLayers = 0x90,
	Disc_WriteProtection = 0xC0,   // TODO: unimplemented,
	Disc_StructureLisc = 0xff
    };

private:
    enum MediaType
    {
	Media_DVD = 0,
	Media_HD_DVD = 0,
	Media_BD = 1
    };

private:
    DiscStructure*	data;
    
protected:
    DiscStructure*	    readStructure(uint32 type, uint32 lba, uint8 layer, MediaType mediatype, uint32 agid);
    bool		    sendStructure(uint32 type, MediaType mediatype, DiscStructure* str, uint32 agid);

public:
			    scsi_DiscStructure(class DriveIO &, DriveStatus&);
    virtual		    ~scsi_DiscStructure();
    virtual bool	    onInit();
    virtual bool	    onProbe(bool cu, uint32 s);

    template<class T>
    T*			    ReadDVDStructure(uint32 lba, uint8 layer)
    {
	cmdname	= "Read DVD Structure";
	return (T*)readStructure(T::Type(), lba, layer, Media_DVD, 0);
    }

    template<class T>
    T*			    ReadBDStructure(uint32 lba, uint8 layer)
    {
	cmdname	= "Read BD Structure";
	return (T*)readStructure(T::Type(), lba, layer, Media_BD, 0);
    }

    template<class T>
    bool		    SendDVDStructure(DiscStructure* ds)
    {
	cmdname = "Send DVD Structure";
	return sendStructure(T::Type(), Media_DVD, ds, 0);
    }

    template<class T>
    bool		    SendBDStructure(DiscStructure* ds)
    {
	cmdname = "Send BD Structure";
	return sendStructure(T::Type(), Media_BD, ds, 0);
    }
};


#endif
