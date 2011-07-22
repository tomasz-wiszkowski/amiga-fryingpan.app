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


#ifndef _ENGINE_JOBS_JOBLAYOUT_H_
#define _ENGINE_JOBS_JOBLAYOUT_H_

#include "Job.h"
#include <DTLib/IData.h>
#include <libdata/Optical/IOptItem.h>
#include <Generic/VectorT.h>
#include <FP/ITrack.h>
#include <Generic/HookT.h>
#include <Generic/RWSyncT.h>

class JobLayout : public Job
{
   IOptItem               *disc;
   IOptItem               *session;
   RWSyncT< VectorT<IData*> > &tracks;
   VectorT<IOptItem*>      items;
   uint32                 &numblocks;
   String                 &operation;   

protected:
   virtual void            analyse(EOpticalError err);
public:
                           JobLayout(Globals &glb, iptr drive, RWSyncT< VectorT<IData*> > &tracks, bool masterize, bool closedisc, String &res, uint32 &size);
   virtual                ~JobLayout();                              
   virtual void            execute();
   virtual uint32	    getProgress();
   virtual const char     *getActionName();
   virtual bool            inhibitDOS();
};

#endif

