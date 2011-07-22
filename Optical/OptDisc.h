#ifndef _OPTDISC_H_
#define _OPTDISC_H_

/*
 * for optical's internal use only
 */

#include "OptSession.h"

class OptDisc : public OptSession
{
public:
                           OptDisc();
   virtual EItemType       getItemType() const;
   virtual uint32          getDiscID() const;
   virtual void            setDiscID(uint32 id);
   virtual uint16          getFlags() const;
   virtual void            setFlags(uint16 lNewFlags);
   virtual IOptItem       *addChild();
};

#endif
