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
#include "Main.h"
#include "ClISO.h"
#include "ISOStructures.h"
#include <libclass/dos.h>
#include <dos/dos.h>

struct Args
{
   char       *dir;
   char	      *iso;
   iptr	      *start_block;

   static char* Template()
   {
      return "DIR/A,TO/A,STARTBLOCK/N";
   }
};


int main()
{
   /*
   ClISO    *pISO;
   Args      args = {0}; 
   RDArgs   *rda;
   uint8    *buf;

   LibrarySpool::Init();

   rda = DOS->ReadArgs(Args::Template(), (void**)&args, 0);

   if (NULL != rda)
   {
       pISO  = new ClISO();
       buf = new uint8[65536];
       iptr cnt;
       iptr lba = 0;

       if (args.start_block)
	   lba = *args.start_block;

       BPTR fh = DOS->Open(args.iso, MODE_NEWFILE);

       if (fh)
       {
	   pISO->getCurrDir()->scanDirectory(args.dir);
	   cnt = pISO->getBlockCount();
	   DOS->VPrintf("Scan complete. Starting data flow...\n", 0);
	   if (pISO->setUp(lba))
	   {
	       for (;;)
	       {
		   int sz = 32 <? cnt;
		   if (!pISO->readData(0, buf, sz << 11))
		       break;

		   if (DOS->Write(fh, buf, sz << 11) != (sz << 11))
		       break;
	       }
	       pISO->cleanUp();
	   }

	   DOS->Close(fh);
       }

       delete pISO;
       delete [] buf;
       DOS->FreeArgs(rda);
   }

   LibrarySpool::Exit();
   */
   return 0;
}
