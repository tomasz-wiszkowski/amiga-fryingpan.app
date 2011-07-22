#ifndef _PLUGLIB_PLUGROOT_H
#define _PLUGLIB_PLUGROOT_H

#include "PlugNameSpace.h"
#include <Generic/VectorT.h>
#include <Generic/RWSyncT.h>
#include <Generic/String.h>

/**
 * \file PlugRoot.h
 * \brief PlugLib internals
 */

/**
 * \copydetails IPlugNode
 */

using namespace GenNS;

class PlugRoot : public PlugNameSpace
{
protected:
public:
    PlugRoot() : PlugNameSpace(0, "-- All Modules --")
    {
    }

    virtual ~PlugRoot()
    {
    }

    virtual const IPlugInternal* FindNode(const struct TagItem* spec)
    {
	return PlugNameSpace::FindNode(spec);
    }
};



#endif

