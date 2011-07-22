#ifndef __Optical_Headers
#define __Optical_Headers

#define __NOLIBBASE__

#include <Generic/Generic.h>

#include <libclass/exec.h>
#include <libclass/dos.h>

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/ports.h>

#include <dos/dos.h>
#include <dos/dostags.h>

#include <utility/tagitem.h>
#include <LibC/LibC.h>
   
   
   extern bool  Clear1, Clear2, AllClear;
   extern class Config        *Cfg;
   
#include "Optical.h"
#include "Internals.h"

#endif
