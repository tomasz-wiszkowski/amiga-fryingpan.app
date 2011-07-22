#ifndef _ISOBUILDER_GLOBALS_H_
#define _ISOBUILDER_GLOBALS_H_

#include <Generic/Locale.h>
#include <Generic/Debug.h>
#include <Generic/Configuration.h>

enum 
{
    LocaleVersion   =	1
};

using namespace GenNS;

/*
** this is mostly read-only structure.
** holds items shared among all components of
** isobuilder.
** In general, we want all the elements accessible
** by default, thus we won't make it a class.
*/
struct Globals
{
    /** Translating facility.
     *  This facility is used to translate the application.
     *  Note that localization MAY be inherited from owner
     *  (and in most cases it will be)
     */
    Localization	    Loc;

    /** Configuration file handling.
     */
    mutable Configuration   Config;

public:

    /** Constructor
     */
    Globals();
};

#endif
