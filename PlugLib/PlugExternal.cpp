#include "PlugExternal.h"
#include <libclass/dos.h>
#include <dos/dos.h>
#include "PlugLib.h"
#include <LibC/LibC.h>
#include <Generic/String.h>
#include <Generic/Debug.h>
#include "Main.h"


bool PlugExternal::tryOpenPlugin(const char* path, const char* name, int minver, int minrev)
{
    iptr next;
    iptr* loc;
    iptr size;
    bool res = true;

    /*
    ** try to load-seg
    */
    pn_Segment = DOS->LoadSeg(path);
    if (0 == pn_Segment)
    {
	_DX(Lvl_Info, "Could not load plugin");
	return false;
    }

    /*
    ** this is a bit of a hacking, but
    ** this works everywhere, at least as long
    ** as platform claims to be amiga compatible
    */
    next = (iptr)pn_Segment;

    /*
    ** look for header
    */
    while ((0 == pn_Header) && (0 != next))
    {
	loc = (iptr*)BADDR(next);
	next = loc[0];
	size = loc[-1];

	size /= sizeof(iptr);
	while (size > 0)
	{
	    if ((loc[0] == Plugin_Magic_Value_1) &&
		(loc[1] == Plugin_Magic_Value_2) &&
		(loc[2] == Plugin_Magic_Value_3) &&
		(loc[3] == Plugin_Magic_Value_4) &&
		(loc[4] == (iptr)loc))
	    {
		_DX(Lvl_Info, "Plugin magic found at %08lx", (iptr)loc);
		_DX(Lvl_Info, ">> %08lx: %08lx %08lx %08lx %08lx %08lx", (iptr)loc, loc[0], loc[1], loc[2], loc[3], loc[4]);
		pn_Header = (PluginHeader*)loc;
		break;
	    }
	    --size;
	    ++loc;
	}
    }

    /*
     * have we found header?
     */
    if (0 == pn_Header)
    {
	_DX(Lvl_Info, "No header located. Not really a plugin.");
	res = false;
    }
    else
    {
	if ((pn_Header->pa_Version < minver) || 
	    ((pn_Header->pa_Version == minver) && (pn_Header->pa_Revision < minrev)))
	{
	    _DX(Lvl_Info, "Plugin version %ld.%ld does not meet min required version %ld.%ld", pn_Header->pa_Version, pn_Header->pa_Revision, minver, minrev);
	    res = false;
	}

	if (0 != strcmp(pn_Header->pa_PluginName, name))
	{
	    _DX(Lvl_Info, "Plugin name (%s) does not match desired name (%s)", (iptr)pn_Header->pa_PluginName, (iptr)name);
	    res = false;
	}

	if (0 != pn_Header->pf_SetUp)
	{
	    res = pn_Header->pf_SetUp(SysBase);
	}
    }

    if (!res)
    {
	_DX(Lvl_Info, "Unloading plugin segment");
	DOS->UnLoadSeg(pn_Segment);
	pn_Segment = 0;
	pn_Header = 0;
	return false;
    }
    _DX(Lvl_Info, "Plugin %s, %ld.%ld loaded.", (iptr)name, pn_Header->pa_Version, pn_Header->pa_Revision);
    return true;
}

PlugExternal::~PlugExternal()
{
    _D(Lvl_Info, __PRETTY_FUNCTION__);
    if (0 != pn_Segment)
    {
	if ((0 != pn_Header) && (0 != pn_Header->pf_CleanUp))
	    pn_Header->pf_CleanUp();
	DOS->UnLoadSeg(pn_Segment);
    }
}

bool PlugExternal::Dispose()
{
    Root->Acquire();
    _DX(Lvl_Info, "Releasing plugin %s", (iptr)GetName());
    --pn_OpenCount;
    if (pn_OpenCount == 0)
    {
	_DX(Lvl_Info, "Last instance released. Disposing plugin.");
	const_cast<IPlugInternal*>(GetParent())->RemChild(this);
	delete this;

	Root->Release();
	return true;
    }
	
    Root->Release();
    return false;
}
    
void PlugExternal::Obtain()
{
    _D(Lvl_Info, __PRETTY_FUNCTION__);
    ++pn_OpenCount;
}

const char* PlugExternal::GetName() const
{
    _D(Lvl_Info, __PRETTY_FUNCTION__);
    return pn_Header->pa_PluginName;
}

PlugExternal::PlugExternal(const IPlugInternal* parent, const char* name, int minver, int minrev, bool noload)
{
    const char* paths[] = { "", "plugins", "libs:plugins" };
    String nm;

    _D(Lvl_Info, __PRETTY_FUNCTION__);

    pn_Header = 0;
    pn_Segment = 0;
    pn_Parent = parent;
    pn_OpenCount = 0;

    if (!noload)
    {
	for (uint32 i=0; i<(sizeof(paths) / sizeof(*paths)); ++i)
	{
	    /*
	    ** go through all paths
	    */
	    nm = paths[i];
	    if (!nm.Length())
		nm = name;
	    else
		nm.AddPath(name);

	    _DX(Lvl_Info, "Trying to load %s as %s", (iptr)name, (iptr)nm.Data());
	    if (tryOpenPlugin(nm, name, minver, minrev))
		break;
	}
    }
}

