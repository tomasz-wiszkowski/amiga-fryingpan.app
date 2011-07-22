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


#ifndef _ENGINE_JOBS_JOB_H_
#define _ENGINE_JOBS_JOB_H_

#include <Generic/Types.h>
#include <Generic/LibrarySpool.h>
#include <libclass/Optical.h>
#include "../Globals.h"

using namespace GenNS;

class Job 
{
protected:
    Globals	&g;
    iptr	Drive;

public:
    Job(Globals& glb, iptr drv);
    virtual                ~Job();
    virtual void            execute()                  = 0;
    virtual uint32	    getProgress()              = 0;
    virtual const char     *getActionName()            = 0;
    virtual bool            inhibitDOS()               = 0;

    inline DbgHandler*	    getDebug() 
    { 
	return g.getDebug() ;
    }
};

#endif

