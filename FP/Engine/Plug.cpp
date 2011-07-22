#include <libclass/exec.h>
#include <libclass/dos.h>
#include <libclass/intuition.h>
#include <libclass/utility.h>
#include <PlugLib/PlugLib.h>
#include <Generic/Debug.h>
#include <Generic/Types.h>
#include <PlugLib/IPlugin.h>
#include <FP/GUI/GenericUI.h>
#include "Plug.h"
#include "Globals.h"
#include "Engine.h"

using namespace GenNS;

static bool setup(struct Library*);
static void cleanup();

struct PluginHeader myHeader = {
    Plugin_Header_Magic,	// magic
    &myHeader,			// self pointer
    Engine_Name,		// name
    Plugin_Header_Version,	// header version
    0,				// flags
    Engine_Version,		// version
    Engine_Revision,		// revision
    0,				//(TagItem* const)&myTags
    pa_Plugin:  0,
    pf_SetUp:	&setup,
    pf_CleanUp: &cleanup,
};
 
uint32 StartupFlags = 0;

PlugLibIFace *plug;
ExecIFace *Exec;
DOSIFace *DOS;
IntuitionIFace *Intuition;
UtilityIFace *Utility;



class Plugin : public EnginePlug
{
private:
    Globals	    g;

public:
    Plugin(PlugLibIFace *plg) :
	g(plg)
    {
    }

    virtual ~Plugin()
    {
    }

    virtual IEngine*	GetEngine(int id)
    {
	return g.Engines[id];
    }

    virtual const ScanData* ScanDevice(const char* device)
    {
	if (g.Optical.IsValid())
	    return (ScanData*)g.Optical->DoMethodA(ARRAY(DRV_ScanDevice, (long)device));
	return 0;
    }

    virtual void FreeScanData(const ScanData* data)
    {
	if (g.Optical.IsValid())
	    g.Optical->DoMethodA(ARRAY(DRV_FreeScanResults, (long)data));
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
    plug = PlugLibIFace::GetInstance(0);

    myHeader.pa_Plugin = new Plugin(plug);

    return true;
}

void cleanup()
{
    delete (Plugin*)myHeader.pa_Plugin;

    plug->FreeInstance();
    Utility->FreeInstance();
    DOS->FreeInstance();
    Intuition->FreeInstance();
    Exec->FreeInstance();
    __cleanup();
}

int main()
{
    request("Information", "This module carries FryingPan Main Engine.\nTo use it you need to launch executable.", "Ok", 0);
    return -1;
}



