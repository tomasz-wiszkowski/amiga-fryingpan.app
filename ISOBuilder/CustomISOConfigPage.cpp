#include "CustomISOConfigPage.h"
#include <libclass/dos.h>
#include <libclass/intuition.h>
#include <libraries/mui.h>
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>
#include <mui/NListview_mcc.h>
#include <Generic/MUI/MUIList.h>
#include <Generic/MUI/MUITree.h>
#include <FP/IEngine.h>
#include <workbench/startup.h>
#include "ClISO.h"

/*
* localization area
*/
enum Loc
{
    loc_Name	    =	ILocalization::Grp_Main,
    loc_Add,
    loc_Remove,
    loc_MakeDir,
    loc_BuildISO,

    loc_ColDirTree = ILocalization::Item_1,

    loc_ColFileName = ILocalization::Item_2,
    loc_ColFileSize,

    loc_SelectFiles = ILocalization::Item_15,
    loc_SelectTarget
};

static class ILocalization::LocaleSet LocaleSets[] =
{
    {  loc_Name,         "ISO Builder",             "LBL_ISOBUILDER"        },
    {  loc_Add,          "a&Add",                   "BTN_ADD"               },
    {  loc_Remove,       "r&Remove",                "BTN_REMOVE"            },
    {  loc_MakeDir,      "c&Create Dir",            "BTN_CREATEDIR"         },
    {  loc_BuildISO,     "b&Build Image",           "BTN_BUILDIMAGE"        },
    {  loc_ColDirTree,   "Dir Tree",                "COL_DIRTREE"           },
    {  loc_ColFileName,  "Item Name",               "COL_ITEMNAME"          },
    {  loc_ColFileSize,  "Item Size",               "COL_ITEMSIZE"          },

    {  loc_SelectFiles,
	"Select files to be added",
	"REQ_SELECTFILES" },
    {  loc_SelectTarget,
	"Select target image file",
	"REQ_SELECTTARGET" },

    {  Localization::LocaleSet::Set_Last, 0, 0                              }
};

static const char* LocaleGroup = "TRACKS_ISO";

static const char*   Cfg_LastImagePath    =  "LastImagePath";
static const char*   Cfg_LastSourcePath   =  "LastSourcePath";

CustomISOConfigPage::CustomISOConfigPage(ClISO* par, const Globals &glb) :
    Glb(glb),
    owner(par)
{
    all    = 0;
    files  = 0;
    dirs   = 0;
    popelem= 0;
    currdir= 0;

    hHkTreeDisplayHook.Initialize(this, &CustomISOConfigPage::treeDisplayHook);
    hHkButton.Initialize(this, &CustomISOConfigPage::button);
    hHkTreeSelect.Initialize(this, &CustomISOConfigPage::treeSelect);
    hHkElemSelect.Initialize(this, &CustomISOConfigPage::elemSelect);
    hHkWBMessage.Initialize(this, &CustomISOConfigPage::onWBMessage);

    hHkFilesConstruct.Initialize(this, &CustomISOConfigPage::filesConstruct);
    hHkFilesDestruct.Initialize(this, &CustomISOConfigPage::filesDestruct);
    hHkFilesDisplay.Initialize(this, &CustomISOConfigPage::filesDisplay);

    setButtonCallBack(hHkButton.GetHook());
}

CustomISOConfigPage::~CustomISOConfigPage()
{
    /*
    ** proceed with deletion only if we know, that the object has been constructed before.
    */
    if (all != 0)
    {
	/*
	** save configuration
	*/
	Glb.Config.setValue(Cfg_LastSourcePath, req->getPath());
	Glb.Config.setValue(Cfg_LastImagePath, buildimg->getPath());

	/*
	** dispose main object
	*/
	MUIMaster->MUI_DisposeObject(all);

	/*
	** free objects
	*/
	delete dirs;
	delete files;
	delete popelem;
	delete req;
	delete buildimg;
    }


    dirs = 0;
    files = 0;
    popelem = 0;
    req = 0;
    buildimg = 0;
    all = 0;

    /*
    ** and quit.
    */
}

iptr *CustomISOConfigPage::getObject()
{
    if (NULL != all)
	return all;

    files  = new GenNS::MUIList("BAR,BAR", true); 
    FAIL(files == 0, "Couldn't create nlist object")
    {
	return 0;
    }

    dirs   = new GenNS::MUITree("");
    FAIL(dirs == 0, "Couldn't create tree object")
    {
	delete files;
	files = 0;
	return 0;
    }

    req    = new GenNS::FileReq(Glb.Loc[loc_SelectFiles]);
    FAIL(req == 0, "Couldn't create requester")
    {
	delete files;
	delete dirs;
	files = 0;
	dirs = 0;
	return 0;
    }

    buildimg = new GenNS::FileReq(Glb.Loc[loc_SelectTarget]);
    FAIL(buildimg == 0, "Couldn't create save rqueseter")
    {
	delete req;
	delete dirs;
	delete files;
	req = 0;
	dirs = 0;
	files = 0;
	return 0;
    }

    popelem = new CustomISOPopUp(Glb); 
    FAIL(popelem == 0, "Could not create pop-up")
    {
	delete buildimg;
	delete req;
	delete dirs;
	delete files;
	req = 0;
	dirs = 0;
	files = 0;
	buildimg = 0;
	return 0;
    }

    req->setMultiSelect(true);
    buildimg->setPath(Glb.Config.getValue(Cfg_LastImagePath, ""));

    dirs->setDisplayHook(hHkTreeDisplayHook.GetHook());
    dirs->setSelectionHook(hHkTreeSelect.GetHook());

    files->setConstructHook(hHkFilesConstruct.GetHook());
    files->setDestructHook(hHkFilesDestruct.GetHook());
    files->setDisplayHook(hHkFilesDisplay.GetHook());
    files->setSelectionHook(hHkElemSelect.GetHook());
    files->setWBDropHook(hHkWBMessage.GetHook());

    dirs->setWeight(40);

    req->setPath(Glb.Config.getValue(Cfg_LastSourcePath, ""));

    all = VGroup,
	Child,                  HGroup,
	Child,                  dirs->getObject(),

	Child,                  BalanceObject,
	End,

	Child,                  files->getObject(),
	End,

	Child,                  ColGroup(4),
	Child,                  muiButton(Glb.Loc[loc_Add], Glb.Loc.Accel(loc_Add), ID_Add),
	Child,                  muiButton(Glb.Loc[loc_Remove], Glb.Loc.Accel(loc_Remove), ID_Remove),
	Child,                  muiButton(Glb.Loc[loc_MakeDir], Glb.Loc.Accel(loc_MakeDir), ID_MakeDir),
	Child,                  muiButton(Glb.Loc[loc_BuildISO], Glb.Loc.Accel(loc_BuildISO), ID_BuildImage),
	End,

	Child,                  popelem->getObject(),
	End;

    FAIL(all == 0, "Couldn't build ISO config page");

    showTree();
    showContents();
    return all;
}

void CustomISOConfigPage::showTree()
{
    dirs->clear();

    if (currdir == 0)
	currdir = owner->getRoot();

    addTreeEntries(0, owner->getRoot());
}

void CustomISOConfigPage::showContents()
{
    files->clear();

    if (currdir == 0)
	currdir = owner->getRoot();

    for (uint32 i=0; i<currdir->getChildrenCount(); i++)
    {
	files->addItem(currdir->getChild(i));
    }
}

iptr CustomISOConfigPage::treeDisplayHook(const char** arr, ClDirectory* elem)
{
    if (elem != 0)
    {
	arr[0] = elem->getNormalName();
    }
    else
    {
	arr[0] = Glb.Loc[loc_ColDirTree];
    }
    return 0;
}

void CustomISOConfigPage::addTreeEntries(uint32 parent, ClDirectory* elem)
{
    parent = dirs->addEntry(parent, elem, elem == currdir);
    for (uint32 i=0; i<elem->getChildrenCount(); i++)
    {
	if (elem->getChild(i)->isDirectory())
	{
	    addTreeEntries(parent, static_cast<ClDirectory*>(elem->getChild(i)));
	}
    }
}

iptr CustomISOConfigPage::button(BtnID id, void* data)
{
    switch (id)
    {
	case ID_MakeDir:
	    {
#warning here!
		FAIL(true, "Reimplement me!");
		currdir->makeDir("New Directory");
		showContents();
	    }
	    break;

	case ID_Remove:
	    {
		removeFiles();
	    }
	    break;

	case ID_Add:
	    {
		addFiles();
	    }
	    break;

	case ID_BuildImage:
	    {
		buildImage();
	    }
	    break;

	default:
	    break;
    }

    return 0;
}

iptr CustomISOConfigPage::treeSelect(ClDirectory* dir, void*)
{
    if (dir != 0)
    {
	currdir = dir;
	showContents();
    }
    return 0;
}

iptr CustomISOConfigPage::elemSelect(Entry* elem, void*)
{
    if (elem != NULL)
    {
	popelem->setElement(elem->elem);
    }
    else
    {
	popelem->setElement(0);
    }
    return 0;
}

iptr CustomISOConfigPage::filesConstruct(void*, ClElement* clelem)
{
    Entry *e = new Entry;
    e->elem = clelem;

    return (uint32)e;
}

iptr CustomISOConfigPage::filesDestruct(void*, Entry* e)
{
    delete e;
    return 0;
}

iptr CustomISOConfigPage::filesDisplay(const char** arr, Entry* e)
{
    if (e == 0)
    {
	arr[0] = Glb.Loc[loc_ColFileName];
	arr[1] = Glb.Loc[loc_ColFileSize];
    }
    else
    {
	if (e->elem->isDirectory())
	{
	    e->s1.FormatStr("\0333%s", ARRAY((int32)e->elem->getNormalName()));
	    e->s2 = "\0333<DIR>";
	}
	else
	{
	    e->s1.FormatStr("%s", ARRAY((int32)e->elem->getNormalName()));
	    e->s2 = Glb.Loc.FormatNumber(e->elem->getISOSize(), 0);
	}

	arr[0] = e->s1.Data();
	arr[1] = e->s2.Data();
    }
    return 0;
}

void CustomISOConfigPage::addFiles()
{
    FAIL(currdir == 0, "Invalid directory selected.")
	return;

    VectorT<const char*> &v = req->openReq();

    for (uint32 i=0; i<v.Count(); i++)
    {
	currdir->addChild(v[i]);
    }

#warning mind update!
    showTree();
    showContents();
}

void CustomISOConfigPage::buildImage()
{
    //String name = buildimg->saveReq();

    request("Info", "Please re-implement", "OK", 0);
    //e->createImage(name.Data());
}

void CustomISOConfigPage::removeFiles()
{
    FAIL(currdir == 0, "No directory selected?")
	return;

    VectorT<Entry*> &v = (VectorT<Entry*>&)files->getSelectedItems();

    for (uint32 i=0; i<v.Count(); i++)
    {
	currdir->remChild(v[i]->elem);
    }

#warning mind update!
    showTree();
    showContents();
    //e->layoutTracks(false, false);
    //update();

}

void CustomISOConfigPage::disableISO()
{
    files->setEnabled(false);
    dirs->setEnabled(false);
    popelem->setEnabled(false);
    muiSetEnabled(ID_Add, false);
    muiSetEnabled(ID_Remove, false);
    muiSetEnabled(ID_MakeDir, false);
    muiSetEnabled(ID_BuildImage, false);
    popelem->close();
}

void CustomISOConfigPage::enableISO()
{
    files->setEnabled(true);
    dirs->setEnabled(true);
    popelem->setEnabled(true);
    muiSetEnabled(ID_Add, true);
    muiSetEnabled(ID_Remove, true);
    muiSetEnabled(ID_MakeDir, true);
    muiSetEnabled(ID_BuildImage, true);
}

iptr CustomISOConfigPage::onWBMessage(AppMessage* m, void*)
{
    char *c = new char[1024];

    for (int32 i=0; i<m->am_NumArgs; i++)
    {
	DOS->NameFromLock(m->am_ArgList[i].wa_Lock, c, 1024);
	DOS->AddPart(c, (char*)m->am_ArgList[i].wa_Name, 1024);
	currdir->addChild(c);
    }

    delete []c;
    return 0;
}

bool CustomISOConfigPage::InitLocale(Globals &glb)
{
    glb.Loc.AddGroup((ILocalization::LocaleSet*)LocaleSets, LocaleGroup);
    return CustomISOPopUp::InitLocale(glb);
}
