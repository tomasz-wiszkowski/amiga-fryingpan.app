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


#include "JobCreateISO.h"

JobCreateISO::JobCreateISO(Globals &glb, uint32 drive, IData *browser, ISpec *module, const char* name) :
   Job(glb, drive)
{
   this->browser  = browser;
   this->name     = name;
   this->writer   = 0;
   this->hook     = module;
}

JobCreateISO::~JobCreateISO()
{
}

void JobCreateISO::execute()
{
    EDtError rc = DT_OK;

    ASSERT(NULL != hook);
    if (NULL == hook)
	return;

    ASSERT(NULL != browser)
	if (NULL == browser)
	    return;

    writer = hook->openWrite(name.Data(), rc);

    if (NULL != writer)
    {
	currBlock = 0;
	writer->setUp(0);
	browser->setUp(0);
	iptr seccnt = numBlocks;
	uint8 *buf = new uint8[16 * 2048];

	while (seccnt != 0)
	{
	    browser->readData(buf, 16);
	    writer->writeData(buf, 16);

	    currBlock += seccnt <? 16;
	    seccnt -= seccnt <? 16;
	}

	writer->cleanUp();
	writer->dispose();
	browser->cleanUp();
	delete [] buf;
    }
}

uint32 JobCreateISO::getProgress()
{
   uint64 s1, s2;

   s2 = numBlocks;
   s1 = currBlock;
   while (s2 > 65536)
   {
      s2 >>= 1;
      s1 >>= 1;
   }

   return ((uint32)s1 * 65535) / (uint32)s2;
}

const char *JobCreateISO::getActionName()
{
   return "Creating ISO Image";
}

bool JobCreateISO::inhibitDOS()
{
   return false;
}
