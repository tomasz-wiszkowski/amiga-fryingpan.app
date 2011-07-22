/*
 * FryingPan - Amiga CD/DVD Recording Software (User Intnerface and supporting Libraries only)
 * Copyright (C) 2001-2008 Tomasz Wiszkowski Tomasz.Wiszkowski at gmail.com
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef _GUIMUI_MUITRACKSISOBUILDER_H_
#define _GUIMUI_MUITRACKSISOBUILDER_H_

#include "MUICommon.h"    
#include "MUIPopISOElement.h"
#include <Generic/ConfigParser.h>
#include <Generic/HookT.h>
#include <Generic/FileReq.h>
#include <workbench/workbench.h>
#include <Generic/MUI/MUIList.h>
#include <Generic/MUI/MUITree.h>

class IEngine;
class ClElement;
class Globals;

class MUITracksISOBuilder : public MUICommon
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
   Globals                   &Glb;
   GenNS::MUITree   *dirs;
   GenNS::MUIList   *files;
   GenNS::FileReq   *req;
   GenNS::FileReq   *buildimg;
   MUIPopISOElement *popelem;

   iptr                      *all;

   ConfigParser              *Config;
   HookT<MUITracksISOBuilder, const char**, class ClDirectory*>   hHkTreeDisplayHook;
   HookT<MUITracksISOBuilder, BtnID, void*>                       hHkButton;
   HookT<MUITracksISOBuilder, ClDirectory*, void*>                hHkTreeSelect;
   HookT<MUITracksISOBuilder, Entry*, void*>                      hHkElemSelect;
   HookT<MUITracksISOBuilder, AppMessage*, void*>                 hHkWBMessage;
   
   HookT<MUITracksISOBuilder, void*, class ClElement*>            hHkFilesConstruct;
   HookT<MUITracksISOBuilder, void*, class Entry*>                hHkFilesDestruct;
   HookT<MUITracksISOBuilder, const char**, class Entry*>         hHkFilesDisplay;

protected:
   void                       showTree(class IBrowser*);
   void                       showContents(class IBrowser*);
   void                       addTreeEntries(uint32 parent, ClDirectory *dir);

   void                       addFiles(IEngine*, IBrowser*);
   void                       removeFiles(IEngine*, IBrowser*);
   void                       buildImage(IEngine*, IBrowser*);

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
                              MUITracksISOBuilder(ConfigParser *parent, Globals &glb);
                             ~MUITracksISOBuilder();
   iptr                      *getObject();
   bool                       start();
   void                       stop();
   const char                *getName();
   void                       update();

   void                       disableISO();
   void                       enableISO();
};

#endif

