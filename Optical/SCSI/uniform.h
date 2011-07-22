#ifndef __SCSI_UNIFORM_H
#define __SCSI_UNIFORM_H

#include <Generic/Types.h>

#define PACKED __attribute__((packed))

class aByte
{
   uint8       byte;
public:
   uint8       getField(int8 off, int8 len);
   aByte      &setField(int8 off, int8 len, uint8 data);
} PACKED;

class aWord
{
   uint16      word;
public:
               operator uint16();
   aWord      &operator =(uint16 data);
   uint16      getField(int8 off, int8 len);
   aWord      &setField(int8 off, int8 len, uint16 data);
} PACKED;

class aLong
{
   uint32      word;
public:
               operator uint32();
   aLong      &operator =(uint32 data);
   uint32      getField(int8 off, int8 len);
   aLong      &setField(int8 off, int8 len, uint32 data);
} PACKED;

#undef PACKED
#endif

