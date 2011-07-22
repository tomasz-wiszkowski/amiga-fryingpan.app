/*
 * FryingPan - Amiga CD/DVD Recording Software (User Intnerface and supporting Libraries only)
 * Copyright (C) 2001-2011 Tomasz Wiszkowski Tomasz.Wiszkowski at gmail.com
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "MUIPopup.h"
#include <libclass/intuition.h>
#include <libraries/mui.h>
#include <Generic/LibrarySpool.h>
#include <proto/intuition.h>

MUIPopup::MUIPopup(const char *name)
{
   all = 0;

   this->name = name;
   hHkOpen.Initialize(this, &MUIPopup::doOpen);
   hHkClose.Initialize(this, &MUIPopup::doClose);
}

MUIPopup::~MUIPopup()
{
}

void MUIPopup::update()
{
   hNotify.Call(id, getValue());
}

void MUIPopup::setID(int id)
{
   this->id = id;
}

void MUIPopup::setCallbackHook(const Hook* hook)
{
   hNotify = hook;
}

unsigned long *MUIPopup::getObject()
{
   if (NULL != all)
      return all;

  
   all = HGroup,
   End;

   if (name.Length() != 0)
   {
      DoMtd(all, ARRAY(OM_ADDMEMBER, (int)(label = muiLabel(name, 0, 0, Align_Right))));
   }

   DoMtd(all, ARRAY(OM_ADDMEMBER, (int)(
            popup = PopobjectObject,
               MUIA_Popstring_String,              display = getPopDisplay(),
               MUIA_Popobject_Object,              object  = getPopObject(),
               MUIA_Popstring_Button,              button  = getPopButton(),
               MUIA_Popobject_ObjStrHook,          hHkClose.GetHook(),
               MUIA_Popobject_StrObjHook,          hHkOpen.GetHook(),
            End)));

   return all;
}

unsigned long MUIPopup::doOpen(void* a, void* x)
{
   return onOpen();
}

unsigned long MUIPopup::doClose(void*, void*)
{
   bool bResult;
   bResult = onClose();
   update();
   return bResult;
}

void MUIPopup::open()
{
   DoMtd(all, ARRAY(MUIM_Popstring_Open));
}

void MUIPopup::close()
{
   DoMtd(all, ARRAY(MUIM_Popstring_Close, true));
}



