#ifndef _OPTTRACK_H_
#define _OPTTRACK_H_

/*
 * optical internal use only
 */

#include "OptItem.h"

class OptSession;

class OptTrack : public OptItem
{
public:
                           OptTrack(IOptItem* parent);
   virtual EItemType       getItemType() const;
   virtual void            setItemType(EItemType lNewType);
   virtual uint32          getDiscID() const;
   virtual void            setDiscID(uint32 id);
   virtual IOptItem       *addChild();
};

#endif
