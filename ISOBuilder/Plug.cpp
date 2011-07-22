#include <libclass/exec.h>
#include <libclass/dos.h>
#include <libclass/intuition.h>
#include <libclass/utility.h>
#include <PlugLib/PlugLib.h>
#include <Generic/Debug.h>
#include <Generic/Types.h>
#include <PlugLib/IPlugin.h>
#include "ClISO.h"
#include "ISOBuilder.h"
#include "Globals.h"

using namespace GenNS;

static bool setup(struct Library*);
static void cleanup();

struct PluginHeader myHeader = {
    Plugin_Header_Magic,	// magic
    &myHeader,			// self pointer
    ISOBuilder_Name,		// name
    Plugin_Header_Version,	// header version
    0,				// flags
    ISOBuilder_Version,		// version
    ISOBuilder_Revision,	// revision
    0,				//(TagItem* const)&myTags
    pa_Plugin:  0,
    pf_SetUp:	&setup,
    pf_CleanUp: &cleanup,
};
 
uint32 StartupFlags = 0;

PlugLibIFace*	plug;
ExecIFace*	Exec;
DOSIFace*	DOS;
IntuitionIFace* Intuition;
UtilityIFace*	Utility;
MUIMasterIFace*	MUIMaster;


class Builder : public ISOBuilder
{
protected:
    Globals	Glob;

public:
    Builder() :
	Glob()
    {
    }

    ~Builder()
    {
    }

    IData* CreateISO(const struct TagItem* tags) const
    {
#warning Do something with the tags.
	return new ClISO(Glob);
    }

    bool DumpCatalog() const
    {
	Glob.Loc.ExportCT("RAM:isobuilder.ct", LocaleVersion);
	Glob.Loc.ExportCD("RAM:isobuilder.cd", LocaleVersion);
	return true;
    }
};


bool setup(struct Library* exec)
{
    SysBase = exec;
    __setup();
    Exec = ExecIFace::GetInstance(SysBase);
    DOS = DOSIFace::GetInstance(0);
    Utility = UtilityIFace::GetInstance(0);
    Intuition = IntuitionIFace::GetInstance(0);
    MUIMaster = MUIMasterIFace::GetInstance(0);

    myHeader.pa_Plugin = new Builder();

    return true;
}

void cleanup()
{
    delete (Builder*)myHeader.pa_Plugin;

    MUIMaster->FreeInstance();
    Utility->FreeInstance();
    DOS->FreeInstance();
    Intuition->FreeInstance();
    Exec->FreeInstance();
    __cleanup();
}

