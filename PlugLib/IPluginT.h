#ifndef _PLUGLIB_IPLUGINT_H
#define _PLUGLIB_IPLUGINT_H

#include "IPlugin.h"

/**
 * \file IPluginT.h
 * \brief Generalization for plugin node
 */
/**
 * \addtogroup Structures Classes, structures
 * @{
 */
/**
 * \brief IPluginT structure.
 * \details Generalization used to collect plugins and namespaces.
 */
using namespace GenNS;

template <typename T> class IPluginT : public IPlugin
{
public:
    /**
     * \fn T operator ->()
     * \brief Operator overload allowing access to underlying class
     */
    inline T operator ->()
    {
	return (T) GetObject();
    }
};

/**
 * @}
 */

#endif
