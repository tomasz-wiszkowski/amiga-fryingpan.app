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


#include "JobMediumAction.h"
#include <libdata/Optical/IOptItem.h>

JobMediumAction::JobMediumAction(Globals &glb, iptr drive, JobMediumAction::Action action) :
   Job(glb, drive)
{
   eAction = action;
}

void JobMediumAction::execute()
{
   switch (eAction)
   {
      case Act_QuickErase:
         {
            g.Optical->DoMethodA(ARRAY(DRV_Blank, Drive, DRT_Blank_Fast));
         }
         break;

      case Act_QuickFormat:
         {
            g.Optical->DoMethodA(ARRAY(DRV_Format, Drive, DRT_Format_Fast));
         }
         break;

      case Act_CompleteErase:
         {
            g.Optical->DoMethodA(ARRAY(DRV_Blank, Drive, DRT_Blank_Complete));
         }
         break;

      case Act_CompleteFormat:
         {
            g.Optical->DoMethodA(ARRAY(DRV_Format, Drive, DRT_Format_Complete));
         }
         break;

      case Act_Prepare:
         {
            g.Optical->DoMethodA(ARRAY(DRV_StructureDisc, Drive));
         }
         break;

      case Act_CloseSession:
         {
            g.Optical->DoMethodA(ARRAY(DRV_CloseDisc, Drive, DRT_Close_Session));
         }
         break;

      case Act_CloseDisc:
         {
            g.Optical->DoMethodA(ARRAY(DRV_CloseDisc, Drive, DRT_Close_Finalize));
         }
         break;

      case Act_CloseTracks:
         {
            closeTracks();
         }
         break;

      case Act_RepairDisc:
         {
            g.Optical->DoMethodA(ARRAY(DRV_RepairDisc, Drive, 0));
         }
         break;
   }
}

uint32 JobMediumAction::getProgress()
{
   return 0;
}

const char *JobMediumAction::getActionName()
{
   switch (eAction)
   {
      case Act_QuickErase:
         return "Quick-Blanking the disc";

      case Act_QuickFormat:
         return "Quick-Formatting the disc";

      case Act_CompleteErase:
         return "Blanking the disc";

      case Act_CompleteFormat:
         return "Formatting the disc";

      case Act_Prepare:
         return "Preparing the disc for write";

      case Act_CloseSession:
         return "Closing session";

      case Act_CloseDisc:
         return "Closing disc";

      case Act_CloseTracks:
         return "Closing incomplete tracks";

      case Act_RepairDisc:
         return "Repairing disc";
   };
   return "Unknown Job";
}

bool JobMediumAction::inhibitDOS()
{
   return true;
}
   
void JobMediumAction::closeTracks()
{
   const IOptItem *dsc, *ses, *trk;

   g.Optical->DoMethodA(ARRAY(DRV_GetAttrs, Drive, DRA_Disc_Contents, (iptr)&dsc, 0));

   if (dsc)
   {
      for (int i=0; i<dsc->getChildCount(); i++)
      {
         ses = dsc->getChild(i);

         for (int j=0; j<ses->getChildCount(); j++)
         {
            trk = ses->getChild(j);
            if (!trk->isComplete())
            {
               g.Optical->DoMethodA(ARRAY(DRV_CloseDisc, Drive, DRT_Close_Track, trk->getItemNumber()));
            }
         }
      }
   }
}

