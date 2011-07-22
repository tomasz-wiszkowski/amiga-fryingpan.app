#ifndef _PLUGLIB_IPLUGIN_H
#define _PLUGLIB_IPLUGIN_H

#include <Generic/Types.h>
#include <Generic/RWSyncT.h>

/**
 * \file IPlugin.h
 * \brief Interface specification for plugin node
 */
/**
 * \addtogroup Structures Classes, structures
 * @{
 */
/**
 * \brief IPlugin structure.
 * \details Structure skeleton used to collect plugins and namespaces.
 */
using namespace GenNS;

class IPlugin
{
public:
    /**
     * \fn void Obtain()
     * \brief Obtains (marks as used) object to prevent its unload.
     */
    virtual void Obtain() = 0;

    /**
     * \fn bool Dispose()
     * \brief Disposes plugin, and unloads module, if no more instances are created.
     * \returns true, if module is unloaded.
     */
    virtual bool Dispose() = 0;

    /**
     * \fn void* GetObject()
     * \brief Reference operator for underlying data
     */
    virtual void* GetObject() const = 0;

    /**
     * \fn const char GetName()
     * \brief retrieves name of current node.
     * \returns pointer to string.
     */
    virtual const char* GetName() const = 0;

    /**
      * \fn int GetVersion()
      * \brief Get version of the module
      * \returns version number
      */
    virtual int GetVersion() const = 0;

    /**
      * \fn int GetRevision()
      * \brief Get revision of the module
      * \returns revision number
      */
    virtual int GetRevision() const = 0;

    /**
      * \fn bool IsValid()
      * \brief Validates current plugin
      * \returns true if plugin is valid
      */
    virtual bool IsValid() const = 0;
};

/**
 * @}
 */

#endif
