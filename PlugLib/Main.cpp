#include <libclass/utility.h>
#include <libclass/dos.h>
#include <utility/tagitem.h>
#include <Startup/Startup.h>
#include <GenericLib/Library.h>
#include "IPlugin.h"
#include "PlugLib.h"
#include "PlugNameSpace.h"
#include "PlugRoot.h"
#include "Main.h"
#include <LibC/LibC.h>
#include <exec/tasks.h>
#include <Generic/Debug.h>
#include <Generic/RWSyncT.h>

/*
 * this simple library uses mechanisms offered by utility
 * to manage and store plugins.
 *
 * This library ensures automatic load and dispose of all modules.
 */

using namespace GenNS;
class IPlugin;

    LIBRARY("plug.library", "$VER: plug.library 1.0 Tomasz Wiszkowski", 1);
    GATE1(const IPlugin*, OpenPlugin,  const TagItem*, a0);

    LIB_FT_Begin
	LIB_FT_Function(OpenPlugin)
    LIB_FT_End

    
    uint32	    StartupFlags = 0;
    ExecIFace*	    Exec = 0;
    UtilityIFace*   Utility = 0;
    DOSIFace*	    DOS = 0;


    SyncT<PlugRoot> *Root;

    DEFINE_DEBUG;



bool Lib_SetUp()
{
    __setup();
    Exec = ExecIFace::GetInstance(SysBase);
    Utility = UtilityIFace::GetInstance(0);
    DOS = DOSIFace::GetInstance(0);

    _ND("PlugLib");
    _D(Lvl_Info, "Initializing library");

    OurBase->lib_Flags |= LIBF_DELEXP;

    Root = new SyncT<PlugRoot>();
    return true;
}

void Lib_CleanUp()
{
    if (0 != Root)
    {
	_D(Lvl_Info, "Freeing PluginRoot");
	delete Root;
	Root = 0;
    }

    _D(Lvl_Info, "Unloading library");
    _ED();

    DOS->FreeInstance();
    Utility->FreeInstance();
    Exec->FreeInstance();

    __cleanup();
}

bool Lib_Acquire()
{
    return true;
}

void Lib_Release()
{
}

const IPlugin* OpenPlugin(const TagItem* tags)
{
    const IPlugInternal *ip = 0;

    _DX(Lvl_Info, "Acquiring semaphore by task %08lx (%s)", (iptr)Exec->FindTask(0), (iptr)Exec->FindTask(0)->tc_Node.ln_Name);
    PlugRoot &pr = Root->Acquire();
#warning Acquire Semaphore removed temporarily!!
    Root->Release();

    _DX(Lvl_Info, "Opening plugin");
    ip = pr.FindNode(tags);

    //_DX(Lvl_Info, "Releasing semaphore by task %08lx", (iptr)Exec->FindTask(0));
    //Root->Release();

    _DX(Lvl_Info, "Done");
    return static_cast<const IPlugin*>(ip);
}

int main()
{
    request("Information", "This file is not meant to be executed", "Ok", 0);
    return -1;
}


