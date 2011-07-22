#ifndef _PLUGLIB_PLUGEXTERNAL_H
#define _PLUGLIB_PLUGEXTERNAL_H

/**
 * \file Plugin.h
 * \brief Body specification for plugin node
 */

#include "IPlugInternal.h"
#include "PlugLib.h"
#include <dos/dos.h>
#include <libclass/dos.h>
#include <LibC/LibC.h>
#include <Generic/Debug.h>

using namespace GenNS;
class PlugNameSpace;

DECLARE_DEBUG;

/**
 * \addtogroup Structures Classes, structures
 * @{
 */
/**
 * \brief Plugin structure.
 * \details Structure backplane for inheriting plugins and namespaces.
 * \internal
 * \copydetails IPlugInternal
 */
class PlugExternal : public IPlugInternal
{
private:
    uint32		    pn_OpenCount;
    const IPlugInternal*    pn_Parent;
    BPTR		    pn_Segment;
    PluginHeader*	    pn_Header;

protected:
    virtual ~PlugExternal();
    bool tryOpenPlugin(const char* path, const char* name, int minver, int minrev);

public:

    PlugExternal(const IPlugInternal* parent, const char* name, int minver, int minrev, bool noload);

public:	    // interface
    virtual void Obtain();
    virtual bool Dispose();
    virtual const char* GetName() const;

    //----------------------------------------------------------------
    virtual int GetVersion() const
    {
	_D(Lvl_Info, __PRETTY_FUNCTION__);
	return pn_Header->pa_Version;
    }

    //----------------------------------------------------------------
    virtual int GetRevision() const
    {
	_D(Lvl_Info, __PRETTY_FUNCTION__);
	return pn_Header->pa_Revision;
    }

    //----------------------------------------------------------------
    virtual bool IsValid() const
    {
	_D(Lvl_Info, __PRETTY_FUNCTION__);
	return (pn_Header != 0);
    }

    //----------------------------------------------------------------
    virtual void* GetObject() const
    {
	_D(Lvl_Info, __PRETTY_FUNCTION__);
	ASSERTS(IsValid(), "Plugin not loaded, but function called!\nThis program is going to crash");
	return pn_Header->pa_Plugin;
    }

protected:  // interface
    
    //----------------------------------------------------------------
    virtual const IPlugInternal* GetParent() const
    {
	_D(Lvl_Info, __PRETTY_FUNCTION__);
	return pn_Parent;
    }
    
    //----------------------------------------------------------------
    virtual bool AddChild(const IPlugInternal* node)
    {
	_D(Lvl_Info, __PRETTY_FUNCTION__);
	return false;
    }
    
    //----------------------------------------------------------------
    virtual bool RemChild(const IPlugInternal* node)
    {
	_D(Lvl_Info, __PRETTY_FUNCTION__);
	return false;
    }
    
    //----------------------------------------------------------------
    virtual bool IsNameSpace() const
    {
	return false;
	_D(Lvl_Info, __PRETTY_FUNCTION__);
    }
    
    //----------------------------------------------------------------
    virtual const IPlugInternal* FindNode(const struct TagItem* spec)
    {
	_D(Lvl_Info, __PRETTY_FUNCTION__);
	return 0;
    }

    //----------------------------------------------------------------
    virtual int GetChildCount() const
    {
	_D(Lvl_Info, __PRETTY_FUNCTION__);
	return 0;
    }
    
    //----------------------------------------------------------------
    virtual const IPlugInternal* GetChild(int i) const
    {
	_D(Lvl_Info, __PRETTY_FUNCTION__);
	return 0;
    }
};

#endif
