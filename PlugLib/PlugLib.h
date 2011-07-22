/*
 * PlugLib - plugin management library for AmigaOS 
 * Copyright (C) 2001-2009 Tomasz Wiszkowski Tomasz.Wiszkowski at gmail.com
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


#ifndef _PLUGLIB_PLUGLIB_H_
#define _PLUGLIB_PLUGLIB_H_

#include <libclass/exec.h>
#include <exec/nodes.h>

#undef MOS_DIRECT_OS
#include <GenericLib/Calls.h>

struct IPlugin;
struct TagItem;

/** 
 * \file PlugLib.h 
 * \brief Plugin handling library for dynamically loaded, nonstandard plugins for all AmigaOS compatible platforms.
 * \details This library offers management functions for dynamically loadable plugins of all sorts.
 * 
 * To write your own plugin, you have to declare header anywhere in your code:
 * 
 * \code
 * 
 * \endcode
 * 
 * then, simply call OpenPlugin() to open plugin and obtain your own plugin interface.
 *
 * \code
 *
 * \endcode
 *
 * Finally, don't forget to clean up once you're done - call ClosePlugin():
 *
 * \code
 * PlugLib::ClosePlugin(plugin);
 * \endcode
 */

/**
 * \addtogroup Functions Library defined functions
 * @{
 */
BEGINDECL(PlugLib, "plug.library")
    /**
     * \fn void* PlugLib::OpenPlugin(const TagItem* tags=a0)
     * \details This function opens / accesses a plugin.
     * If plugin is a loadable module with name passed via
     * \ref PLO_PluginName, then library searches (in order)
     *  - CWD/
     *  - CWD/plugins/
     *  - CWD/libs/plugins/
     *  - libs:plugins/
     *  .
     * directories in search for specified file. The same rules as with
     * OpenLibrary() apply, meaning
     *  - your plugin name has to case-match the file-part of plugin name
     *  - minimum version and revision must match (assuming 0 if not given)
     *  .
     * differences:
     *  - multiple instances of plugin can be kept in different namespaces
     *  - plugins are automatically unloaded after last application disposes data
     *  .
     * \param tags - array of \ref PL_OpenTags tags
     * \return void* - pointer to your plugin data or 0
     * \note You have to call Dispose() on your plugin to release it.
     */

    /**
     * \cond SKIP
     */
    FUNC1(const IPlugin*, OpenPlugin, 5, const TagItem*, tags, a0);
    /**
     * \endcond
     */
ENDDECL
/* @} */
/**
 * \addtogroup Tags PlugLib tags
 * @{
 */
/** 
 * \brief Tags used with \ref OpenPlugin call 
 */
enum PL_OpenTags
{
    /**
     * \par const char* 
     * Name of the namespace to store module in, for example 
     * if you want to keep same modules but be able to distinguish 
     * \b debug from \b release, you can do so by using namespaces.
     */
    PLO_NameSpace = 0x83100000,

    /**
     * \par const char*
     * Name of module to load (if you want to load external module) or null 
     * (in that case only an area of memory will be created for your own purpose).
     */
    PLO_PluginName,

    /**
     * \par int
     * Minimum plugin version. Default 0.
     */
    PLO_MinVersion,

    /**
     * \par int
     * Minimum plugin revision. Default 0.
     */
    PLO_MinRevision,

    /**
     * \par bool
     * Specifies, whether to load plugin if one is not loaded already. Default false.
     */
    PLO_NoLoad,

};

/** 
 * \brief Tags used to define plugin through \ref Plugin::pa_Tags
 */
enum PL_PluginTags
{
    /** \par char*
     * Plugin author 
     */
    PLP_Author = 0x83200000,

    /** \par char*
     * Plugin version
     */
    PLP_Version,
};

/**
 * \brief Header Magic values
 * \details \b Plugin_Magic_Value_1 through \b Plugin_Magic_Value_4 define
 * four id's for \ref Plugin header. \ref Plugin module is identified by
 * presence of the four long words in \ref Plugin::pa_Magic.
 */
enum Plugin_Header_Magic_Values
{
    /**
     * Version of current plugin header.
     */
    Plugin_Header_Version   =	1,
    /** \cond SKIP */
    Plugin_Magic_Value_1    =	'PLUG',
    Plugin_Magic_Value_2    =	'IN M',
    Plugin_Magic_Value_3    =   'AGIC',
    Plugin_Magic_Value_4    =	0xf0d2b497 
    /** \endcond */
};

/**
 * @}
 */

/**
 * \addtogroup Structures Classes, structures
 * @{
 */
/**
 * \brief Definition of plugin header
 * \details This definition must be present in every file you want 
 * to compile as a plugin. Its contents are also critical for the
 * library to understand your file is actually a real plugin.
 * To build a plugin, place this definition anywhere in your file:
 *
 * \code
 * struct TagItem* const myTags[] = {
 *    { PLP_Author,         (long)"Tomasz Wiszkowski" },
 *    { PLP_Version,	    (long)"$VER: example.plugin 1.0 Tomasz Wiszkowski" },
 *    { TAG_DONE,           0 },
 * };
 *
 * struct PluginHeader myHeader {
 *    Plugin_Header_Magic,	// magic
 *    &myHeader,		// self pointer
 *    "example.plugin",		// name
 *    Plugin_Header_Version,	// header version
 *    0,			// flags
 *    1,			// version
 *    0,			// revision
 *    (TagItem* const)&myTags,	// tagarray
 *    &functionArray		// user data (could be initialized with setup)
 *    0,			// setup
 *    0,			// cleanup
 * };
 * \endcode
 *
 */
struct PluginHeader
{
    /**
     * Must be set to Plugin_Header_Magic.
     */
    Plugin_Header_Magic_Values const    pa_Magic[4];

    /**
     * Pointer to self.
     */
    const PluginHeader* const		pa_Self;

    /**
     * Plugin name - as stored
     */
    const char* const			pa_PluginName;

    /**
     * Plugin header version
     */
    const uint8				pa_HeaderVersion;

    /**
     * Flags (0 for now)
     */
    const uint8				pa_Flags;

    /**
     * Plugin version
     */
    const uint8				pa_Version;

    /**
     * Plugin revision
     */
    const uint8				pa_Revision;

    /**
     * Array of \ref PL_PluginTags tags.
     */
    const TagItem* const		pa_Tags;

    /**
     * User pointer to plugin data (returned upon -> reference)
     * this pointer can be initialized with setup routine
     */
    void*				pa_Plugin;

    /**
     * \fn bool (*pf_SetUp)()
     * Pointer to initialization routine. Called once after module is loaded.
     * \param exec - pointer to exec base
     * \returns true if initialization completed successfully.
     * \note This pointer can be set to 0.
     */
    bool (*pf_SetUp)(struct Library* exec);

    /**
     * \fn void (*pf_CleanUp)()
     * Pointer to finalization routine. Called once before module is unloaded.
     * \param none
     * \returns nothing
     * \note This pointer can be set to 0.
     */
    void (*pf_CleanUp)();

};
/* @} */

/**
 * \addtogroup Macros Macro definitions
 * @{
 */
/**
 * Define header Magic ID for Plugin structure
 * \sa Plugin_Header_Magic_Values
 */
#define Plugin_Header_Magic { Plugin_Magic_Value_1, Plugin_Magic_Value_2, Plugin_Magic_Value_3, Plugin_Magic_Value_4 }
/* @} */
#endif

