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


#ifndef _CLDIRECTORY_H_
#define _CLDIRECTORY_H_

#include "ClElement.h"
#include <Generic/Generic.h>
#include <Generic/VectorT.h>
#include <dos/exall.h>
#include "../RRStructures.h"

class ClRoot;
class ISOPathRecord;
class ISODirRecord;
class ISOWDirRecord;


class ClDirectory : public ClElement
{
   friend class ClRoot;

protected:
   VectorT<ClElement*>           hChildren;
   int32			lParentPathID;
   int32			lISOPathTableEntrySize;
   int32			lJolietPathTableEntrySize;
   RR_SP                         rr_sp;

private:
   // generation only.
   // lISO = ISO size, bytes
   // lRRCE = RR continuation extension size
   uint32	    lISO, lRRCE;
   uint32	    lISOIdx, lRRIdx;

private:
   static bool                   vecDeleteChild(ClElement* const&);
   static int                    vecCompareChildren(ClElement* const&, ClElement* const&);
   static int                    vecCompareISONames(const ClName* const&, const ClName* const&);

protected:
   bool                          internalAddChild(ClElement* elem);

protected:
   virtual void                  initialize(char *sName);
   virtual void                  setJolietPathTableEntrySize(iptr lSize);
   virtual void                  setISOPathTableEntrySize(iptr lSize); 

   virtual uint32	buildISOPathTableEntry(struct ISOPathRecord *pRec, bool bIsMSB);
   virtual uint32	buildJolietPathTableEntry(struct ISOPathRecord *pRec, bool bIsMSB);
   virtual uint32	buildISODirTable(uint8*, uint32);
   virtual uint32	buildJolietDirTable(uint8*, uint32);

   virtual void                  setParentISOEntry(bool bIsISO);
   virtual void                  setParentJolietEntry(bool bIsJoliet);
   virtual void                  setParentRockRidgeEntry(bool bIsRR);

   virtual void                  setParentElementPathID(iptr lID);
   
   virtual bool                  setISOLevel(ClName::Level lvl);
   virtual bool                  setISORelaxed(bool state);

   virtual uint32   getJolietPathTableEntrySize() const;
   virtual uint32   getISOPathTableEntrySize() const; 
   virtual void                  rebuild();                    // basically the iso name
   virtual bool                  recalculate();
   virtual void                  freeISOName();

public:
                        ClDirectory(ClRoot* pRoot, ClDirectory *pParent);
                        ClDirectory(ClRoot* pRoot, ClDirectory *pParent, const ExAllData *ed);
                        ClDirectory(ClRoot* pRoot, ClDirectory *pParent, const FileInfoBlock *fib);
                        ClDirectory(ClRoot* pRoot, ClDirectory *pParent, const String &sName);
   virtual              ~ClDirectory();
   virtual bool         isDirectory();
   virtual bool         addChild(ClElement *aChild);
   virtual void         remChild(ClElement *aChild);
   virtual bool         scanDirectory(const String &sDirectory);
   virtual ClElement*	addChild(const String &sPath);
   virtual bool         update();
   virtual uint32	getChildrenCount() const;
   virtual ClElement*	getChild(int lNum);

   virtual void         setISOEntry(bool bIsISO);
   virtual void         setJolietEntry(bool bIsJoliet);
   virtual void         setRockRidgeEntry(bool bIsRR);
   virtual void         sort();

   virtual bool		setUp();
   virtual void		cleanUp();
   virtual ClDirectory* makeDir(const char* name);
};

#endif //_ISODIRECTORY_H_
