#ifndef _OPTICAL_CDTEXT_H_
#define _OPTICAL_CDTEXT_H_

#include <Generic/LibrarySpool.h>
#include <Generic/VectorT.h>
#include "IOptItem.h"
#include <Generic/SumCRC.h>

using namespace GenNS;

class CDText 
{
   /* enums go here */

protected:
   enum Block
   {
      Blk_Unknown    =  0x00,
      Blk_Title      =  0x80,
      Blk_Performer,
      Blk_SongWriter,
      Blk_Composer,
      Blk_Arranger,
      Blk_Message,
      Blk_DiscID,
      Blk_Genre,
      Blk_TOC,
      Blk_TOC2,
      Blk_Resvd1,
      Blk_Resvd2,
      Blk_Resvd3,
      Blk_ClosedInfo,
      Blk_ISRC,
      Blk_Size
   };

public:
   enum Genre
   {
      Gen_Unused     =  0x00,
      Gen_Undefined,
      Gen_Adult,
      Gen_AlternativeRock,
      Gen_Children,
      Gen_Classic,
      Gen_Christian,
      Gen_Country,
      Gen_Dance,
      Gen_EasyListening,
      Gen_Erotic,
      Gen_Folk,
      Gen_Gospel,
      Gen_HipHop,
      Gen_Jazz,
      Gen_Latin,
      Gen_Musical,
      Gen_NewAge,
      Gen_Opera,
      Gen_Operetta,
      Gen_Pop,
      Gen_Rap,
      Gen_Reggae,
      Gen_Rock,
      Gen_RnB,
      Gen_SoundFX,
      Gen_SpokenWorld,
      Gen_WorldMusic,
      Gen_Reserved,
      Gen_RIAA          = 32768 // to 65535
   };

   enum CharCode
   {
      Chr_ISO8859_1     =  0x00,
      Chr_ASCII,
      Chr_Reserved02,
      Chr_Kanji         =  0x80,
      Chr_Korean,
      Chr_Chinese,
      Chr_Reserved83
   };

   enum LanguageCode
   {
      Lng_Czech      =  6,
      Lng_Danish,
      Lng_German,
      Lng_English,
      Lng_Spanish,
      Lng_French     =  15,
      Lng_Italian    =  21,
      Lng_Hungarian  =  27,
      Lng_Dutch      =  29,
      Lng_Norwegian  =  30,
      Lng_Polish     =  32,
      Lng_Portuguese =  33,
      Lng_Slovene    =  38,
      Lng_Finnish    =  39,
      Lng_Swedish    =  40,
      Lng_Russian    =  86,
      Lng_Korean     =  101,
      Lng_Japanese   =  105,
      Lng_Greek      =  112,
      Lng_Chinese    =  117,
   };

   /* structs - just fyi */
protected:
   struct SizeInfo
   {
      uint8          charCode;
      uint8          firstTrack;
      uint8          lastTrack;
      uint8          copyright;     // we want 0 here, 3 means 'copyright protected'
      uint8          packsCount[16];// for tags 0x80 - 0x8f: how many packs are there?
      uint8          lastSeqNum[8]; // for 8 different sets - we can specify the last sequence number (last pack #). basically only [0] is what we fill.
      uint8          languages[8];  // for 8 different sets, again we want to fill only [0].
   };

   struct Pack
   {
      uint8    block;         // block id (enum Block)
      uint8    blockTrack;    // track # or pack # in current block
      uint8    totalPack;     // pack # in the whole set
      uint8    charPos;       // current char pos, 0 based.
      uint8    data[12];      // data
      uint8    crc16[2];      // crc16 calced using CCITT mthd
   };

protected:
   VectorT<Pack>        packs;
   bool                 cdText;
   SizeInfo             size;
   SumCRC               crc;

   Block                currentBlock;
   Pack                 currentPack;
   uint8                currentPackNo;
   uint8                currentOffset;
   uint8                currentTrack;
   uint8                currentByte;
protected:
   void                 addBlockData(Block, uint8 *data, int16 size);
   void                 closePack();
public:
                        CDText(const IOptItem*);
                       ~CDText();
   void                 printReport(DbgHandler* dbg);
   bool                 hasCDText();
   bool                 getDataBuffer(uint8*&, int32&);
};

#endif
