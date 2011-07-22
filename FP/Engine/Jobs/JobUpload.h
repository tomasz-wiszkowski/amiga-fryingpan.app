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


#ifndef _ENGINE_JOBS_JOBUPLOAD_H_
#define _ENGINE_JOBS_JOBUPLOAD_H_

#include "Job.h"
#include <DTLib/IData.h>
#include <DTLib/ISpec.h>
#include <libdata/Optical/IOptItem.h>
#include <Generic/VectorT.h>
#include <FP/ITrack.h>
#include <Generic/HookT.h>
#include <Generic/RWSyncT.h>

class JobUpload : public Job
{
private:
    struct Pair
    {
	IData*	    fr;
	IOptItem*   oi;
    };

private:
   IOptItem               *disc;
   IOptItem               *session;
   RWSyncT< VectorT<IData*> > &tracks;
   VectorT<IData*>	    &vec;
   VectorT<Pair>	    items;
   uint64		    numBlocks;
   uint64		    currBlock;
   unsigned short          currTrack;
   bool                    finalize;
   bool                    masterize;
   String                  operation;

protected:
   void onError(EOpticalError e);
public:
                           JobUpload(Globals& glb, iptr drive, RWSyncT< VectorT<IData*> > &tracks, bool masterize, bool closedisc);
   virtual                ~JobUpload();                              
   virtual void            execute();
   virtual uint32	   getProgress();
   virtual const char     *getActionName();
   virtual bool            inhibitDOS();
};

#endif

