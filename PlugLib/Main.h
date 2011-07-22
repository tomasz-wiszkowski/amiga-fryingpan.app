#ifndef _PLUGLIB_MAIN_H
#define _PLUGLIB_MAIN_H

#include "PlugLib.h"
#include <Generic/Debug.h>
#include <Generic/SyncT.h>
#include "PlugRoot.h"

using namespace GenNS;
/**
 * \file Main.h
 * \brief PlugLib internals
 */

extern SyncT<PlugRoot> *Root;
/**
 * \copydetails PlugLib
 */
const IPlugin* OpenPlugin(const TagItem* tags);
bool ClosePlugin(const IPlugin* plug);

/**
 *
 */
DECLARE_DEBUG;

#endif
