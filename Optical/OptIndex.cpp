#include "OptIndex.h"

OptIndex::OptIndex(IOptItem* parent)
   : OptItem(parent)
{
   setComplete(true);
}

EItemType       OptIndex::getItemType() const
{
   return Item_Index;
}

void            OptIndex::setItemType(EItemType lNewType)
{
   ASSERT(false);
}

uint32          OptIndex::getDiscID() const
{
   return 0;
}

void            OptIndex::setDiscID(uint32 id)
{
   ASSERT(false);
}

IOptItem       *OptIndex::addChild()
{
   ASSERTS(false, "Indices may not have children");
   return 0;
}

EDataType       OptIndex::getDataType() const
{
   return getParent()->getDataType();
}

void            OptIndex::setDataType(EDataType lNewType)
{
   ASSERT(false);
}

bool            OptIndex::isProtected() const
{
   return getParent()->isProtected();
}

void            OptIndex::setProtected(bool bProtected)
{
   ASSERT(false);
}

bool            OptIndex::isPreemphasized() const
{
   return getParent()->isPreemphasized();
}

void            OptIndex::setPreemphasized(bool bPreemph)
{
   ASSERT(false);
}

bool            OptIndex::isIncremental() const
{
   return getParent()->isIncremental();
}

void            OptIndex::setIncremental(bool bState)
{
   ASSERT(false);
}

bool            OptIndex::hasCDText() const
{
   return false;
}

void            OptIndex::setCDText(bool bCDText)
{
   ASSERT(false);
}

const char*     OptIndex::getCDTTitle() const
{
   return 0;
}

void            OptIndex::setCDTTitle(const char* sNewTitle)
{
   ASSERT(false);
}

const char*     OptIndex::getCDTArtist() const
{
   return 0;
}

void            OptIndex::setCDTArtist(const char* sNewArtist)
{
   ASSERT(false);
}

const char*     OptIndex::getCDTMessage() const
{
   return 0;
}

void            OptIndex::setCDTMessage(const char* sNewMessage)
{
   ASSERT(false);
}

const char*     OptIndex::getCDTLyrics() const
{
   return 0;
}

void            OptIndex::setCDTLyrics(const char* sNewLyrics)
{
   ASSERT(false);
}

const char*     OptIndex::getCDTComposer() const
{
   return 0;
}

void            OptIndex::setCDTComposer(const char* sNewComposer)
{
   ASSERT(false);
}

const char*     OptIndex::getCDTDirector() const
{
   return 0;
}

void            OptIndex::setCDTDirector(const char* sNewDirector)
{
   ASSERT(false);
}

bool            OptIndex::isBlank() const
{
   return getParent()->isBlank();
}

void            OptIndex::setBlank(bool blank)
{
   ASSERT(false);
}

uint16          OptIndex::getSectorSize() const
{
   return getParent()->getSectorSize();
}

void            OptIndex::setSectorSize(uint16 size)
{
   ASSERT(false);
}

uint16          OptIndex::getPacketSize() const
{
   return getParent()->getPacketSize();
}

void            OptIndex::setPacketSize(uint16 NewSize)
{
   ASSERT(false);
}
    
