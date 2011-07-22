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


#include <Generic/LibrarySpool.h>
#include "ClISO.h"
#include "Structure/ClDirectory.h"
#include "Structure/ClRoot.h"
#include "Structure/ClFile.h"
#include "ISOStructures.h"
#include <libclass/utility.h>
#include <libclass/exec.h>
#include <libclass/dos.h>
#include <Generic/Debug.h>
#include "CustomISOConfigPage.h"

extern UtilityIFace *Utility;

/*
 * TODO:
 * 1) allocate 512b temporary buffer size
 * 2) create as much data in primary buffer as possible
 * 3) create a temporaty data in temporary buffer and fill in primary buffer
 * 4) io
 * 5) copy remaining data from temporary buffer back to primary 
 * 6) goto 2 while we still have something to generate
 * 7) dispose buffer
 */

void ClISO::setDebug(DbgHandler* d)
{
   debug = d;
}

DbgHandler *ClISO::getDebug()
{
   return debug;
}

ClISO::ClISO(const Globals &glb) :
    Glb(glb)
{
   debug = 0;
   _createDebug(1, "ISOBuilder");
   pRoot          = new ClRoot(getDebug());
   pCurrDir       = pRoot;
   label	  = mui.muiLabel("To be implemented");
   page	    = new CustomISOConfigPage(this, Glb);
}

ClISO::~ClISO()
{
    MUIMaster->MUI_DisposeObject(label);
    delete page;
    delete pRoot;
    _destroyDebug();
}

ClRoot *ClISO::getRoot() 
{
   return pRoot;
}

ClDirectory *ClISO::getParent() 
{
   return pCurrDir->getParent();
}

ClDirectory *ClISO::getCurrDir() 
{
   return pCurrDir;
}

void ClISO::goRoot() 
{
   pCurrDir = pRoot;
}

void ClISO::goParent()
{
   pCurrDir = pCurrDir->getParent();
}

unsigned long ClISO::validate()
{
    pRoot->updateSizes();
    return pRoot->getImageSize();
}

void ClISO::dispose()
{
   delete this;
}

ClDirectory *ClISO::makeDir()
{
   ClDirectory* d = new ClDirectory(getRoot(), getCurrDir());
   getCurrDir()->addChild(d);   
   return d;
}

void ClISO::setCurrDir(ClDirectory* dir)
{
   if (dir == 0)
   {
      pCurrDir = pRoot;
   }
   else
   {
      pCurrDir = dir;
   }
}

void ClISO::setISOLevel(ISOLevel l)
{
   switch (l)
   {
      case ISOLevel_1:
         pRoot->setISOLevel(ClName::ISOLevel_1);
         break;

      case ISOLevel_2:
         pRoot->setISOLevel(ClName::ISOLevel_2);
         break;

      case ISOLevel_3:
         pRoot->setISOLevel(ClName::ISOLevel_3);
         break;
   }
}

ClISO::ISOLevel ClISO::getISOLevel()
{
   switch (pRoot->getISOLevel())
   {
      case ClName::ISOLevel_1:
         return ISOLevel_1;
         break;

      case ClName::ISOLevel_2:
         return ISOLevel_2;
         break;

      case ClName::ISOLevel_3:
         return ISOLevel_3;
         break;
   }
   return ISOLevel_1;
}

bool ClISO::setUp(iptr block)
{
    _d(Lvl_Info, "Called setUp with block=%ld", block);
    bool res = pRoot->setUp(block);
    if (!res)
    {
	_d(Lvl_Error, "FAILED TO SETUP ROOT!");
	pRoot->cleanUp();
    }
    return res;
}
   
bool ClISO::setUp(const IOptItem* trk)
{
    _dx(Lvl_Info, "new setup with trk=%08lx, start=%ld called.", (iptr)trk, trk->getStartAddress());
    if (trk != 0)
	return setUp(trk->getStartAddress());
    return setUp((iptr)0);
}

void ClISO::cleanUp()
{
    pRoot->cleanUp();
}

bool ClISO::readData(void* buf, int len)
{
    len <<= 11;
    return pRoot->generate((uint8*)buf, len);
}

uint32 ClISO::getBlockCount() const
{
    //_dx(Lvl_Info, "Checking Root image size");
    return pRoot->getImageSize();
}

bool ClISO::fillOptItem(IOptItem *item) const
{
    FAIL(item->getItemType() != Item_Session, "Invalid type of parameter, expecting session")
	return false;

    IOptItem* n = item->addChild();

#warning mozemy sobie zachowac ten wskaznik na potem.
    //_dx(Lvl_Info, "Configuring item");
    n->setDataType(Data_Mode1);
    n->setDataBlockCount(getBlockCount());
    n->setSectorSize(2048);
#warning czy powinienem wypelniac wiecej?
    return true;
}

iptr* ClISO::getSettingsPage() const
{
    return page->getObject();
}
