#ifndef _ISOBUILDER_CUSTOMISOCONFIGPAGE_H_
#define _ISOBUILDER_CUSTOMISOCONFIGPAGE_H_ 

#include "CustomISOPopUp.h"
#include <Generic/HookT.h>
#include <Generic/FileReq.h>
#include <workbench/workbench.h>
#include <Generic/MUI/MUIList.h>
#include <Generic/MUI/MUITree.h>
#include "Globals.h"

class IEngine;
class ClElement;
class ClISO;

class CustomISOConfigPage : public MUI
{
protected:
   enum BtnID
   {
      ID_Add,
      ID_Remove,
      ID_MakeDir,
      ID_BuildImage,
      ID_Name
   };

   struct Entry 
   {
      ClElement     *elem;
      String         s1;
      String         s2;
      String         s3;
   };

protected:
   const Globals&	    Glb;
   GenNS::MUITree   *dirs;
   GenNS::MUIList   *files;
   GenNS::FileReq   *req;
   GenNS::FileReq   *buildimg;
   CustomISOPopUp   *popelem;

   ClISO*			owner;
   ClDirectory*			currdir;
   iptr*			all;

   HookT<CustomISOConfigPage, const char**, class ClDirectory*>   hHkTreeDisplayHook;
   HookT<CustomISOConfigPage, BtnID, void*>                       hHkButton;
   HookT<CustomISOConfigPage, ClDirectory*, void*>                hHkTreeSelect;
   HookT<CustomISOConfigPage, Entry*, void*>                      hHkElemSelect;
   HookT<CustomISOConfigPage, AppMessage*, void*>                 hHkWBMessage;
   
   HookT<CustomISOConfigPage, void*, class ClElement*>            hHkFilesConstruct;
   HookT<CustomISOConfigPage, void*, class Entry*>                hHkFilesDestruct;
   HookT<CustomISOConfigPage, const char**, class Entry*>         hHkFilesDisplay;

protected:
   void                       showTree();
   void                       showContents();
   void                       addTreeEntries(uint32 parent, ClDirectory *dir);

   void                       addFiles();
   void                       removeFiles();
   void                       buildImage();

protected:
   iptr                       treeDisplayHook(const char**, ClDirectory*);
   iptr                       button(BtnID, void*);
   iptr                       treeSelect(ClDirectory*, void*);
   iptr                       elemSelect(Entry*, void*);
   iptr                       onWBMessage(AppMessage*, void*);

   iptr                       filesConstruct(void*, ClElement*);
   iptr                       filesDestruct(void*, Entry*);
   iptr                       filesDisplay(const char**, Entry*);

public:
                              CustomISOConfigPage(ClISO* iso, const Globals& glb);
                             ~CustomISOConfigPage();
   iptr                      *getObject();
   void                       disableISO();
   void                       enableISO();

   /* static methods */
   static bool	InitLocale(Globals& glb);
};

#endif


