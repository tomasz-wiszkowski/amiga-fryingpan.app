#include "OptSession.h"
#include "OptTrack.h"
#include "OptIndex.h"

OptTrack::OptTrack(IOptItem* parent)
   : OptItem(parent)
{
}

EItemType       OptTrack::getItemType() const
{
   return Item_Track;
}

void            OptTrack::setItemType(EItemType lNewType)
{
   ASSERT(false);
}

uint32          OptTrack::getDiscID() const
{
   return 0;
}

void            OptTrack::setDiscID(uint32 id)
{
   ASSERT(false);
}

IOptItem       *OptTrack::addChild()
{
   IOptItem *idx = new OptIndex(this);

   return OptItem::addChild(idx);
}

