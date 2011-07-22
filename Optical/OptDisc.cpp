#include "OptDisc.h"

OptDisc::OptDisc() 
   : OptSession(0)
{
}

EItemType       OptDisc::getItemType() const
{
   return Item_Disc;
}

uint32          OptDisc::getDiscID() const
{
   return OptItem::getDiscID();
}

void            OptDisc::setDiscID(uint32 newid)
{
   OptItem::setDiscID(newid);
}

uint16          OptDisc::getFlags() const
{
   return OptItem::getFlags();
}

void            OptDisc::setFlags(uint16 lNewFlags)
{
   OptItem::setFlags(lNewFlags);
}

IOptItem       *OptDisc::addChild()
{
   OptSession *sess = new OptSession(this);
   return OptItem::addChild(sess);
}

