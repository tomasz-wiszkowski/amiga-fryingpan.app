#ifndef __SCSI_GETCONFIGURATION_H
#define __SCSI_GETCONFIGURATION_H

#include "SCSICommand.h"
#include "uniform.h"

enum Feat_ID
{
    Feature_ProfileList           =  0,
    Feature_Core                  =  1,
    Feature_Morphing              =  2,
    Feature_RemovableMedium       =  3,
    Feature_WriteProtect          =  4,

    Feature_RandomReadable        =  0x10,
    Feature_MultiRead             =  0x1d,
    Feature_CDRead                =  0x1e,
    Feature_DVDRead               =  0x1f,

    Feature_RandomWritable        =  0x20,
    Feature_StreamWritable        =  0x21,
    Feature_SectorErasable        =  0x22,
    Feature_Formattable            =  0x23,
    Feature_HWDefectManagement    =  0x24,
    Feature_WriteOnce             =  0x25,
    Feature_RestrictedOverwrite   =  0x26,
    Feature_CD_RW_CAV             =  0x27,
    Feature_CD_MRW                =  0x28,
    Feature_DefectReporting       =  0x29,
    Feature_DVD_PlusRW            =  0x2a,
    Feature_DVD_PlusR             =  0x2b,
    Feature_RigidOverwrite        =  0x2c,
    Feature_CD_TrackAtOnce        =  0x2d,
    Feature_CD_SessionAtOnce      =  0x2e,
    Feature_DVD_MinusR_RW_Write   =  0x2f,

    Feature_DD_CD_Read            =  0x30,
    Feature_DD_CD_R_Write         =  0x31,
    Feature_DD_CD_RW_Write        =  0x32,
    Feature_CD_RW_MediaWriteSupp  =  0x37,    // supported media types
    Feature_BD_R_PseudoOverwrite  =  0x38,
    Feature_DVD_PlusRW_DualLayer  =  0x3a,
    Feature_DVD_PlusR_DualLayer   =  0x3b,
    Feature_BD_Read               =  0x40,
    Feature_BD_Write              =  0x41,

    Feature_PowerManagement       =  0x100,
    Feature_SMART                 =  0x101,
    Feature_Changer               =  0x102,
    Feature_ExternalAudioPlay     =  0x103,
    Feature_MicrocodeUpgradable   =  0x104,
    Feature_Timeout               =  0x105,
    Feature_DVD_CSS               =  0x106,
    Feature_RealTimeStreaming     =  0x107,
    Feature_DriveSerialNumber     =  0x108,
    Feature_MediaSerialNumber     =  0x109,   // SPC3 - Command AB/01 - ServiceAction!!!
    Feature_DiscControlBlocks     =  0x10a,
    Feature_DVD_CPRM              =  0x10b,
    Feature_FirmwareInformation   =  0x10c
};

struct Feature
{
private:
    aWord    id;
    struct _1 : protected aByte
    {
	uint8 getVersion()   { return getField(2, 4); }
	bool  isPersistent() { return getField(1, 1); }
	bool  isCurrent()    { return getField(0, 1); }  
    } type __attribute__((packed));
    uint8    length;

public:
    int32    IsPersistent(void)   {  return type.isPersistent();                  };
    int32    IsCurrent(void)      {  return type.isCurrent();                     };
    int32    GetVersion(void)     {  return type.getVersion();                    };
    Feat_ID  GetId()              {  return (Feat_ID)(uint16)id;                  };
    Feature* Next()               {  return (Feature*)(((uint32)this)+length+4);  };
    int32    GetLength()          {  return length+4;                             };
    void*    GetBody()            {  return &((uint8*)this)[4];                   };
    int32    GetBodyLength()      {  return length;                               };
};

struct Feat_Formattable : public Feature
{
    struct _1 : protected aLong
    {
	bool BDRENoSpareAlloc() { return getField(27, 1); }
	bool Expand() { return getField(26, 1); }
	bool QuickCert() { return getField(25, 1); }
	bool FullCert() { return getField(24, 1); }
	bool FastReFormat() { return getField(23, 1); }
    } support1 __attribute__((packed));

    struct _2 : protected aLong
    {
	bool RandomRecordingMode() { return getField(24, 1); }
    } support2 __attribute__((packed));


    static Feat_ID Type()
    {
	return Feature_Formattable;
    }
};

struct Feat_ProfileList : public Feature
{
    struct 
    {
	aWord    profile;
	struct _1 : protected aWord
	{
	    bool isCurrent() { return getField(8, 1); }
	} conf __attribute__((packed));
    } items[0];

    static Feat_ID Type()
    {
	return Feature_ProfileList;
    }
};

struct Feat_DVD_PlusR : public Feature
{
    struct _1 : protected aLong
    {
	bool isWritable() { return getField(24, 1); }
    } conf __attribute__((packed));

    static Feat_ID Type()
    {
	return Feature_DVD_PlusR;
    }
};

struct Feat_DVD_PlusRW : public Feature
{
    struct _1 : public aLong
    {
	bool doesWrite()	{ return getField(24, 1); }
	bool doesQuickStart()	{ return getField(17, 1); }
	bool doesCloseOnly()	{ return getField(16, 1); }
    } conf __attribute__((packed));

    static Feat_ID Type()
    {
	return Feature_DVD_PlusRW;
    }
};

struct Feat_DVD_MinusR_RW_Write : public Feature
{
    struct _1 : protected aLong
    {
	bool doesBurnProof() { return getField(30, 1); }
	bool doesTestWrite() { return getField(26, 1); }
	bool isReWritable()  { return getField(25, 1); }
    } conf __attribute__((packed));

    static Feat_ID Type()
    {
	return Feature_DVD_MinusR_RW_Write;
    };
};

struct Feat_CD_TrackAtOnce : public Feature
{
    struct _1 : protected aLong
    {
	bool doesBurnProof() { return getField(30, 1); }
	bool doesRWRawSubcode() { return getField(28, 1); }
	bool doesRWPackSubcode() { return getField(27, 1); }
	bool doesTestWrite() { return getField(26, 1); }
	bool isReWritable()  { return getField(25, 1); }
	bool doesRWSubcode() { return getField(24, 1); }
    } conf __attribute__((packed));

    static Feat_ID Type()
    {
	return Feature_CD_TrackAtOnce;
    }
};

struct Feat_CD_SessionAtOnce: public Feature
{
    struct _1 : protected aLong
    {
	bool doesBurnProof() { return getField(30, 1); }
	bool doesSessionAtOnce() { return getField(29, 1); }
	bool doesRawMultisession() { return getField(28, 1); }
	bool doesRawRecording() { return getField(27, 1); }
	bool doesTestWrite() { return getField(26, 1); }
	bool isReWritable()  { return getField(25, 1); }
	bool doesRWSubcode() { return getField(24, 1); }
	uint32 maxCueSheetLen() { return getField(0, 24); }
    } conf __attribute__((packed));

    static Feat_ID Type()
    {
	return Feature_CD_SessionAtOnce;
    }
};

struct Feat_DiscControlBlocks : public Feature
{
    uint32         features[0];

    static Feat_ID Type()
    {
	return Feature_DiscControlBlocks;
    }
};

struct Feat_DVD_PlusR_DualLayer : public Feature
{
    struct _1 : public aLong
    {
	bool doesWriteDualLayer() { return getField(24, 1); }
    } conf __attribute__((packed));
    static Feat_ID Type()
    {
	return Feature_DVD_PlusR_DualLayer;
    }
};

struct Feat_DVD_PlusRW_DualLayer : public Feature
{
    struct _1 : public aLong
    {
	bool doesWriteDualLayer() { return getField(24, 1); }
	bool doesQuickStart()	  { return getField(17, 1); }
	bool doesCloseOnly()	  { return getField(16, 1); }
    } conf __attribute__((packed));
    static Feat_ID Type()
    {
	return Feature_DVD_PlusRW_DualLayer;
    }
};

struct Feat_RigidOverwrite : public Feature
{
    struct _1 : public aLong
    {
	bool doesDfctStatDataGenerate()	{ return getField(27, 1); }
	bool doesDfctStatDataRead()	{ return getField(26, 1); }
	bool doesQuickFmtIntermediate()	{ return getField(25, 1); }
	bool doesBlank()		{ return getField(24, 1); }
    };

    static Feat_ID Type()
    {
	return Feature_RigidOverwrite;
    }
};

struct Feat_RestrictedOverwrite : public Feature
{
    static Feat_ID Type()
    {
	return Feature_RestrictedOverwrite;
    }
};

struct Feat_CD_MRW : public Feature
{
    struct _1 : public aLong
    {
	bool doesWrite() { return getField(24, 1); }
    } conf __attribute__((packed));

    static Feat_ID Type()
    {
	return Feature_CD_MRW;
    }
};

struct Feat_CD_RW_MediaWriteSupp : public Feature
{
    struct _1 : public aLong
    {
	bool doesSubtype7() { return getField(23, 1); }
	bool doesSubtype6() { return getField(22, 1); }
	bool doesSubtype5() { return getField(21, 1); }
	bool doesSubtype4() { return getField(20, 1); }
	bool doesSubtype3() { return getField(19, 1); }
	bool doesSubtype2() { return getField(18, 1); }
	bool doesSubtype1() { return getField(17, 1); }
	bool doesSubtype0() { return getField(16, 1); }
    } conf __attribute__((packed));

    static Feat_ID Type()
    {
	return Feature_CD_RW_MediaWriteSupp;
    }
};

struct Feat_WriteProtect : public Feature
{
    struct _1 : public aLong
    {
	bool doesDWP() { return getField(27, 1); }
	bool doesWDCB() { return getField(26, 1); }
	bool doesPWP() { return getField(25, 1); }
	bool doesSWPP() { return getField(24, 1); }
    } conf __attribute__((packed));

    static Feat_ID Type()
    {
	return Feature_WriteProtect;
    }
};

struct Feat_BD_Write : public Feature
{
    struct _1 : public aLong
    {
	bool supportsVerifyNotRequired() { return getField(24, 1); }
    } conf __attribute__((packed));

    struct _2 : public aLong
    {
	bool supportsBDREv2() { return getField(26, 1); }
    } bdre __attribute__((packed));
    
    uint32 resvd0;

    struct _3 : public aLong
    {
	bool supportsBDR() { return getField(25, 1); }
    } bdr __attribute__((packed));

    uint32 resvd1;

    static Feat_ID Type()
    {
	return Feature_BD_Write;
    }
};

struct Feat_RandomWritable : public Feature
{
    aLong LastLBA;
    aLong LBASize;
    aWord Blocking;
    aWord resvd0;

    static Feat_ID Type()
    {
	return Feature_RandomWritable;
    }
};

struct Feat_BD_R_PseudoOverwrite : public Feature
{
    uint32 resvd;

    static Feat_ID Type()
    {
	return Feature_BD_R_PseudoOverwrite;
    }
};

struct Feat_WriteOnce : public Feature
{
    aLong logBlockSize;
    aWord blocking;
    struct _1 : public aWord
    {
	bool pagePresent() { return getField(8, 1); }
    } rwRecovery __attribute__((packed));

    static Feat_ID Type()
    {
	return Feature_WriteOnce;
    }
};


class scsi_GetConfiguration : public SCSICommand
{

    struct _feature
    {
	aLong    length;
	aWord    resvd;
	aWord    current_profile;
	uint8    profiles[0];
    };


    _feature   *feature_data;
    uint32       read_media;
    uint32       write_media;
protected:
    Feature *getFeature(Feat_ID id);
public:

    scsi_GetConfiguration(DriveIO &, DriveStatus&);
    virtual ~scsi_GetConfiguration();

    bool onInit();
    bool onProbe(bool, uint32);

    int32   GetCurrentProfile();
    uint32  GetMediaReadSupport();
    uint32  GetMediaWriteSupport();
    uint32  GetDriveCapabilities();
    Feature* GetNextFeature(Feature *last);

    template<class T>
    T*	    GetFeature()
    {
	return (T*)getFeature(T::Type());
    }

};


#endif

