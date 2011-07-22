#ifndef _ISOBUILDER_ISOBUILDER_H
#define _ISOBUILDER_ISOBUILDER_H

#include <PlugLib/IPluginT.h>
#include <Generic/Locale.h>

class IBrowser;

#define ISOBuilder_Name		"isobuilder.plugin"
#define ISOBuilder_Version	1
#define ISOBuilder_Revision	0

enum ISOBuilder_Tags
{

};

class ISOBuilder
{
protected:
    virtual ~ISOBuilder() {}

public:
    virtual IData*  CreateISO(const struct TagItem* tags) const = 0;
    virtual bool    DumpCatalog() const = 0;
};

typedef IPluginT<ISOBuilder*>    ISOBuilderPlugin;

#endif
