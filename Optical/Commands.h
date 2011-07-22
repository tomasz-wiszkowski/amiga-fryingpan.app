#ifndef __COMMANDS_H
#define __COMMANDS_H

#include "Various.h"

#include <Generic/String.h>
#include <Generic/DynList.h>
#include <Generic/Types.h>
#include "IOptItem.h"
#include "SCSI/uniform.h"
#include "SCSI/SCSICommand.h"

using namespace GenNS;


struct TrackInfo
{
private:

   aWord       length;
   uint8       track_number_lsb;
   uint8       session_number_lsb;

   struct _1 : public aLong
   {
      bool     isDamaged()    { return getField(21, 1); }
      bool     isCopy()       { return getField(20, 1); }
      uint8    getTrackMode() { return getField(16, 4); }
      bool     isReserved()   { return getField(15, 1); }
      bool     isBlank()      { return getField(14, 1); }
      bool     isPacket()     { return getField(13, 1); }
      bool     isFixedPacket(){ return getField(12, 1); }
      uint8    getDataMode()  { return getField(8,  4); }
      bool     isLRAValid()   { return getField(1,  1); }
      bool     isNWAValid()   { return getField(0,  1); }
   } flags;

   aLong       track_start;
   aLong       next_writable_address;
   aLong       free_blocks;
   aLong       packet_size;
   aLong       track_size;
   aLong       last_recorded_address;

   uint8       track_number_msb;
   uint8       session_number_msb;
   aWord       reserved;
   aLong       read_compatibility_lba;

   uint8       resvd[0];

public:

   int32      Length(void)
   {
      return length+2;
   };

   int32      GetTrackNumber(void)
   {
      if (Length() >= 34)
         return (track_number_msb<<8) | track_number_lsb;
      else
         return (track_number_lsb);
   };

   int32      GetSessionNumber(void)
   {
      if (Length() >= 34)
         return (session_number_msb<<8) | session_number_lsb;
      else
         return (session_number_lsb);
   };

   uint32    GetStartAddress(void)
   {
      return track_start;
   };

   uint32    GetEndAddress(void)
   {
      return track_start + track_size - 1;
   };

   uint32    GetActualSize(void)
   {
      if (IsBlank()) 
         return 0;
      if (!flags.isLRAValid()) 
         return GetTotalSize();
      return last_recorded_address - track_start;
   }

   uint32    GetTotalSize(void)
   {
      return track_size;
   };

   uint32    GetFreeBlocks()
   {
      return free_blocks;
   }

   int32      IsRecordable(void)
   {
      return flags.isNWAValid();
   };

   int32      IsDamaged(void)
   {
      return flags.isDamaged();
   };

   int32      IsIncremental(void)
   {
      if ((flags.getTrackMode() & ~2) == 5) 
         return 1;               // track partially/entirely recorded in incremental mode
      return flags.isPacket();   // track configured to be recorded in incremental mode
   };

   int32      IsReserved(void)
   {
      return flags.isReserved();
   };

   int32      IsProtected(void)
   {
      return Transform::CtlToProtection(flags.getTrackMode());
   }

   EDataType GetTrackType(void)
   {
      if (flags.isBlank()) 
         return Data_Unknown;

      switch (flags.getTrackMode() & ~3) {
         case 0:     
            return Data_Audio;
         case 4:     
           if       (flags.getDataMode() == 1)  
              return Data_Mode1;  // yep
           else if  (flags.getDataMode() == 2)  
              return Data_Mode2;  // alright
           else if  (flags.getDataMode() == 15) 
              return Data_Mode1;  // officially: UNKNOWN
           else                       
              return Data_Unknown;    // officially: UNSUPPORTED
         case 8:                      
           return Data_Audio;
         case 12:                     
           return Data_Unknown;
      };
      return Data_Unknown;
   };

   int32      IsPreemphasized(void)
   {
      return Transform::CtlToEmphasy(flags.getTrackMode());
   };

   int32      IsBlank(void)
   {
      return flags.isBlank();
   };

   int32      GetPacketSize(void)
   {
      return packet_size ? packet_size : 16;
   };

   int32      GetSectorSize(void)
   {
      if (IsBlank()) return 2048;                              // a small fix for faranheit; will display blanks properly
      return Transform::TrackTypeToSectorSize(GetTrackType());
   };

   void     FillInDiscItem(IOptItem*);
};

struct DiscInfo 
{
private:
   aWord       length;
   struct _1 : public aByte
   {
      bool  isErasable()            { return getField(4, 1); }
      uint8 getLastSessionStatus()  { return getField(2, 2); }
      uint8 getDiscStatus()         { return getField(0, 2); }
   } status __attribute__((packed));

   uint8       first_track_number;
   uint8       num_sessions_lsb;
   uint8       first_track_in_last_session_lsb;
   uint8       last_track_in_last_session_lsb;

   struct _2 : public aByte
   {
      bool isDiscIDValid()          { return getField(7, 1); }
      bool isDiscBarCodeValid()     { return getField(6, 1); }
      bool isDiscUsageUnrestricted(){ return getField(5, 1); }
   } flags __attribute__((packed));

   uint8       disc_type;
   uint8       num_sessions_msb;
   uint8       first_Track_in_last_session_msb;
   uint8       last_track_in_last_session_msb;

   aLong       disc_id_code;
   aLong       last_session_leadin_start_time;
   aLong       last_possible_leadout_start_time;
   aLong       disc_barcode[2];

   uint8       resvd3;
   uint8       num_opc_entries;

   struct OPCEntry {
      aWord  write_speed;
      aWord  opc_values[3];
   } opc[0];

   public:
   int32   Length(void)
   {
      return length+2;
   };

   bool    IsLastSessionIncomplete()
   {
      return status.getLastSessionStatus() == 1;
   }

   bool    IsLastSessionEmpty()
   {
      return status.getLastSessionStatus() == 0;
   }

   bool    IsLastSessionClosed()
   {
      return status.getLastSessionStatus() > 1;
   }

   bool    IsDiscEmpty()
   {
      return status.getDiscStatus() == 0;
   }

   bool    IsDiscIncomplete()
   {
      return status.getDiscStatus() == 1;
   }

   bool    IsDiscClosed()
   {
      return status.getDiscStatus() > 1;
   }

   int32   IsWritable(void)
   {
      if  (last_possible_leadout_start_time == 0xffffffff) return 0;
      return 1;
   }

   int32   IsErasable(void)
   {
      return status.isErasable();
   }
      
   int32   GetNumTracks(void)
   {
      return (last_track_in_last_session_msb << 8) | (last_track_in_last_session_lsb);
   }

   unsigned long GetLeadInStart()
   {
      return Transform::MsfToLba(last_session_leadin_start_time);
   }
   
   unsigned long GetLeadInLength()
   {
      if ((last_session_leadin_start_time != 0xffffff) &&
          (last_session_leadin_start_time != 0x000000))
      {
         if (last_session_leadin_start_time >= (80<<16))  // only if larger than 80 minutes
         {
            return 450000 - Transform::MsfToLba(last_session_leadin_start_time);
         }
         return Transform::MsfToLba(1, 0, 0);
      }
      return 0;
   }
};



class Page_Header
{ 
private:
   aWord    length;
   uint8    ccs_code;
   uint8    reserved[4];
   uint8    scsi;
   uint8    scsidata[0];

public:

   int32  TotalSize(void)
   {  
      return length+2;      
   };
  
   void*GetPage()
   {
      return &scsidata[scsi];
   }
};

class Page_ID
{
protected:
   struct _1 : public aByte
   {
      bool  isSavable()    { return getField(7, 1); }
      bool  isModified()   { return getField(6, 1); }
      void  setModified()  { setField(6, 1, 1);     }
      void  clrModified()  { setField(6, 1, 0);     }
      uint8 getPage()      { return getField(0, 6); }
   } header __attribute__((packed));
   uint8          page_length;

public:
   uint8          PageID()
   {
      return header.getPage();
   }

   int32          PageSize()
   {
      return page_length + 2;
   }

   void           SetModified()
   {
      header.setModified();
   }

   bool           IsModified()
   {
      return header.isModified();
   }

   void           ClearModified()
   {
      header.clrModified();
   }
};

struct Page_Capabilities : public Page_ID
{
private:
   struct _1 : protected aLong
   {
      bool doesReadDVDRam()      { return getField(29, 1); }
      bool doesReadDVDMinusR()   { return getField(28, 1); }
      bool doesReadDVDRom()      { return getField(27, 1); }
      bool doesReadMethod2()     { return getField(26, 1); }
      bool doesReadCDRW()        { return getField(25, 1); }
      bool doesReadCDR()         { return getField(24, 1); }
      bool doesWriteDVDRam()     { return getField(21, 1); }
      bool doesWriteDVDMinusR()  { return getField(20, 1); }
      bool doesWriteTestMode()   { return getField(18, 1); }
      bool doesWriteCDRW()       { return getField(17, 1); }
      bool doesWriteCDR()        { return getField(16, 1); }
      bool doesBurnProof()       { return getField(15, 1); }
      bool doesMultisession()    { return getField(14, 1); }
      bool doesMode2Form2()      { return getField(13, 1); }
      bool doesMode2Form1()      { return getField(12, 1); }
      bool hasDigitalPort2()     { return getField(11, 1); }
      bool hasDigitalPort1()     { return getField(10, 1); }
      bool hasCompositeOut()     { return getField(9, 1);  }
      bool doesPlayAudio()       { return getField(8, 1);  }
      bool doesReadBarCode()     { return getField(7, 1);  }
      bool doesReadUPC()         { return getField(6, 1);  }
      bool doesReadISRC()        { return getField(5, 1);  }
      bool hasC2Pointers()       { return getField(4, 1);  }
      bool doesRWCorrectio()     { return getField(3, 1);  }
      bool doesRWData()          { return getField(2, 1);  }
      bool isAudioAccurate()     { return getField(1, 1);  }
      bool isCDDASupported()     { return getField(0, 1);  }
   } rw;

   struct _2 : protected aWord
   {
      uint8 getMechanismType()   { return getField(13, 3); }
      bool  doesEject()          { return getField(11, 1); }
      bool  hasPreventionJumper(){ return getField(10, 1); }
      bool  doesLockState()      { return getField(9, 1);  }
      bool  doesLock()           { return getField(8, 1);  }
      bool  doesRWLeadIn()       { return getField(5, 1);  }
      bool  doesChangeSide()     { return getField(4, 1);  }
      bool  doesSoftSlotSelect() { return getField(3, 1);  }
      bool  doesDiscDetection()  { return getField(2, 1);  }
      bool  doesChannelMute()    { return getField(1, 1);  } // separate channel mute
      bool  doesChannelVolume()  { return getField(0, 1);  }
   } mechanism __attribute__((packed));

   aWord          max_read_speed;
   aWord          num_volume_levels;
   aWord          buffer_size;
   aWord          current_read_speed;

   struct _3 : protected aWord
   {
      uint8 getLength()          { return getField(4, 2); }
      bool  doesSupportLSBF()    { return getField(3, 1); }
      bool  doesSupportRCK()     { return getField(2, 1); }
      bool  doesSupportBCKF()    { return getField(1, 1); }
   } data __attribute__((packed));

   aWord          max_write_speed;
   aWord          current_write_speed_old;

   aWord          copy_management_revision;

   struct _4 : protected aLong
   {
      uint8 getSelRotationCtl()  { return getField(0, 2); }
   } rotation __attribute__((packed));

   aWord          current_write_speed;
   aWord          num_speed_performance_descriptors;
   aLong          speed_performance_descriptors[];

public:

   int32            GetSpeedDescriptorsCount();
   int32            GetSpeedDescriptor(int32 i);
   int32            GetMediaReadSupport();
   int32            GetMediaWriteSupport();
   uint32           GetCapabilities();
   int32            GetAudioFlags();
   int32            GetAudioVolumeLevels();
   uint16           GetCurrentWriteSpeed();
   uint16           GetCurrentWriteSpeedOld();
   uint16           GetCurrentReadSpeed();
   uint16           GetMaximumWriteSpeed();
   uint16           GetMaximumReadSpeed();
   uint16           GetBufferSize();
   DRT_Mechanism    GetMechanismType();
};

struct Page_Write : public Page_ID
{
private:
   struct _1 : protected aLong
   {
      bool  isBurnProof()              { return getField(30, 1);  }
      void  setBurnProof(bool f)       { setField(30, 1, f);      }
      bool  isLinkSizeValid()          { return getField(29, 1);  }
      void  setLinkSizeValid(bool f)   { setField(29, 1, f);      }
      bool  isTestMode()               { return getField(28, 1);  }
      void  setTestMode(bool f)        { setField(28, 1, f);      }
      uint8 getWriteType()             { return getField(24, 4);  }
      void  setWriteType(uint8 t)      { setField(24, 4, t);      }
      
      uint8 getMultisession()          { return getField(22, 2);  }
      void  setMultisession(uint8 t)   { setField(22, 2, t);      }
      bool  isFixedPacket()            { return getField(21, 1);  }
      void  setFixedPacket(bool f)     { setField(21, 1, f);      }
      bool  isCopy()                   { return getField(20, 1);  }
      void  setCopy(bool f)            { setField(20, 1, f);      }
      uint8 getTrackMode()             { return getField(16, 4);  }
      void  setTrackMode(uint8 t)      { setField(16, 4, t);      }

      uint8 getDataType()              { return getField(8, 4);   }
      void  setDataType(uint8 t)       { setField(8, 4, t);       }

      uint8 getLinkSize()              { return getField(0, 8);   }
      void  setLinkSize(uint8 t)       { setField(0, 8, t);       }
   } set1;

   struct _2 : protected aLong
   {
      uint8 getApplicationCode()       { return getField(16, 6);  }
      void  setApplicationCode(uint8 t){ setField(16, 6, t);      }
      uint8 getSessionFormat()         { return getField(8, 8);   }
      void  setSessionFormat(uint8 t)  { setField(8, 8, t);       }
   } set2;

   aLong          packet_size;
   aWord          audio_pause_length;
   int8           upc[16];
   int8           isrc[16];
   uint8          subheader[4];
   uint8          vendor_specific[4];

public:

   enum WriteType
   {
      WriteType_Packet,
      WriteType_TrackAtOnce,
      WriteType_SessionAtOnce,
      WriteType_Raw
   };

   enum TrackMode
   {
      TrackMode_Audio            = 0,
      TrackMode_Data_Sequential  = 4,
      TrackMode_Data_Incremental = 5
   };

   enum DataMode
   {
      DataMode_Raw                  = 0,     // 2352
      DataMode_Raw_PQ               = 1,     // 2368
      DataMode_Raw_PW               = 2,     // 2448
      DataMode_Raw_PW_Interleaved   = 3,     // 2448
      DataMode_Mode1                = 8,     // 2048
      DataMode_Mode2                = 9,     // 2336
      DataMode_Mode2Form1           = 10,    // 2048
      DataMode_Mode2Form1B          = 11,    // 2056
      DataMode_Mode2Form2           = 12,    // 2324
      DataMode_Mode2Form2B          = 13     // 2332
   };

   enum SessionFormat
   {
      Session_Data                  = 0,     // regular
      Session_Interactive           = 16,    // cd-i
      Session_ExtendedArchitecture  = 32
   };

   void           SetWriteType(WriteType t);
   WriteType      GetWriteType(void);
   void           SetPacketSize(uint32);
   uint32         GetPacketSize(void);
   DataMode       GetDataMode(void);
   void           SetDataMode(DataMode);
   TrackMode      GetTrackMode(void);
   void           SetTrackMode(TrackMode);
   void           SetLinkSize(uint8);
   SessionFormat  GetSessionFormat(void);
   void           SetSessionFormat(SessionFormat);
   void           SetTestMode(int32);
   int32          IsTestMode(void);
   void           SetMultisession(int32);
   int32          IsMultisession(void);
};

template <class T=class Page_ID>
class Page 
{
protected:
   Page_Header      *header;
   T                *page;
   bool              own;     // true = page is owned.

public:
   Page()
   {
      header   = 0;
      page     = 0;
      own      = true;
   }

   Page(Page_Header*p)
   {
      header   = 0;
      page     = 0;
      own      = true;
      *this = p;
   };

   Page<T> &operator =(Page_Header* p)
   {
      if (own && (header != 0))
         delete header;

      header = p;
      if (header != 0)
         page   = (T*)p->GetPage();
      else
         page   = 0;
      return *this;
   }

   ~Page()
   {
      if (own && (header != 0))
         delete header;
      header = 0;
   }

   operator Page_Header*()
   {
      return header;
   }

   T* operator ->()
   {
      return page;
   }

   operator T*()
   {
      return page;
   }

   bool IsValid()
   {
      return (header != 0) ? true : false;
   }

   void SetOwnPage(bool isown)
   {
      own = isown;
   }
};



struct TOC_Entry 
{
private:

   uint8    session;
   struct _1 : protected aByte
   {
      uint8 getAdr() { return getField(4, 4); }
      uint8 getCtl() { return getField(0, 4); }
   } adrctl __attribute__((packed));
   uint8    track;
   uint8    point;
   uint8    min;
   uint8    sec;
   uint8    frame;
   uint8    zero;
   uint8    pmin;
   uint8    psec;
   uint8    pframe;

public:

   int32      GetTrackSession(void)
      {  return session;                                                };

   int32      GetTrackNumber(void)
      {  return point;                                                  };

   int32      GetTrackPositionLBA(void)
      {  return Transform::MsfToLba(pmin, psec, pframe);                };

   EDataType GetTrackType(void)
      {  return Transform::CtlToTrackType(adrctl.getCtl());             };

   int32      IsProtected(void)
      {  return Transform::CtlToProtection(adrctl.getCtl());            };

   int32      IsPreemphasized(void)
      {  return Transform::CtlToEmphasy(adrctl.getCtl());               };

   int32      GetTrackSectorSize(void)
      {  return Transform::TrackTypeToSectorSize(GetTrackType());       };

   uint8    GetMin()
      {  return min;                                                    };

   uint8    GetSec()
      {  return sec;                                                    };
};

struct TOC_PrimitiveEntry
{
private:
   char           reserved;
   struct _1 : protected aByte
   {
      uint8 getAdr() { return getField(4, 4); }
      uint8 getCtl() { return getField(0, 4); }
   } adrctl __attribute__((packed));
   uint8          track;
   char           reserved2;
   aLong          position;   

public:
   int32      GetTrackSession(void)
      {  return 1;                                                      };

   uint8    GetTrackNumber(void)
      {  return track;                                                  };

   int32      GetTrackPositionLBA(void)
      {  return position;                                               };

   EDataType GetTrackType(void)
      {  return Transform::CtlToTrackType(adrctl.getCtl());             };

   int32      IsProtected(void)
      {  return Transform::CtlToProtection(adrctl.getCtl());            };

   int32      IsPreemphasized(void)
      {  return Transform::CtlToEmphasy(adrctl.getCtl());               };

   int32      GetTrackSectorSize(void)
      {  return Transform::TrackTypeToSectorSize(GetTrackType());       };

   uint8    GetMin()
      {  return (position >> 16) & 0xff;                                };

   uint8    GetSec()
      {  return (position >> 8) & 0xff;                                 };
};

struct TOC_FullTOC 
{
   private:

   aWord       length;
   uint8       first_track;
   uint8       last_track;
   TOC_Entry   elem[0];

   public:

   int32         GetNumTracks(void);
   int32         GetNumElements(void)
      {  return (length-2)/11;                                          };
   int32         GetDiscSize(void);
   TOC_Entry  *FindTOCEntry(int32);
   void        FillInDiscItem(IOptItem*, int32);
};

struct TOC_PrimitiveTOC 
{
   private:

   aWord                length;
   uint8                first_track;
   uint8                last_track;
   TOC_PrimitiveEntry   elem[0];

   public:

   int32                  GetNumTracks(void);
   int32                  GetNumElements(void)
      {  return (length-2)/8;                               };
   int32                  GetDiscSize(void)
      {  return FindTOCEntry(0xAA)->GetTrackPositionLBA();  };

   TOC_PrimitiveEntry  *FindTOCEntry(int32);
   void                 FillInDiscItem(IOptItem*, int32);
};

struct TOC_PMA 
{
};

struct TOC_ATIP
{
   private:

   aWord    length;
   aWord    resvd0;

   struct _1 : protected aLong
   {
      uint8 getWritingPower()       { return getField(28, 4); }
      uint8 getReferenceSpeed()     { return getField(24, 3); }
      bool  isDiscUseUnrestricted() { return getField(22, 1); }
      bool  isRewritable()          { return getField(14, 1); }
      uint8 getDiscSubType()        { return getField(11, 3); }
      bool  isA1Valid()             { return getField(10, 1); }
      bool  isA2Valid()             { return getField(9,  1); }
      bool  isA3Valid()             { return getField(8,  1); }
   } conf;

   uint8    atip_leadin_start_m;
   uint8    atip_leadin_start_s;
   uint8    atip_leadin_start_f;
   uint8    resvd4;

   uint8    atip_leadout_start_m;
   uint8    atip_leadout_start_s;
   uint8    atip_leadout_start_f;
   uint8    resvd5;

   uint8    a1[3];
   uint8    resvd6;

   uint8    a2[3];
   uint8    resvd7;

   uint8    a3[3];
   uint8    resvd8;

   uint8    s4[3];
   uint8    resvd9;

   public:

   uint32   Length(void)                  { return length+2;                                                                                 };
   uint32   GetDiscSubType(void)          { return conf.getDiscSubType();                                                                    };
   uint32   GetLeadInPos(void)            { return Transform::MsfToLba(atip_leadin_start_m, atip_leadin_start_s, atip_leadin_start_f);       };
   void           GetLeadInPos(int8& m, int8& s, int8& f)
   { 
      m = atip_leadin_start_m;
      s = atip_leadin_start_s;
      f = atip_leadin_start_f;
   };

   uint32   GetLeadOutPos(void)           { return Transform::MsfToLba(atip_leadout_start_m, atip_leadout_start_s, atip_leadout_start_f);    };
   uint8          AtipLeadInM()                 { return atip_leadin_start_m; };
   uint8          AtipLeadInS()                 { return atip_leadin_start_s; };
   uint8          AtipLeadInF()                 { return atip_leadin_start_f; };
};

struct TOC_CDTextEntry 
{
   uint8    block_id;
   uint8    id[2];
   uint8    charpos[1];
   uint8    cdtextinfo[12];
   aWord    crc;
};

struct TOC_CDText 
{
   private:
   aWord             length;
   aWord             resvd0;
   TOC_CDTextEntry   items[0];

   VectorT<String> *MergeBlocks(int32);

   public:

   int32               NumCDTextBlocks(void)
      {  return (length-2)/sizeof(TOC_CDTextEntry);         };


   VectorT<String>   *GetTitles(void);
   VectorT<String>   *GetPerformers(void);
   VectorT<String>   *GetSongWriters(void);
   VectorT<String>   *GetComposers(void);
   VectorT<String>   *GetArrangers(void);
   VectorT<String>   *GetMessages(void);
};

struct SUB_Header
{
private:
   unsigned char     pad;
   unsigned char     audiostatus;
   aWord             datalen;
public:
   int32 Length() 
   { 
      return datalen+4; 
   }
};

struct SUB_Position : public SUB_Header
{
private:
   uint8             code;
   struct _1 : protected aByte
   {
      uint8 getAdr() { return getField(4, 4); }
      uint8 getCtl() { return getField(0, 4); }
   } adrctl __attribute__((packed));
   uint8             track;
   uint8             index;
   aLong             absolute_address;
   aLong             relative_address;

public:
   int32 getTrack()
   {
      return track;
   }

   int32 getIndex()
   {
      return index;
   }
 
   int32 getAddress()
   {
      return absolute_address;
   }  
};

struct SUB_MCN : public SUB_Header
{
private:
   unsigned char     code;
   unsigned char     pad1[3];
   struct _1 : protected aByte
   {
      bool isValid() { return getField(7, 1); }
   } valid __attribute__((packed));
   unsigned char     mcn[13];
   unsigned char     zero;
   unsigned char     frame;

public:
   const unsigned char *getMCN() 
   {
      if (valid.isValid())
         return (unsigned char*)&mcn;
      return 0;
   };
};

struct SUB_ISRC : public SUB_Header
{
private:
   unsigned char     code;
   struct _1 : protected aByte
   {
      uint8 getAdr() { return getField(4, 4); }
      uint8 getCtl() { return getField(0, 4); }
   } adrctl __attribute__((packed));
   unsigned char     track;
   unsigned char     pad1;
   struct _2 : protected aByte
   {
      bool isValid() { return getField(7, 1); }
   } valid __attribute__((packed));
   unsigned char     country[2];
   unsigned char     owner[3];
   unsigned char     year[2];
   unsigned char     serial[5];
   unsigned char     zero;
   unsigned char     frame;
   unsigned char     pad3;

public:
   bool isValid()
   {
      return valid.isValid();
   }

   const unsigned char *getCountry()
   {
      if (isValid())
         return (unsigned char*)&country;
      return 0;
   }

   const unsigned char *getOwner()
   {
      if (isValid())
         return (unsigned char*)&owner;
      return 0;
   }

   const unsigned char *getYear()
   {
      if (isValid())
         return (unsigned char*)&year;
      return 0;
   }

   const unsigned char *getSerial()
   {
      if (isValid())
         return (unsigned char*)&serial;
      return 0;
   }

};

struct SUB_Q
{
protected:
   uint8       raw_block[2352];

   struct _1 : protected aByte
   {
      uint8 getAdr() { return getField(4, 4); }
      uint8 getCtl() { return getField(0, 4); }
   } adrctl __attribute__((packed));
   uint8       track;
   uint8       index;
   uint8       rel_m;
   uint8       rel_s;
   uint8       rel_f;
   uint8       zero;
   uint8       abs_m;
   uint8       abs_s;
   uint8       abs_f;
   aWord       crc;
   uint32      pad;

public:
   uint32      getRelPos()
   {
      return (rel_m << 16) | (rel_s << 8) | (rel_f);
   }

   uint32      getAbsPos()
   {
      return (abs_m << 16) | (abs_s << 8) | (abs_f);
   }

   uint8       getCtl()
   {
      return adrctl.getCtl();
   }

   uint8       getAdr()
   {
      return adrctl.getAdr();
   }

   uint8       getTrack()
   {
      return (((track & 0xf0)>>4) * 10) + (track & 0xf);
   }

   uint8       getIndex()
   {
      return (((index & 0xf0)>>4) * 10) + (index & 0xf);
   }
};


class cmd_TestUnitReady : public SCSICommand
{
   public:

                cmd_TestUnitReady(DriveIO &, DriveStatus&);
   virtual	~cmd_TestUnitReady() { };
   bool onInit();
};

class cmd_Reset: public SCSICommand
{
public:
    cmd_Reset(DriveIO &, DriveStatus&);
    bool onInit(void);
};

class cmd_Inquiry : public SCSICommand
{

   struct _inquiry
   {
      struct _1 : public aLong
      {
         uint8 getQualifier()          { return getField(13, 3); }
         uint8 getDeviceType()         { return getField(8, 5);  }
         bool  isRemovable()           { return getField(7, 1);  }
         bool  getVersion()            { return getField(8,  8); }
         bool  isAsyncEventReporting() { return getField(7,  1); }
         bool  isNormalACA()           { return getField(5,  1); }
         bool  isHierarchical()        { return getField(4,  1); }
         uint8 getRespDataFormat()     { return getField(0,  4); }
      } flags;

      struct _3 : public aLong
      {
         uint8 getAdditionalLength()   { return getField(24, 8); }
         bool  isStorageArray()        { return getField(23, 1); }
         bool  doesThirdPartyCommands(){ return getField(22, 1); }
         bool  doesBasicQueuing()      { return getField(15, 1); }
         bool  doesEnclosureServices() { return getField(14, 1); }
         bool  isVendorSpecific()      { return getField(13, 1); }
         bool  isMultiPort()           { return getField(12, 1); }
         bool  isMediumChanger()       { return getField(11, 1); }
         bool  doesRelativeAddressing(){ return getField(7,  1); }
         bool  isWideBus()             { return getField(5,  1); }
         bool  isSynchronous()         { return getField(4,  1); }
         bool  doesLinkedCommands()    { return getField(3,  1); }
         bool  doesCommandQueuing()    { return getField(1,  1); }
         bool  isVendorSpecific2()     { return getField(0,  1); }
      } features;

      char        vendor_id[8];
      char        product_id[16];
      char        product_version[4];
      char        firmware_version[20];
      char        reserved2[2];
      aWord       version_descriptors[8];
      aWord       reserved3[11];
      uint8       vendor_specific4[1];
   };

   _inquiry    *inquiry_data;
   char        *vendor_id;
   char        *product_id;
   char        *product_version;
   char        *firmware_version;

   public:

   enum {
      Supports_None  = 0,
      Supports_CCS   = 2,
      Supports_SPC1,
      Supports_SPC2,
      Supports_SPC3
   };

   enum {
      Type_Disk = 0,
      Type_Tape,
      Type_Printer,
      Type_Processor,
      Type_WriteOnce,
      Type_Optical,
      Type_Scanner,
      Type_OpticalMemory,
      Type_MediumChanger,
      Type_Communications,
      Type_GraphicArts,
      Type_GraphicArts2,
      Type_StorageArrat,
      Type_EnclosureServices,
      Type_SimpleDisk,
      Type_OpticalCard,
      Type_Reserved,
      Type_ObjectAccess
   };

               cmd_Inquiry(DriveIO &, DriveStatus&);
   virtual    ~cmd_Inquiry(void);

   bool onInit(void);
   void onExit(bool ok, uint32 err);

   char *ProductID(void)
      {  return product_id;                              };

   char *ProductVersion(void)
      {  return product_version;                         };

   char *VendorID(void)
      {  return vendor_id;                               };

   int32   DriveType(void)
      {  return inquiry_data->flags.getDeviceType();     };

   int32   SupportedStandard(void)
      {  return inquiry_data->flags.getVersion();         };

   char *FirmwareVersion(void)
      {  return firmware_version;                        };
};

class cmd_Mode : public SCSICommand
{
protected:
   Page_Header      *ms;
   int32             page;
   unsigned          direction:1;

public:
   enum 
   {
      Id_Page_Write        = 0x05,
      Id_Page_Capabilities = 0x2a
   };

public:
                        cmd_Mode(DriveIO &, DriveStatus&);
   virtual             ~cmd_Mode();
   Page_Header         *GetPage(int32);
   bool SetPage(Page_Header*);
   bool onInit();
   bool onProbe(bool, uint32);
};

class cmd_ReadDiscInfo : public SCSICommand
{
   public:


   private:

   DiscInfo            *dinfo;

   public:

                        cmd_ReadDiscInfo(DriveIO &, DriveStatus&);
   virtual             ~cmd_ReadDiscInfo(void);
   bool onInit();
   bool onProbe(bool, uint32);
   DiscInfo            *GetData(void)
      {  return dinfo;     }
   long                 GetLeadInLength();
};

class cmd_ReadTrackInfo : public SCSICommand
{
   private:

   TrackInfo        *tinfo;
   uint32             track_nr;

   public:

   enum
   {
      Track_Invisible = 0xffffffff
   };

                     cmd_ReadTrackInfo(DriveIO &, DriveStatus&);
   virtual          ~cmd_ReadTrackInfo(void);
   bool onInit();
   bool onProbe(bool, uint32);

   TrackInfo        *GetData(void)
      {  return tinfo;              };
   void              SelectTrack(uint32 x)
      {  track_nr = x;              };
};

class cmd_ReadTOC : public SCSICommand
{
   private:


   struct _toc_resp {
      private:
      aWord       length;
      aWord       pad[1];

      public:
      int32 Length(void)     { return length+2; };
   };


   _toc_resp        *toc;

   uint8             type;
   bool              wantmsf;

   public:

   enum {
      Type_TOC       =  0,
      Type_FullTOC   =  2,
      Type_PMA       =  3,
      Type_ATIP      =  4,
      Type_CDText    =  5
   };

                                          cmd_ReadTOC(DriveIO &, DriveStatus&);
   virtual                               ~cmd_ReadTOC(void);
   bool onInit();
   bool onProbe(bool, uint32);

   void                                   SelectType(uint32 x)
      {  type = x;                                             };

   TOC_FullTOC                           *GetFullTOC(bool msf=true)
   {	    
       type = Type_FullTOC; 
       wantmsf = msf; 
       if (Go())	
	   return (TOC_FullTOC*)toc; 
       return 0;    
   };

   TOC_PrimitiveTOC                      *GetTOC(bool msf=true)
   {  
       type = Type_TOC; 
       wantmsf = msf; 
       if (Go()) 
	   return (TOC_PrimitiveTOC*)toc;
       return 0;
   };

   TOC_PMA                               *GetPMA()
   {  
       type = Type_PMA; 
       wantmsf = false; 
       if (Go()) 
	   return (TOC_PMA*)toc;           
       return 0;
   };

   TOC_ATIP                              *GetATIP(void)
   {  
       type = Type_ATIP; 
       wantmsf = false; 
       if (Go()) 
	   return (TOC_ATIP*)toc;         
       return 0;
   };

   TOC_CDText                            *GetCDText(void)
   {  
       type = Type_CDText; 
       wantmsf = false; 
       if (Go()) 
	   return (TOC_CDText*)toc;     
       return 0;
   };

};

class cmd_ReadSubChannel : public SCSICommand
{
   SUB_Header       *hdr;
   int32               size;
   int32               type; 
   int32               track;
public:

   enum {
      Type_Position  =  1,
      Type_MCN       =  2,
      Type_ISRC      =  3
   };

                     cmd_ReadSubChannel(DriveIO &, DriveStatus&);
   bool onInit();
   bool onProbe(bool, uint32);
   SUB_Position     *getPosition();
   SUB_MCN          *getMCN();
   SUB_ISRC         *getISRC(int32 track);   
   
};

class cmd_Seek : public SCSICommand
{
   uint32            sector;
public:
                     cmd_Seek(DriveIO &, DriveStatus&);
   bool onInit();
   bool                seek(uint32 sector);
};

class cmd_Play : public SCSICommand
{
   int32             start;
   int32             end;
public:
                     cmd_Play(DriveIO &, DriveStatus&);
   bool onInit();
   bool                play(int32 start, int32 end);
};

class cmd_Blank : public SCSICommand
{
   int32      type;
   uint32   num;
   bool     immed;

   public:

   enum BlankType {
      Blank_All = 0,
      Blank_Minimal,
      Blank_Track,
      Blank_UnreserveTrack,
      Blank_TrackTail,
      Blank_UncloseSession,
      Blank_Session
   };


                  cmd_Blank(DriveIO &, DriveStatus&);
   virtual       ~cmd_Blank() { };
   bool onInit();
   void           setType(cmd_Blank::BlankType t, uint32 num);
   void           setImmediate(bool f);
};

class cmd_StartStopUnit : public SCSICommand
{
   public:

   enum StartStopType {
      StartStop_Stop    = 0,
      StartStop_Start,
      StartStop_Eject,
      StartStop_Load,
      StartStop_Idle    = 0x20,
      StartStop_Standby = 0x30,
      StartStop_Sleep   = 0x50
   };

          cmd_StartStopUnit(DriveIO &, DriveStatus&);
   virtual    ~cmd_StartStopUnit();
   bool onInit();
   void        setType(StartStopType t);
};

class cmd_LockDrive : public SCSICommand
{
   public:
               cmd_LockDrive(DriveIO &, DriveStatus&);
   virtual    ~cmd_LockDrive();
   bool onInit();
   void        setLocked(bool locked);
};

class cmd_ReadHeader : public SCSICommand
{
   public:


   private:

   struct _rhdata {
      uint8    type;
      uint8    resvd[3];
      aLong    lba;
   };

   struct _rhdata *d;
   uint32           lba;

   public:

         cmd_ReadHeader(DriveIO &, DriveStatus&);
   virtual             ~cmd_ReadHeader(void);
   bool onInit();
   EDataType            GetTrackType(uint32 lba);

};

class cmd_Read : public SCSICommand
{
   public:


   private:

   APTR        data;
   uint32       firstsector;
   uint16       numsectors;

   public:

         cmd_Read(DriveIO &, DriveStatus&);
   virtual             ~cmd_Read(void);
   bool onInit();
   bool ReadData(uint32, uint32, APTR);
};

class cmd_ReadCD : public SCSICommand
{
public:

   enum Flags
   {
      Flg_Sync       = 1,     // synchronization (12 bytes)
      Flg_4BHeader   = 2,     // 4byte header (4 bytes)
      Flg_8BHeader   = 4,     // 8byte header (8 bytes)
      Flg_12BHeader  = 8,     // 12byte header (12 bytes)
      Flg_UserData   = 16,    // user data (2048-2352 bytes)
      Flg_ECC        = 32,    // ECC / EDC data ,
      Flg_C2Error    = 64,    // C2 Errors (294 bytes)
      Flg_C2Block    = 128,   // C2 Errors + Block Error (296 bytes)
      Flg_SubRawPW   = 256,   // P-W RAW subchannel (96 bytes)
      Flg_SubQ       = 512,   // Formatted Q subchannel (16 bytes)
      Flg_SubRW      = 1024,  // Formatted R-W subchannel (96 bytes)

      Flg_AllSubQ    = Flg_Sync | Flg_12BHeader | Flg_UserData | Flg_ECC | Flg_SubQ,
      Flg_AllRawPW   = Flg_Sync | Flg_12BHeader | Flg_UserData | Flg_ECC | Flg_SubRawPW,
      Flg_AllRawRW   = Flg_Sync | Flg_12BHeader | Flg_UserData | Flg_ECC | Flg_SubRW,
   };

private:
   void    *data;
   uint32   firstsector;
   uint16   numsectors;
   uint16   secSize;
   uint8    dataspec;
   uint8    subchspec;   

public:

                        cmd_ReadCD(DriveIO &, DriveStatus&);
   virtual             ~cmd_ReadCD(void);
   bool onInit();
   bool readCD(int32 start, uint16 count, uint16 secsize, void* mem, uint32 flags);
};

class cmd_Write : public SCSICommand
{
   public:


   private:

   APTR        data;
   uint32       firstsector;
   uint16       numsectors;
   uint16       sectorsize;

   public:

         cmd_Write(DriveIO &, DriveStatus&);
   virtual             ~cmd_Write(void);
   bool onInit();
   bool WriteData(uint32, uint32, uint32, APTR);
};

class cmd_Close : public SCSICommand
{
   int32   type;
   uint32 num;
   int32   immed;

   public:

   enum CloseType {
      Close_CDR_Track                     = 1,
      Close_CDR_LastSession               = 2,     // nominal close

      Close_DVDMinusR_Track               = 1,
      Close_DVDMinusR_LastSession         = 2,
      Close_DVDMinusR_SpecialCase         = 3,     // for restricted overwrite disc

      Close_DVDPlusR_Track                = 1,
      Close_DVDPlusR_LastSession          = 2,
      Close_DVDPlusR_FinalizeCompatible   = 5,     // the 30mm radius on layer 0
      Close_DVDPlusR_FinalizeNominal      = 6,     // finalize in nominal way

      Close_DVDPlusRDL_Track              = 1,
      Close_DVDPlusRDL_LastSession        = 2,
      Close_DVDPlusRDL_FinalizeExtended   = 4,     // the 30mm radius on both layers
      Close_DVDPlusRDL_FinalizeCompatible = 5,     // the 30mm radius on layer 0
      Close_DVDPlusRDL_FinalizeNominal    = 6,     // finalize in nominal way

      Close_DVDPlusRW_DeIcing             = 0,
      Close_DVDPlusRW_FinalizeCompatible  = 2,
      Close_DVDPlusRW_FinalizeNominal     = 3,

      Close_CDMRW_StopFormat              = 2,
      Close_CDMRW_Finalize                = 6,

      /*
      ** the following are only for discs recorded in sequential recording
      */
      Close_BDR_Track			  = 1,
      Close_BDR_Session			  = 2,
      Close_BDR_Finalize		  = 6,

      Close_FlushBuffers                  = -1
   };


        cmd_Close(DriveIO &, DriveStatus&);
   virtual       ~cmd_Close() { };
   bool onInit();
   void           setType(cmd_Close::CloseType t, uint32 num);
   void           setImmediate(int32 f);
};

class cmd_Reserve : public SCSICommand
{
   public:


   private:

   uint32       numsectors;


   public:

         cmd_Reserve(DriveIO &, DriveStatus&);
   virtual             ~cmd_Reserve(void);
   bool onInit();
   bool                   Reserve(uint32);
};


class cmd_SetSpeed : public SCSICommand
{
   public:

   private:

   uint16       read;
   uint16       write;

   public:
         cmd_SetSpeed(DriveIO &, DriveStatus&);
   virtual             ~cmd_SetSpeed(void);
   bool onInit();
   bool SetSpeed(uint16, uint16);
};

class cmd_SendCueSheet : public SCSICommand
{
   unsigned char     *pCue;
   unsigned short     lElements;
public:
                        cmd_SendCueSheet(DriveIO &, DriveStatus&);
   virtual             ~cmd_SendCueSheet(void);
   bool onInit();
   void                 SendCueSheet(unsigned char *pCue, int32 lElements);
};

class cmd_Calibrate : public SCSICommand
{
public:

                        cmd_Calibrate(DriveIO &, DriveStatus&);
   bool onInit();
};


#endif

