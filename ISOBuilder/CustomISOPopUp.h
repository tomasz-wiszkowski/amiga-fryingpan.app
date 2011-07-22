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


#ifndef _GUIMUI_POPISOELEMENT_H_
#define _GUIMUI_POPISOELEMENT_H_ 

#include <Generic/MUI/MUIPopup.h>
#include <Generic/MUI/MUI.h>
#include <Generic/HookT.h>
#include <Generic/HookAttrT.h>
#include <Generic/String.h>
#include <Generic/Locale.h>
#include <ISOBuilder/ClISO.h>
#include "Globals.h"

using namespace GenNS;

class CustomISOPopUp : public MUIPopup
{
protected:
   enum BtnID
   {
      ID_String      =  100,
      
      ID_NormalName  =  1000,
      ID_ISOName,
      ID_RRName,
      ID_JolietName,
      ID_Comment,

      ID_AddISOEntry =  1100,
      ID_AddRREntry,
      ID_AddJolietEntry,

      ID_ISOLevel    =  2000,
      ID_AddRockRidge,
      ID_AddJoliet,
      ID_RelaxedISO,

      ID_VolumeName  =  2100,
      ID_SystemType,
      ID_VolumeSetName,
      ID_Publisher,
      ID_Preparer,
      ID_Application
   };

protected:
   const Globals& Glb; 
   String	value;
   iptr*	string;
   iptr*	page;

   ClISO*	browser;
   ClElement*	elem;

   HookAttrT<void*, void*>          hCallback;

protected:
   HookT<CustomISOPopUp, int, void*>           hHkBtnHook;

protected:
   virtual bool onOpen();
   virtual bool onClose();
   virtual iptr *getPopDisplay();
   virtual iptr *getPopButton();
   virtual iptr *getPopObject();
   virtual iptr buttonHandler(int id, void* data);

   virtual iptr *getElemSettings();
   virtual iptr *getISOSettings();
public:
                CustomISOPopUp(const Globals& glb);
   virtual      ~CustomISOPopUp();
   virtual void setValue(const void* string);
   virtual const void *getValue();
   virtual iptr *getObject();

   virtual void setBrowser(ClISO *pBser);
   virtual void setElement(ClElement *pElem);

   virtual void setEnabled(bool enabled);

   /* statics */
   static bool	InitLocale(Globals& glb);
};

#endif


