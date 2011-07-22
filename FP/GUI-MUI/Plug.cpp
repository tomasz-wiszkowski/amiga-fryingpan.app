#include <libclass/exec.h>
#include <libclass/dos.h>
#include <libclass/intuition.h>
#include <libclass/utility.h>
#include <PlugLib/PlugLib.h>
#include <Generic/Debug.h>
#include <Generic/Types.h>
#include <PlugLib/IPlugin.h>
#include <FP/GUI/GenericUI.h>
#include "GUI.h"

using namespace GenNS;

static bool setup(struct Library*);
static void cleanup();

struct PluginHeader myHeader = {
    Plugin_Header_Magic,	// magic
    &myHeader,			// self pointer
    GUI_Name,			// name
    Plugin_Header_Version,	// header version
    0,				// flags
    GUI_Version,		// version
    GUI_Revision,		// revision
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

bool setup(struct Library* exec)
{
    SysBase = exec;
    __setup();
    Exec = ExecIFace::GetInstance(SysBase);
    DOS = DOSIFace::GetInstance(0);
    Utility = UtilityIFace::GetInstance(0);
    Intuition = IntuitionIFace::GetInstance(0);

    plug = PlugLibIFace::GetInstance(0);

    FAIL(plug == 0, "plug.library failed to open.")
    {
	/*
	** this case would be extremely odd, but not impossible.
	*/
	cleanup();
	return false;
    } 

    myHeader.pa_Plugin = new GUI();
    return true;
}

void cleanup()
{
    if (myHeader.pa_Plugin != 0)
	delete (GUI*)myHeader.pa_Plugin;

    if (plug != 0)
	plug->FreeInstance();

    Utility->FreeInstance();
    DOS->FreeInstance();
    Intuition->FreeInstance();
    Exec->FreeInstance();
    __cleanup();

    myHeader.pa_Plugin = 0;
    plug = 0;
}


