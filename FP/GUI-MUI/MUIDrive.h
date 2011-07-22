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


#ifndef _GUIMUI_MUIDRIVE_H_
#define _GUIMUI_MUIDRIVE_H_

#include <Generic/GenericMUI.h>
#include "MUICommon.h"
#include "MUIPopDevice.h"
#include "MUIPopUnit.h"
#include <Generic/HookT.h>
#include "Globals.h"
#include "../IEngine.h"
#include <Generic/ConfigParser.h>

class DiscSpeed;

using namespace GenNS;

class MUIDrive : public MUICommon
{
protected:
   enum
   {
      ID_DeviceSelect      =  1,
      ID_UnitSelect,
      
      ID_DriveVendor       =  10,
      ID_DriveProduct,
      ID_DriveVersion,

      ID_MechanismType     =  20,
      ID_ReadableMedia,
      ID_WritableMedia,
      ID_SupportedData,
      ID_Burnproof,
      ID_ISRC,
      ID_MCN,
      ID_AudioStream,
      ID_TestMode,
      ID_Multisession,
      ID_CDText,
      ID_ReadSpeeds,
      ID_WriteSpeeds,
      ID_DriveStatus,
      ID_AudioPlayback,
      ID_WriteProtect

   };

protected:
   Globals                   &Glb;
   iptr             *all;
   ConfigParser              *Config;

   iptr             *vendor;
   iptr             *product;
   iptr             *version;

   iptr             *mechanism;
   iptr             *readmedia;
   iptr             *writemedia;
   iptr             *writedata;
   iptr             *burnproof;
   iptr             *isrc;  
   iptr             *mcn;    
   iptr             *audiostream;
   iptr             *testwrite;
   iptr             *multisession;
   iptr             *buffersize; 
   iptr             *cdtext;    
   iptr             *readspeeds; 
   iptr             *writespeeds;
   iptr             *state;          
   iptr            *writeprotect;
   iptr                      *audioplayback;

   MUIPopDevice              *popDevice;
   MUIPopUnit                *popUnit;

   String                     sDevice;
   int                        lUnit;
protected:
   HookT<MUIDrive, int, void*>            hHkButtonHook;
   DbgHandler                *getDebug();

protected:
   virtual iptr      buttonHook(int id, void* data);
   virtual String             mechanismToString(DRT_Mechanism);
   virtual String             mediaToString(iptr);
   virtual String             dataToString(iptr);
   virtual String             speedsToString(DiscSpeed*);

public:

                              MUIDrive(ConfigParser *parent, Globals &glb);
   virtual                   ~MUIDrive();
   virtual bool               start();
   virtual void               update();
   virtual void               stop();
   virtual iptr     *getObject();

};

#endif

