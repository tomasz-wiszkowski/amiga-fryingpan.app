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


#include "ClRoot.h"
#include <Generic/LibrarySpool.h>
#include <Generic/HookAttrT.h>
#include <Generic/Debug.h>

void ClRoot::setDebug(DbgHandler* n)
{
    debug = n;
}

DbgHandler *ClRoot::getDebug()
{
    return debug;
}

ClRoot::ClRoot(DbgHandler *dbg)
    : ClDirectory(this, this, "")
{
    setDebug(dbg);

    lFirstSector   = 0;
    setParentElementPathID(1);
    hRootName.setLevel(ClName::ISOLevel_3);
    hRootName.setRelaxed(true);
    hRootName.setShortForm(false);
    initialize("CD-ROM");
}

ClRoot::~ClRoot()
{
}

ClName::Level ClRoot::getISOLevel() const
{
    return (ClElement::getISOLevel());
}

bool ClRoot::setISOLevel(ClName::Level aLevel)
{
    return ClDirectory::setISOLevel(aLevel);
}

bool ClRoot::isISORelaxed() const
{
    return ClElement::isISORelaxed();
}

bool ClRoot::setISORelaxed(bool bRelaxed)
{
    return ClDirectory::setISORelaxed(bRelaxed);
}

iptr ClRoot::getFilesSizeB()
{
    return 0;
}

iptr ClRoot::getISOPathTableSizeB()
{
    if (!isISOEntry())
	return 0;
    if (!lISOPathTablesSize)
	return 1;
    return (lISOPathTablesSize + 2047) >> 11;
}

iptr ClRoot::getISOPathTableSize()
{
    if (!isISOEntry())
	return 0;
    return lISOPathTablesSize;
}

iptr ClRoot::getJolietPathTableSizeB()
{
    if (!isJolietEntry())
	return 0;
    if (!lJolietPathTablesSize)
	return 1;
    return (lJolietPathTablesSize + 2047) >> 11;
}

iptr ClRoot::getJolietPathTableSize()
{
    if (!isJolietEntry())
	return 0;
    return lJolietPathTablesSize;
}

void ClRoot::rebuild()
{
    update();
    _d(Lvl_Info, "Setting disc names to %s / %s", (int)getISOName(), (int)getJolietName());
    hPrimary.setVolumeID(hRootName.getName());
    hSupplementary.setVolumeID(getJolietName());
    ClDirectory::rebuild();
}

bool ClRoot::updateSizes()
{
    uint32 lSector  =  lFirstSector + 16;      // include pad data
    uint32 lCur;

    _dx(Lvl_Info, "Updating sizes. First sector=%ld", lFirstSector);

    /* create two vectors: hDirVec contains flat dir tree, hFileVec contains all files */
    {

	hDirVec.Empty();
	hFileVec.Empty();

	hDirVec << this;     // :) yea i finally got the idea :P

	for (lCur = 0; lCur < hDirVec.Count(); lCur++)
	{
	    ClDirectory *pDir = hDirVec[lCur];
	    ClDirectory *pElem;
	    ClElement   *pEl;

	    _dx(Lvl_Info, "Queue dir %s", (iptr)pDir->getNormalName());
	    ASSERT(pDir != 0);
	    for (uint32 i=0; i<pDir->getChildrenCount(); i++)
	    {
		pEl   = pDir->getChild(i);

		if (pEl->isDirectory())
		{
		    pElem = static_cast<ClDirectory*>(pEl);
		    pElem->setParentElementPathID(lCur+1);
		    hDirVec << pElem;
		}
		else
		{
		    hFileVec << static_cast<ClFile*>(pEl);
		}
	    }         
	}
    }

    /* rebuild all directories so we have proper names */
    {  
	_d(Lvl_Debug, "Number of directories: %ld", hDirVec.Count());
	for (lCur = 0; lCur < hDirVec.Count(); lCur++)
	{
	    _d(Lvl_Info, "--> Rebuilding directory %s", (int)hDirVec[lCur]->getNormalName());
	    hDirVec[lCur]->rebuild();
	}

	_d(Lvl_Debug, "Rebuild complete");
    }

    /* as we have the names etc here, recalculate all structures */
    {  

	for (lCur = 0; lCur < hDirVec.Count(); lCur++)
	{
	    _d(Lvl_Info, "--> Recalculating directory %s", (int)hDirVec[lCur]->getNormalName());
	    hDirVec[lCur]->recalculate();
	}

	_d(Lvl_Debug, "Recalculation complete");
    }

    /* calculate path tables size */
    {  
	lISOPathTablesSize      = 0;
	lJolietPathTablesSize   = 0;

	for (lCur = 0; lCur < hDirVec.Count(); lCur++)
	{
	    _d(Lvl_Info, "Calcing PT sizes for %s", (int)hDirVec[lCur]->getNormalName());
	    lISOPathTablesSize      += hDirVec[lCur]->getISOPathTableEntrySize();
	    lJolietPathTablesSize   += hDirVec[lCur]->getJolietPathTableEntrySize();
	}
    }

    /* we can start the finals now :) */

    /* root sectors: primary, supplementary and end descriptors */
    {  

	// if we are meant to include ISO root sector...
	if (isISOEntry())
	{
	    ++lSector;
	}

	// or joliet root sector
	if (isJolietEntry())
	{
	    ++lSector;
	}

	// and pad sector :)
	++lSector;
    }

    /* path tables: two for iso, two for joliet */
    {  
	hPrimary.setLSBPathTablePosition(lSector);
	lSector += (lISOPathTablesSize    + 2047) >> 11;
	hPrimary.setMSBPathTablePosition(lSector);
	lSector += (lISOPathTablesSize    + 2047) >> 11;

	hSupplementary.setLSBPathTablePosition(lSector);
	lSector += (lJolietPathTablesSize + 2047) >> 11;
	hSupplementary.setMSBPathTablePosition(lSector);
	lSector += (lJolietPathTablesSize + 2047) >> 11;
    }

    /* relocate iso and joliet dir tables */
    {  
	for (lCur=0; lCur<hDirVec.Count(); lCur++)
	{
	    hDirVec[lCur]->setISOPosition(lSector);
	    lSector += hDirVec[lCur]->getISOSize() >> 11;
	}

	for (lCur=0; lCur<hDirVec.Count(); lCur++)
	{
	    hDirVec[lCur]->setJolietPosition(lSector);
	    lSector += hDirVec[lCur]->getJolietSize() >> 11;
	}
    }

    /* relocate all files */
    {  
	for (lCur=0; lCur<hFileVec.Count(); lCur++)
	{
	    hFileVec[lCur]->setISOPosition(lSector);
	    if (hFileVec[lCur]->getISOSize() > 0)
		lSector += (hFileVec[lCur]->getISOSize()+2047) >> 11;
	    else
		lSector ++;
	}
    }

    /* mark image size & finalize */
    {  
	lPadSize   = lSector;
	lTotalSize = (lSector + 3); // rounding will be done on demand
	lPadSize   = lTotalSize - lPadSize;
	hPrimary.setVolumeSize(lTotalSize);
	hPrimary.setPathTableSize(lISOPathTablesSize);
	hPrimary.setDate(&getDate());

	hSupplementary.setVolumeSize(lTotalSize);
	hSupplementary.setPathTableSize(lJolietPathTablesSize);
	hSupplementary.setDate(&getDate());
    }

    _d(Lvl_Debug, "Estimated sizes:\n"
	    "- ISO Path Tables Size:         %ld bytes\n"
	    "- Joliet Path Tables Size:      %ld bytes\n"
	    "- ISO Dir Tables Size:          %ld bytes\n"
	    "- Joliet Dir Tables Size:       %ld bytes",
	    lISOPathTablesSize,
	    lJolietPathTablesSize,
	    getISOSize(),
	    getJolietSize());



    return true;
}

bool ClRoot::setUp()
{
    _dx(Lvl_Error, "We shouldn't be here!");
    return false;
}

bool ClRoot::setUp(iptr block)
{
    _d(Lvl_Info, "Initializing generation for block %ld", block);
    lFirstSector = block;
    lGenParam1 = 0;
    lGenParam2 = 0;
    pGenTemp = new uint8[2048];
    eGenType = Gen_EntryPad;

    updateSizes();

    return ClDirectory::setUp();
}

void ClRoot::cleanUp()
{
    delete [] pGenTemp;
    pGenTemp = 0;

    ClDirectory::cleanUp();
}

iptr ClRoot::generate(uint8* mem, iptr len)
{
    bool stat;
    pGenBuff = mem;
    lGenMax = len;
    lGenPos = 0;

    if (eGenType == Gen_Done)
	return 0;

    _d(Lvl_Info, "Current GenType: %ld", eGenType);

    while (lGenPos < lGenMax)
    {
	switch (eGenType)
	{
	    case Gen_EntryPad:		stat = genPad(false);		break;
	    case Gen_Headers:		stat = genHeaders();		break;
	    case Gen_LEISOTrees:	stat = genISOTrees(false);	break;
	    case Gen_BEISOTrees:	stat = genISOTrees(true);	break;
	    case Gen_LEJolietTrees:	stat = genJolietTrees(false);   break;
	    case Gen_BEJolietTrees:	stat = genJolietTrees(true);    break;
	    case Gen_ISODirectories:	stat = genISODirectories();	break;
	    case Gen_JolietDirectories:	stat = genJolietDirectories();	break;
	    case Gen_Files:		stat = genFiles();		break;
	    case Gen_ClosingPad:	stat = genPad(true);		break;
	    case Gen_Done:		stat = true;			break;
	    default:			stat = false;			break;
	}

	// rare event
	if (!stat)
	{
	    lGenParam1 = 0;
	    lGenParam2 = 0;
	    eGenType = (enum Generate)((int)eGenType + 1);
	}

	// possible event
	if (eGenType == Gen_Done)
	    break;
    }

    return lGenPos;
}

bool ClRoot::genPad(bool closing)
{
    if (!closing)
    {
	// here we can assume we always start with empty buffer.
	_d(Lvl_Info, "Writing 16 pad sectors");
	ASSERTS((lGenMax & ~32767), "BUFFER SIZE MUST BE 32768*n!");
        memset(pGenBuff, 0, 16 * 2048); 
	lGenPos = 16 * 2048;
    }
    else
    {
	_d(Lvl_Info, "Writing closing pad sectors");
	memset(&pGenBuff[lGenPos], 0, lGenMax - lGenPos);
	lGenPos = lGenMax;
    }
    return false;
}

bool ClRoot::genHeaders()
{
    ISOEndDescriptor  hDesc;
    // if we entered here, we have at least 2048 bytes available.
    // actually loop here is most likely pointless. we start right after
    // 16 pad sectors and we REQUIRE buffer to be n*16s in length,
    // so either way we have at least 16 spare sectors available.
    if (isISOEntry())
    {
	_d(Lvl_Debug, "Writing primary volume descriptor");
	Exec->CopyMemQuick(hPrimary.getData(), &pGenBuff[lGenPos], 2048);
    }
    lGenPos += 2048;

    if (isJolietEntry())
    {
	_d(Lvl_Debug, "Writing supplementary volume descriptor");
	Exec->CopyMemQuick(&hSupplementary, &pGenBuff[lGenPos], 2048);
    }
    lGenPos += 2048;

    _d(Lvl_Debug, "Writing closing descriptor");
    Exec->CopyMemQuick(&hDesc, &pGenBuff[lGenPos], 2048);
    lGenPos += 2048;

    return false;
}
    
bool ClRoot::genISOTrees(bool be)
{
    uint32 len;

    if (isISOEntry())
    {
	_d(Lvl_Debug, "Generating Path Tables");

	/*
	 * entering this function for the first time, lGenParam1 is 0
	 * so the following loop should be safe.
	 */
	if (lGenParam2 > 0)
	{
	    _d(Lvl_Info, "Attempting partial path table to target buffer");
	    Exec->CopyMem(pGenTemp, &pGenBuff[lGenPos], lGenParam2);
	    lGenPos += lGenParam2;
	    lGenParam2 = 0;
	}

	for (; lGenParam1<hDirVec.Count(); lGenParam1++)
	{
	    if (!hDirVec[lGenParam1]->isJolietEntry())
		continue;
	    /*
	     * path tables: entry size won't go beyond 128 here 
	     * due to amigados name length limit. that's so good ;-)
	     *
	     * most likely will stay around 32-40 bytes.
	     */
	    if ((lGenMax - lGenPos < 128) &&
	       ((lGenMax - lGenPos) < (len = hDirVec[lGenParam1]->getISOPathTableEntrySize())))
	    {
		_d(Lvl_Info, "Path table won't fit entirely in target buffer");
		hDirVec[lGenParam1]->buildISOPathTableEntry((ISOPathRecord*)pGenTemp, be);
		// stuff partially the target buffer
		Exec->CopyMem(pGenTemp, &pGenBuff[lGenPos], lGenMax-lGenPos);
		// copy remainder so we can access it at another call
		lGenParam2 = len - (lGenMax - lGenPos);
		Exec->CopyMem(&pGenTemp[len - lGenParam2], pGenTemp, lGenParam2);
		lGenPos = lGenMax;
		// since we're here, the buffer is full!
		break;
	    }
	    else
	    {
		lGenPos += hDirVec[lGenParam1]->buildISOPathTableEntry((ISOPathRecord*)&pGenBuff[lGenPos], be);
	    }
	}
    }

    if ((lGenParam2 != 0) || (lGenParam1 != hDirVec.Count()))
	return true;

    memset(&pGenBuff[lGenPos], 0, (lGenMax - lGenPos) & 2047);
    lGenPos = (lGenPos + 2047) & ~2047;
    return false;
}

bool ClRoot::genJolietTrees(bool be)
{
    iptr len;

    if (isJolietEntry())
    {
	_d(Lvl_Debug, "Generating Path Tables [Joliet]");

	/*
	 * entering this function for the first time, lGenParam1 is 0
	 * so the following loop should be safe.
	 */
	if (lGenParam2 > 0)
	{
	    _d(Lvl_Info, "Attempting partial path table (%ldb) to target buffer", lGenParam2);
	    Exec->CopyMem(pGenTemp, &pGenBuff[lGenPos], lGenParam2);
	    lGenPos += lGenParam2;
	    lGenParam2 = 0;
	}

	for (; lGenParam1<hDirVec.Count(); lGenParam1++)
	{
	    if (!hDirVec[lGenParam1]->isJolietEntry())
		continue;

	    /*
	     * path tables: entry size won't go beyond 248 here 
	     * due to amigados name length limit. that's so good ;-)
	     *
	     * most likely will stay around 64-80 bytes.
	     */
	    if ((lGenMax - lGenPos < 248) &&
	       ((lGenMax - lGenPos) < (len = hDirVec[lGenParam1]->getJolietPathTableEntrySize())))
	    {
		_d(Lvl_Info, "Path table won't fit entirely in target buffer");
		hDirVec[lGenParam1]->buildJolietPathTableEntry((ISOPathRecord*)pGenTemp, be);
		// stuff partially the target buffer
		Exec->CopyMem(pGenTemp, &pGenBuff[lGenPos], lGenMax-lGenPos);
		// copy remainder so we can access it at another call
		lGenParam2 = len - (lGenMax - lGenPos);
		Exec->CopyMem(&pGenTemp[len - lGenParam2], pGenTemp, lGenParam2);
		lGenPos = lGenMax;
		// since we're here, the buffer is full!
		break;
	    }
	    else
	    {
		lGenPos += hDirVec[lGenParam1]->buildJolietPathTableEntry((ISOPathRecord*)&pGenBuff[lGenPos], be);
	    }
	}
    }

    if ((lGenParam2 != 0) || (lGenParam1 != hDirVec.Count()))
	return true;

    memset(&pGenBuff[lGenPos], 0, (lGenMax - lGenPos) & 2047);
    lGenPos = (lGenPos + 2047) & ~2047;
    return false;
}

bool ClRoot::genISODirectories()
{
    for (; lGenParam1 < hDirVec.Count(); lGenParam1++)
    {
	if (lGenPos == lGenMax)
	    break;

	// one block at a time
	_d(Lvl_Info, "  ISO >> %ld", lGenPos);
	while ((lGenParam2 = hDirVec[lGenParam1]->buildISODirTable(&pGenBuff[lGenPos], lGenParam2)) != 0)
	{
	    lGenPos += 2048;

	    // we're not done yet, but buffer is full
	    if (lGenPos == lGenMax)
		return true;
	    _d(Lvl_Info, "  ISO >> %ld", lGenPos);
	}
	lGenPos += 2048;
    }

    return !((lGenParam1 == hDirVec.Count()) && (lGenParam2 == 0));
}

bool ClRoot::genJolietDirectories()
{
    for (; lGenParam1 < hDirVec.Count(); lGenParam1++)
    {
	if (lGenPos == lGenMax)
	    break;
	// one block at a time
	_d(Lvl_Info, "  >> %ld", lGenPos);
	while ((lGenParam2 = hDirVec[lGenParam1]->buildJolietDirTable(&pGenBuff[lGenPos], lGenParam2)) != 0)
	{
	    lGenPos += 2048;

	    // we're not done yet, but buffer is full
	    if (lGenPos == lGenMax)
		return true;
	    _d(Lvl_Info, "  >> %ld", lGenPos);
	}
	lGenPos += 2048;
    }

    return !((lGenParam1 == hDirVec.Count()) && (lGenParam2 == 0));
}

bool ClRoot::genFiles()
{
    iptr sz;
    for (; lGenParam1 < hDirVec.Count(); lGenParam1++)
    {
	if (lGenPos == lGenMax)
	    return true;

	for (; lGenParam2 < hDirVec[lGenParam1]->getChildrenCount(); lGenParam2++)
	{
	    ClElement *e = hDirVec[lGenParam1]->getChild(lGenParam2);

	    if (e->isDirectory())
		continue;

	    sz = ((ClFile*)e)->getData(&pGenBuff[lGenPos], lGenMax - lGenPos);
	    sz = (sz + 2047) &~ 2047;
	    lGenPos += sz;

	    // file not done just yet => filled buffer.
	    if (lGenPos == lGenMax)
		return true;

	    if (sz == 0)
		continue;

	    sz = ((ClFile*)e)->getData(pGenBuff, lGenMax);
	    ASSERT(sz == 0);
	}

	lGenParam2 = 0;
    }

    _d(Lvl_Info, "All files transferred");
    return false;
}

const VectorT<ClDirectory*> &ClRoot::getDirVector() const
{
    return hDirVec;
}

const VectorT<ClFile*> &ClRoot::getFileVector() const
{
    return hFileVec;
}

void ClRoot::setISOSize(iptr lSize)
{
    ClDirectory::setISOSize(lSize);
    hPrimary.setRootSize(lSize);
}

void ClRoot::setISOPosition(iptr lPosition)
{
    ClDirectory::setISOPosition(lPosition);
    hPrimary.setRootPosition(lPosition);
}

void ClRoot::setJolietSize(iptr lSize)
{
    ClDirectory::setJolietSize(lSize);
    hSupplementary.setRootSize(lSize);
}

void ClRoot::setJolietPosition(iptr lPosition)
{
    ClDirectory::setJolietPosition(lPosition);
    hSupplementary.setRootPosition(lPosition);
}

uint32 ClRoot::buildISOPathTableEntry(ISOPathRecord *pRec, bool bIsMSB)
{
    if (!isISOEntry())
	return 0;

    long lLen = getISOPathTableEntrySize();

    pRec->setExtent(getISOPosition(), bIsMSB);
    pRec->setParent(lParentPathID, bIsMSB);
    pRec->setISOName(0);
    return lLen;
}

uint32 ClRoot::buildJolietPathTableEntry(ISOPathRecord *pRec, bool bIsMSB)
{
    if (!isJolietEntry())
	return 0;

    long lLen = getJolietPathTableEntrySize();

    pRec->setExtent(getJolietPosition(), bIsMSB);
    pRec->setParent(lParentPathID, bIsMSB);
    pRec->setJolietName(0);
    return lLen;
}

uint32 ClRoot::getJolietPathTableEntrySize() const
{
    if (!isJolietEntry())
	return 0;

    return sizeof(ISOPathRecord);
}

uint32 ClRoot::getISOPathTableEntrySize() const
{
    if (!isISOEntry())
	return 0;


    return sizeof(ISOPathRecord);
}

/* own functions...? */

const char *ClRoot::getVolumeID()
{
    return hPrimary.getVolumeID();
}

const char *ClRoot::getVolumeSetID()
{
    return hPrimary.getVolumeSetID();
}

const char *ClRoot::getSystemID()
{
    return hPrimary.getSystemID();
}

const char *ClRoot::getPreparerID()
{
    return hPrimary.getPreparerID();
}

const char *ClRoot::getPublisherID()
{
    return hPrimary.getPublisherID();
}

const char *ClRoot::getApplicationID()
{
    return hPrimary.getApplicationID();
}

void ClRoot::setVolumeID(const char* id)
{
    setNormalName(id);
    setJolietName(id);
    hPrimary.setVolumeID(id);
    hSupplementary.setVolumeID(id);
}

void ClRoot::setVolumeSetID(const char* id)
{
    hPrimary.setVolumeSetID(id);
    hSupplementary.setVolumeSetID(id);
}

void ClRoot::setSystemID(const char* id)
{
    hPrimary.setSystemID(id);
    hSupplementary.setSystemID(id);
}

void ClRoot::setPreparerID(const char* id)
{
    hPrimary.setPreparerID(id);
    hSupplementary.setPreparerID(id);
}

void ClRoot::setPublisherID(const char* id)
{
    hPrimary.setPublisherID(id);
    hSupplementary.setPublisherID(id);
}

void ClRoot::setApplicationID(const char* id)
{
    hPrimary.setApplicationID(id);
    hSupplementary.setApplicationID(id);
}

uint32 ClRoot::getImageSize()
{
#warning fix this!!
    _dx(Lvl_Info, "Speed me up PLEASE");
    updateSizes();
    _dx(Lvl_Info, "Image size: %ld", lTotalSize);
    return lTotalSize;
}

void ClRoot::setNormalName(const char *name)
{
    ClDirectory::setNormalName(name);
    hRootName.setFullName(name);
    hRootName.update();
    hPrimary.setVolumeID(hRootName.getName());
}

const char *ClRoot::getNormalName() const
{
    return hRootName.getName();
}
