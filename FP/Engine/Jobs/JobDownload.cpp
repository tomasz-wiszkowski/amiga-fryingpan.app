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


#include "JobDownload.h"

JobDownload::JobDownload(Globals &glb, iptr drive, const IOptItem* item, ISpec *module, const char* name) :
   Job(glb, drive)
{
   this->item     = item;
   this->name     = name;
   this->writer   = 0;
   this->hook     = module;
}

JobDownload::~JobDownload()
{
   item->dispose();
}

void JobDownload::execute()
{
   bool status = false;
   EDtError rc = DT_OK;

   ASSERT(NULL != hook);
   if (NULL == hook)
      return;

   ASSERT(NULL != item)
   if (NULL == item)
      return;

   writer = hook->openWrite(name.Data(), rc);

   if (NULL != writer)
   {
      char *mem = new char[(16 * item->getSectorSize())];

      numBlocks = item->getBlockCount();          // this includes block 0. total number of blocks is always end-start+1
      currBlock = 0;
      if (true == writer->setUp(item))
      {
         while (currBlock < numBlocks)
         {
            int count = 16 <? (numBlocks-currBlock);

            g.Optical->DoMethodA(ARRAY(DRV_ReadTrackRelative, Drive, (int)item, currBlock, count, (int)mem));
            status = writer->writeData(mem, count);
            if (false == status)
               break;

            currBlock += count;
         }
      }

      if (false == status)
      {
         request("Error", "An error occured during download.\nOperation aborted.", "Ok", 0);
      }

      delete [] mem;
      writer->cleanUp();
      writer->dispose();
   }
}

uint32 JobDownload::getProgress()
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

const char *JobDownload::getActionName()
{
   return "Downloading track";
}

bool JobDownload::inhibitDOS()
{
   return false;
}
