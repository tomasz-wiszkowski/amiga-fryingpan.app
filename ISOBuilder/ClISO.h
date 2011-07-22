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


#ifndef CLISO_H_
#define CLISO_H_

#include "Structure/ClRoot.h"
#include "ISOStructures.h"
#include "Structure/ClRoot.h"
#include <Generic/HookT.h>
#include <Generic/VectorT.h>
#include <Generic/Debug.h>
#include <Optical/IOptItem.h>
#include <Generic/MUI/MUI.h>
#include "Globals.h"
#include <DTLib/IData.h>

using namespace GenNS;
class CustomISOConfigPage;


class ClISO : public IData
{
public:
   enum ISOLevel
   {
      ISOLevel_1,
      ISOLevel_2,
      ISOLevel_3
   };


protected:
    DbgHandler       *debug;
    const Globals&   Glb;

public:
    void             setDebug(DbgHandler*);
    DbgHandler       *getDebug();

protected:

    ClRoot           *pRoot;
    ClDirectory      *pCurrDir;
    bool             bAbortDataFlow;
    void             *pMemBlk;
    iptr		    lMemBlkSize;
    iptr		    lCurrentPos;
    MUI		    mui;

protected:
    iptr*	    label;
    CustomISOConfigPage*	page;

public:
    ClISO(const Globals &g);
    virtual                     ~ClISO();

    virtual ClRoot              *getRoot();
    virtual ClDirectory         *getParent();
    virtual ClDirectory         *getCurrDir();
    virtual void                 goRoot();
    virtual void                 goParent();
    virtual ClDirectory         *makeDir();
    virtual void        setCurrDir(ClDirectory*);
    virtual iptr	validate();

    virtual void        setISOLevel(ISOLevel);
    virtual ISOLevel    getISOLevel();

    virtual bool	setUp(iptr start_block);
    virtual bool	setUp(const IOptItem* trk);
    virtual void	cleanUp();
    virtual bool	readData(void* buf, int len);
    virtual void	dispose();
    virtual uint32	getBlockCount() const;
    virtual bool        fillOptItem(IOptItem *item) const;

    virtual iptr*       getSettingsPage() const;

    /** all stuff that goes inline
    */
    virtual const char* getName() const
    {
	return "ISO Constructor";
    }

    virtual const char* getTrackName() const
    {
	return "ISO Constructor";
    }

    virtual bool	writeData(void* pBuff, int pLen)
    {
	return false;
    }

    virtual uint16	getBlockSize() const
    {
	return 2048;
    }

    virtual bool	isAudio() const
    {
	return false;
    }

    virtual bool	isData() const
    {
	return true;
    }
};

#endif /*CLISO_H_*/
