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


#ifndef _FP_ENGINE_PLUG_H
#define _FP_ENGINE_PLUG_H 

#include <PlugLib/IPluginT.h>
#include "../IEngine.h"

#define Engine_Name	"engine.plugin"
#define Engine_Version	1
#define Engine_Revision	0

class EnginePlug
{
public:
    virtual IEngine*		GetEngine(int id) = 0;
    virtual const ScanData*	ScanDevice(const char* device) = 0;
    virtual void		FreeScanData(const ScanData* data) = 0;
};

typedef IPluginT<EnginePlug*>    EnginePlugin;

#endif


