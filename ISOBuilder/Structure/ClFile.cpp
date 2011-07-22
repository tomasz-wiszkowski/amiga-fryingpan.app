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
#include "ClFile.h"
#include "ClName.h"
#include <libclass/dos.h>
#include <dos/exall.h>

ClFile::ClFile(ClRoot *pRoot, ClDirectory *pParent, const ExAllData *ed)
   : ClElement(pRoot, pParent)
{
    struct DateStamp hDS;

    fh = 0;
    size = 0;
    transferring = false;
    hDS.ds_Days    = ed->ed_Days;
    hDS.ds_Minute  = ed->ed_Mins;
    hDS.ds_Tick    = ed->ed_Ticks;

    setISOSize(ed->ed_Size);
    setJolietSize(ed->ed_Size);
    setNormalName((char*)ed->ed_Name);
    setComment((char*)ed->ed_Comment);
    setDate((struct DateStamp*)&hDS);
    setProtection(ed->ed_Prot);
}

ClFile::ClFile(ClRoot *pRoot, ClDirectory *pParent, const FileInfoBlock *fib)
   : ClElement(pRoot, pParent)
{
    fh = 0;
    size = 0;
    transferring = false;

    setISOSize(fib->fib_Size);
    setJolietSize(fib->fib_Size);
    setNormalName((char*)fib->fib_FileName);
    setComment((char*)fib->fib_Comment);
    setDate(&fib->fib_Date);
    setProtection(fib->fib_Protection);
}

ClFile::~ClFile()
{
    if (0 != fh)
	DOS->Close(fh);
    fh = 0;
}

bool ClFile::isDirectory()
{
   return false;
}

void ClFile::setISOPosition(unsigned long lPos)
{
   if (isISOEntry())
   {
      ClElement::setISOPosition(lPos);
      if (isJolietEntry())
         ClElement::setJolietPosition(lPos);
   }
}

void ClFile::setISOSize(unsigned long lSize)
{
   if (isISOEntry())
   {
      ClElement::setISOSize(lSize);
      if (isJolietEntry())
         ClElement::setJolietSize(lSize);
   }
}

uint32 ClFile::getData(void* buf, uint32 len)
{
    if ((fh == 0) && (!transferring))
    {
	_d(Lvl_Info, "Transferring: %s", (iptr)getNormalName());
	if (isISOEntry())
	    size = getISOSize();
	else
	    size = getJolietSize();

	if ((~getProtection()) & 8)
	    fh = DOS->Open(getPath().Data(), MODE_OLDFILE);

	transferring = true;
    }
    else if ((size == 0) && transferring)
    {
	_d(Lvl_Info, "Transfer Complete.");
   	if (fh != 0)
	    DOS->Close(fh);
	fh = 0;

	transferring = false;
	return 0;
    }
    
    len = len <? size;
    _d(Lvl_Info, "Transferring, from %08lx, %ld bytes to go, %ld requested", (iptr)fh, size, len);

    if (fh != 0)
	DOS->Read(fh, buf, len);
    else
	memset(buf, 0, len);

    size -= len;
    return len;
}

bool ClFile::setUp()
{
    fh = 0;
    size = 0;
    transferring = false;

    return true;
}

void ClFile::cleanUp()
{
    if (0 != fh)
	DOS->Close(fh);
    fh = 0;
    size = 0;
    transferring = 0;
}
