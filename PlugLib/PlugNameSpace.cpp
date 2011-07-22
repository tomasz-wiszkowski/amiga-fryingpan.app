#include "PlugNameSpace.h"
#include "Main.h"
#include "PlugExternal.h"

using namespace GenNS;

PlugNameSpace::PlugNameSpace(const IPlugInternal* parent, const char* name)
{
    pn_Name = name;
    pn_Parent = parent;
    _DX(Lvl_Info, "Created new namespace %s", (iptr)name);
}

PlugNameSpace::~PlugNameSpace()
{
    Dispose();
}
    
bool PlugNameSpace::Dispose()
{
    _DX(Lvl_Info, "Destroying namespace %s", (iptr)GetName());

    if (GetChildCount() > 0)
    {
	_DX(Lvl_Error, "There are still %ld nodes here! Cannot delete yet.", GetChildCount());
	return false;
    }
    if (0 != GetParent())
    {
	const_cast<IPlugInternal*>(GetParent())->RemChild(this);
	_DX(Lvl_Info, "Namespace %s disposed", (iptr)GetName());
    }
    return true;
}

bool PlugNameSpace::AddChild(const IPlugInternal* node)
{
    _DX(Lvl_Info, "Adding node %s to namespace %s", (iptr)node->GetName(), (iptr)GetName());
    pn_Nodes << node;
    return true;
}

bool PlugNameSpace::RemChild(const IPlugInternal* node)
{
    _DX(Lvl_Info, "Removing node %s", (iptr)node->GetName());
    pn_Nodes >> node;

    Dispose();
    return true;
}

const IPlugInternal* PlugNameSpace::FindNode(const TagItem* spec)
{
    const IPlugInternal *p = 0;
    if (spec->ti_Tag == PLO_NameSpace)
    {
	_DX(Lvl_Info, "Looking up namespace %s...", spec->ti_Data);
	const IPlugInternal *plg = intFindCreateNS((const char*)spec->ti_Data);
	++spec;
	if (plg != 0)
	    p = const_cast<IPlugInternal*>(plg)->FindNode(spec);
    }
    else if (spec->ti_Tag == PLO_PluginName)
    {
	_DX(Lvl_Info, "Looking up plugin %s...", spec->ti_Data);
	p = intFindCreateExt(spec);
    }
    else
    {
	_DX(Lvl_Error, "Invalid tag in chain: %08lx\n", spec->ti_Tag);
    }

    if (0 == p)
	Dispose();
    return p;
}

const IPlugInternal* PlugNameSpace::intFindCreateNS(const char* name)
{
    const IPlugInternal* pi;
    _DX(Lvl_Info, "Searching for namespace %s...", (iptr)name);
    for (int i=0; i<GetChildCount(); ++i)
    {
	pi = GetChild(i);

	if (pi->IsNameSpace() && (!strcmp(name, GetChild(i)->GetName())))
	{
	    _DX(Lvl_Info, "Namespace address: %08lx", (iptr)pi);
	    const_cast<IPlugInternal*>(pi)->Obtain();
	    return pi;
	}
    }

    pi = new PlugNameSpace(this, name);
    const_cast<IPlugInternal*>(pi)->Obtain();
    AddChild(pi);
    _DX(Lvl_Info, "Namespace not found, created one at %08lx", (iptr)pi);
    return pi;
}

const IPlugInternal* PlugNameSpace::intFindCreateExt(const struct TagItem* ti)
{
    const char *name = 0;
    int version = 0;
    int revision = 0;
    bool noload = false;
    IPlugInternal* pi;

    name = (const char*)Utility->GetTagData(PLO_PluginName, (iptr)name, ti);
    version = Utility->GetTagData(PLO_MinVersion, version, ti);
    revision = Utility->GetTagData(PLO_MinRevision, revision, ti);
    noload = Utility->GetTagData(PLO_NoLoad, noload, ti);

    _DX(Lvl_Info, "Searching for plugin %s, v%ld.%ld...", (iptr)name, version, revision);
    for (int i=0; i<GetChildCount(); ++i)
    {
	pi = const_cast<IPlugInternal*>(GetChild(i));

	if ((!pi->IsNameSpace()) && (!strcmp(name, GetChild(i)->GetName())))
	{
	    _DX(Lvl_Info, "Plugin address: %08lx", (iptr)pi);
	    pi->Obtain();
	    return pi;
	}
    }

    pi = new PlugExternal(this, name, version, revision, noload);

    pi->Obtain();
    AddChild(pi);

    _DX(Lvl_Info, "ExtPlugin @ %08lx, state: %ld.", (iptr)pi, pi->IsValid());
    return pi;
}
