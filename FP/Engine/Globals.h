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


#ifndef _ENGINE_GLOBALS_H_
#define _ENGINE_GLOBALS_H_

#include <Generic/DynList.h>
#include <Optical/Optical.h>
#include <Generic/Configuration.h>
#include <Generic/Debug.h>


using namespace GenNS;

struct IRegHook;
class Engine;

class Globals
{
public:
    /*
    ** auto initialized
    */
    Configuration	    Config;
    VectorT<Engine*>	    Engines;

    /*
    ** loaded, external
    */
    OpticalPlugin	    &Optical;
    DbgHandler		    *dbg;

    /*
    ** functions
    */
    Globals(struct PlugLibIFace* plug);
    ~Globals();
    void Update();
    DbgHandler* getDebug() const;
    void setDebug(DbgHandler* h); 
};
   

#endif

