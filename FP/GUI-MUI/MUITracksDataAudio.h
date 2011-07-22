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


#ifndef _GUIMUI_MUITRACKSDATAAUDIO_H_
#define _GUIMUI_MUITRACKSDATAAUDIO_H_

#include "MUICommon.h"
#include <Generic/ConfigParser.h>
#include <Generic/MUI/MUIList.h>
#include <Generic/MUI/MUIDynamicObject.h>
#include <Generic/FileReq.h>
#include "Globals.h"
#include "../ITrack.h"

class MUITracksDataAudio : public MUICommon
{
protected:
   enum BtnID
   {
      ID_Add,
      ID_Remove,
      ID_Play,
      ID_Return,
      ID_AddISO,
   };

protected:
   struct ITrackEntry
   {
      IData*	track;
      String    information;
   };

protected:
   Globals          &Glb;
   iptr             *all;

   GenNS::MUIList   *tracks;
   GenNS::MUIDynamicObject* config;
   GenNS::FileReq   *addReq;
   ConfigParser     *Config;
   

protected:
   DbgHandler                *getDebug();

protected:
   HookT<MUITracksDataAudio, BtnID, void*>                  hHkButton;
   HookT<MUITracksDataAudio, void*, IData*>		    hConstruct;
   HookT<MUITracksDataAudio, void*, ITrackEntry*>           hDestruct;
   HookT<MUITracksDataAudio, const char**, ITrackEntry*>    hDisplay;
   HookT<MUITracksDataAudio, AppMessage*, void*>            hWBMessage;
   HookT<MUITracksDataAudio, VectorT<ITrackEntry*>*, void*> hDragSort;
   HookT<MUITracksDataAudio, MUIList*, ITrackEntry*>	    hDblClick;

protected:
   iptr onButton(BtnID, void*);
   iptr onConstruct(void*, IData*);
   iptr onDestruct(void*, ITrackEntry*);
   iptr onDisplay(const char**, ITrackEntry*);
   iptr	onWBMessage(AppMessage* m, void*);
   iptr	onDragSort(VectorT<ITrackEntry*>*, void*);
   iptr onDblClick(MUIList*, ITrackEntry*);

protected:
   void		addItem(BPTR parent, const char* name, bool descent, bool donotask);
   void		addISOBuilder();
   void         addTracks();
   void         remTracks();
public:
                              MUITracksDataAudio(ConfigParser *parent, Globals &glb);
                             ~MUITracksDataAudio();
   iptr                      *getObject();
   bool                       start();
   void                       stop();
   const char                *getName();
   void                       update();
};

#endif

