#include "OptDisc.h"
#include "OptTrack.h"

OptSession::OptSession(IOptItem *parent)
   : OptItem(parent)
{
}

EItemType       OptSession::getItemType() const
{
   return Item_Session;
}

void            OptSession::setItemType(EItemType lNewType)
{
   ASSERT(false);
}

EDataType       OptSession::getDataType() const
{
   return Data_Unknown;
}

void            OptSession::setDataType(EDataType lNewType)
{
   ASSERT(false);
}

uint16          OptSession::getPreGapSize() const
{
   return 0;
}

void            OptSession::setPreGapSize(uint16 lNewSize)
{
   ASSERT(false);
}

uint16          OptSession::getPostGapSize() const
{
   return 0;
}

void            OptSession::setPostGapSize(uint16 lNewSize)
{
   ASSERT(false);
}

int32           OptSession::getStartAddress() const
{
   if (getChildCount() == 0)
      return 0;

   return getChild(0)->getStartAddress();
}

void            OptSession::setStartAddress(int32 lNewAddress)
{
   ASSERT(false);
}

int32           OptSession::getEndAddress() const
{
   if (getChildCount() == 0)
      return 0;

   return getChild(-1)->getEndAddress();
}

void            OptSession::setEndAddress(int32 lNewAddress)
{
   ASSERT(false);
}

int32           OptSession::getDataBlockCount() const
{
   int32 db=0;

   for (int i=0; i<getChildCount(); i++)
   {
      db += getChild(i)->getDataBlockCount();
   }

   return db;
}

void            OptSession::setDataBlockCount(int32 lNewSize)
{
   ASSERT(false);
}

void            OptSession::setProtected(bool bProtected)
{
   ASSERT(false);
}

void            OptSession::setPreemphasized(bool bPreemph)
{
   ASSERT(false);
}

uint32          OptSession::getDiscID() const
{
   return 0;
}

void            OptSession::setDiscID(uint32 id) 
{
   ASSERT(false);
}

uint16          OptSession::getFlags() const
{
   return OptItem::getFlags();
}

void            OptSession::setFlags(uint16 lNewFlags)
{
   OptItem::setFlags(lNewFlags);
}

IOptItem       *OptSession::addChild()
{
   OptTrack *trak = new OptTrack(this);
   return OptItem::addChild(trak);
}

