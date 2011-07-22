#include <libclass/exec.h>
#include <libclass/dos.h>
#include <libclass/intuition.h>
#include <libclass/utility.h>
#include <PlugLib/PlugLib.h>
#include <Generic/Debug.h>
#include <Generic/Types.h>
#include <PlugLib/IPlugin.h>
#include "Main.h"
#include "Optical.h"
#include "DriveSpool.h"
#include "DriveClient.h"
#include "Humming.h"
#include "Config.h"

using namespace GenNS;

Config          *Cfg;
ExecIFace	*Exec;
DOSIFace	*DOS;
IntuitionIFace	*Intuition;
UtilityIFace	*Utility;
bool AllClear;
bool Clear1, Clear2;
static bool setup(struct Library*);
static void cleanup();

struct PluginHeader myHeader = {
    Plugin_Header_Magic,	// magic
    &myHeader,			// self pointer
    Optical_Name,		// name
    Plugin_Header_Version,	// header version
    0,				// flags
    Optical_Version,		// version
    Optical_Revision,		// revision
    0,				//(TagItem* const)&myTags
    pa_Plugin:  0,
    pf_SetUp:	&setup,
    pf_CleanUp: &cleanup,
};


class Plugin : public Optical
{
public:
    Plugin()
    {
	Cfg = new Config();
	Clear2 = analyze(Clear1);
	DriveSpool::Init();
    }

    virtual IDriveClient*   OpenDrive(const char* name, int unit)
    {
	IDriveClient *clt = DriveSpool::GetDriveClient(name, unit);
	return clt;
    }

    virtual ~Plugin()
    {
	DriveSpool::Exit();

	while (!DbgMaster::CleanUp())
	    DOS->Delay(25);

	Cfg->onWrite();
	delete Cfg;

    }
};

bool setup(struct Library* exec)
{
    SysBase = exec;
    __setup();

    Exec = ExecIFace::GetInstance(SysBase);
    DOS = DOSIFace::GetInstance(0); 
    Intuition = IntuitionIFace::GetInstance(0);
    Utility = UtilityIFace::GetInstance(0);

    myHeader.pa_Plugin = new Plugin();
    return true;
}

void cleanup()
{
    delete (Plugin*)myHeader.pa_Plugin;

    Utility->FreeInstance();
    Intuition->FreeInstance();
    DOS->FreeInstance();
    Exec->FreeInstance();
    __cleanup();
}


