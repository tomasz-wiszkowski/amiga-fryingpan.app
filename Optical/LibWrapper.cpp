#define __NOLIBBASE__
#include <GenericLib/Library.h>
#include <Startup/Startup.h>
#include <exec/libraries.h>
#include "Headers.h"
#include "Main.h"
#include "Config.h"

   LIBRARY("Optical", "$VER: Optical 2.0 (" __DATE__ " " __TIME__ ") Tomasz Wiszkowski", 2);

   GATE1(iptr,             DoLibMethodA,     iptr*,  a0);
   GATE0(class IOptItem*,  OptCreateDisc);
   LIB_FT_Begin
      LIB_FT_Function(DoLibMethodA)
      LIB_FT_Function(OptCreateDisc)
   LIB_FT_End

bool Lib_SetUp()
{
   if (false == __setup())
      return false;

   OurBase->lib_Flags |= LIBF_DELEXP;
   return SetUp();
}

void Lib_CleanUp()
{
   CleanUp();
   __cleanup();
}

bool Lib_Acquire()
{
   return true;
}

void Lib_Release()
{
   if (Cfg != 0)
      Cfg->onWrite();
}

