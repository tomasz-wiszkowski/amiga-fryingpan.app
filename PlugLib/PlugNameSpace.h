#ifndef _PLUGLIB_PLUGNAMESPACE_H
#define _PLUGLIB_PLUGNAMESPACE_H

#include "IPlugInternal.h"
#include <Generic/VectorT.h>
#include <Generic/RWSyncT.h>
#include <Generic/String.h>
#include <Generic/Debug.h>

/**
 * \file PlugNameSpace.h
 * \brief PlugLib internals
 */

/**
 * \copydetails IPlugInternal
 */

using namespace GenNS;

class PlugNameSpace : public IPlugInternal
{
    String		    pn_Name;
    VectorT<const IPlugInternal*> pn_Nodes;
    const IPlugInternal*    pn_Parent;
    
public:
    PlugNameSpace(const IPlugInternal* parent, const char* name);
    virtual ~PlugNameSpace();

protected:
    const IPlugInternal* intFindCreateNS(const char* name);
    const IPlugInternal* intFindCreateExt(const struct TagItem* ti);

public:	    // interface
    virtual bool Dispose();
    
    //----------------------------------------------------------------
    virtual void Obtain()
    {
    }
    
    //----------------------------------------------------------------
    virtual const char* GetName() const
    {
	return pn_Name.Data();
    }

    //----------------------------------------------------------------
    virtual bool IsValid() const
    {
	return true;
    }
    
    //----------------------------------------------------------------
    virtual void* GetObject() const
    {
	ASSERTS(false, "GetObject() called on Namespace object!");
	return 0;
    }
    

protected:  // interface
    
    //----------------------------------------------------------------
    virtual const IPlugInternal* FindNode(const struct TagItem* spec);
    virtual bool AddChild(const IPlugInternal* node);
    virtual bool RemChild(const IPlugInternal* node);
    
    //----------------------------------------------------------------
    virtual const IPlugInternal* GetParent() const
    {
	return pn_Parent;
    }
    
    //----------------------------------------------------------------
    virtual bool IsNameSpace() const
    {
	return true;
    }
    
    //----------------------------------------------------------------
    virtual int GetChildCount() const
    {
	return pn_Nodes.Count();
    }
    
    //----------------------------------------------------------------
    virtual const IPlugInternal* GetChild(int i) const
    {
	return pn_Nodes[i];
    }
    
    //----------------------------------------------------------------
    virtual int GetVersion() const
    {
	return 0;
    }

    //----------------------------------------------------------------
    virtual int GetRevision() const
    {
	return 0;
    }
};



#endif
