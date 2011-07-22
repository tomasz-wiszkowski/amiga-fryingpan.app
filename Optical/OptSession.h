#ifndef _OPTSESSION_H_
#define _OPTSESSION_H_

/*
 * this is for optical's internal use *only*
 */
#include "OptItem.h"

class OptDisc;

class OptSession : public OptItem
{
public:
                           OptSession(IOptItem *parent);

   virtual EItemType       getItemType() const;
   virtual void            setItemType(EItemType lNewType);
   virtual EDataType       getDataType() const;
   virtual void            setDataType(EDataType lNewType);
   virtual uint16          getPreGapSize() const;
   virtual void            setPreGapSize(uint16 lNewSize);
   virtual uint16          getPostGapSize() const;
   virtual void            setPostGapSize(uint16 lNewSize);
   virtual int32           getStartAddress() const;
   virtual void            setStartAddress(int32 lNewAddress);
   virtual int32           getEndAddress() const;
   virtual void            setEndAddress(int32 lNewAddress);
   virtual void            setDataBlockCount(int32 lNewSize);

   virtual int32           getDataBlockCount() const;

   virtual void            setProtected(bool bProtected);
   virtual void            setPreemphasized(bool bPreemph);

   virtual uint32          getDiscID() const;
   virtual void            setDiscID(uint32 id);
   
   virtual uint16          getFlags() const;
   virtual void            setFlags(uint16 lNewFlags);

   virtual IOptItem       *addChild();
};

#endif

