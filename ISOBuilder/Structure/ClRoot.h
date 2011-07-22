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


#ifndef _CLROOT_H_
#define _CLROOT_H_

#include "ClDirectory.h"
#include "ClFile.h"
#include "ClName.h"
#include "../ISOStructures.h"

class ClRoot : public ClDirectory
{
public:
    enum Level
    {
	ISOLevel_1 = 1,
	ISOLevel_2,
	ISOLevel_3,
    };

protected:
    class DbgHandler*	debug;

public:
    void                setDebug(DbgHandler*);
    virtual DbgHandler*	getDebug();

protected:
    iptr                 lFirstSector;

    VectorT<ClDirectory*>		hDirVec;
    VectorT<ClFile*>			hFileVec;
    ISOPrimaryVolumeDescriptor		hPrimary;
    ISOSupplementaryVolumeDescriptor	hSupplementary;

    ClName               hRootName;

    iptr                 lISOPathTablesSize;
    iptr                 lJolietPathTablesSize;
    iptr                 lPadSize;
    iptr                 lTotalSize;

protected: // generate
    enum Generate
    {
	Gen_EntryPad,
	Gen_Headers,
	Gen_LEISOTrees,
	Gen_BEISOTrees,
	Gen_LEJolietTrees,
	Gen_BEJolietTrees,
	Gen_ISODirectories,
	Gen_JolietDirectories,
	Gen_Files,
	Gen_ClosingPad,
	Gen_Done
    } eGenType;

    uint8*		pGenBuff;   // gen buffer
    uint32		lGenPos;    // gen buffer offset
    uint32		lGenMax;    // max buffer max length
    uint32		lGenParam1, // function specific
			lGenParam2; // function specific
    uint8*		pGenTemp;   // temp buffer, 2048b long

protected: // generate
    bool	genPad(bool closing);
    bool	genHeaders();
    bool	genISOTrees(bool be);
    bool	genJolietTrees(bool be);
    bool	genISODirectories();
    bool	genJolietDirectories();
    bool	genFiles();

protected:

    virtual iptr         getFilesSizeB();           // blocks
    virtual iptr         getISOPathTableSizeB();    // blocks
    virtual iptr         getISOPathTableSize();     // bytes
    virtual iptr         getJolietPathTableSizeB(); // blocks
    virtual iptr         getJolietPathTableSize();  // bytes

    virtual void         setISOSize(iptr lSize);
    virtual void         setISOPosition(iptr lPosition);
    virtual void         setJolietSize(iptr lSize);
    virtual void         setJolietPosition(iptr lPosition);

    virtual uint32  getJolietPathTableEntrySize() const;
    virtual uint32  getISOPathTableEntrySize() const; 
    virtual uint32  buildISOPathTableEntry(struct ISOPathRecord *pRec, bool bIsMSB);
    virtual uint32  buildJolietPathTableEntry(struct ISOPathRecord *pRec, bool bIsMSB);

    // hide this one. we need block!
    virtual bool	setUp();

public:
    ClRoot(DbgHandler *debug);
    virtual                            ~ClRoot();

    virtual ClName::Level               getISOLevel() const;
    virtual bool                        setISOLevel(ClName::Level aLevel);
    virtual bool                        isISORelaxed() const;
    virtual bool                        setISORelaxed(bool bRelaxed);

    bool	    setUp(iptr block);
    iptr	    generate(uint8* b, iptr l);
    virtual void    cleanUp();
    //virtual bool                        generate(const Hook *data);
    virtual void                        rebuild();
    virtual bool                        updateSizes();             // just internally

    virtual const VectorT<ClDirectory*>&getDirVector() const;
    virtual const VectorT<ClFile*>     &getFileVector() const;

    //
    //
    //

    virtual const char                 *getVolumeID();
    virtual const char                 *getVolumeSetID();
    virtual const char                 *getSystemID();
    virtual const char                 *getPreparerID();
    virtual const char                 *getPublisherID();
    virtual const char                 *getApplicationID();

    virtual void                        setVolumeID(const char* id);
    virtual void                        setVolumeSetID(const char* id);
    virtual void                        setSystemID(const char* id);
    virtual void                        setPreparerID(const char* id);
    virtual void                        setPublisherID(const char* id);
    virtual void                        setApplicationID(const char* id);

    virtual void                        setNormalName(const char*);
    virtual const char                 *getNormalName() const;

    virtual uint32                      getImageSize();
};

#endif //_ISOROOT_H_
