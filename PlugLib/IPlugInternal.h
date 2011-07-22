#ifndef _PLUGLIB_IPLUGINTERNAL_H
#define _PLUGLIB_IPLUGINTERNAL_H

#include "IPlugin.h"

/**
 * \file IPlugInternal.h
 * \brief Interface specification for plugin node
 */
/**
 * \copydetails IPlugin
 * \addtogroup Structures Classes, structures
 * @{
 */
/**
 * \brief IPlugInternal structure.
 * \details Internal structure skeleton used to collect plugins and namespaces.
 */
using namespace GenNS;

class IPlugInternal : public IPlugin
{
public:
    /**
     * \fn IPlugin* GetParent()
     * \brief Obtain pointer to parent node.
     * \returns pointer to parent node.
     */
    virtual const IPlugInternal* GetParent() const = 0;

    /**
     * \fn bool AddChild(IPlugin* node)
     * \brief Queues child for current namespace
     * \param node - pointer to node to add
     * \returns true on success.
     */
    virtual bool AddChild(const IPlugInternal* node) = 0;

    /**
     * \fn bool RemChild(IPlugin* node)
     * \brief Dequeues child from current namespace
     * \param node - pointer to node to dequeue
     * \returns true, if node has been unloaded.
     */
    virtual bool RemChild(const IPlugInternal* node) = 0;

    /**
     * \fn bool IsNameSpace()
     * \brief Checks if current node is a namespace.
     * \returns true if node is namespace, false otherwise.
     */
    virtual bool IsNameSpace() const = 0;

    /**
      * \fn int GetChildCount()
      * \brief Method used to collect information about number of children in current node
      * \returns number of children
      */
    virtual int GetChildCount() const = 0;

    /**
     * \fn IPlugin* GetChild(int num)
     * \brief Retrieves n-th child, or 0 if there is no child with that number
     * \param int num - child number
     * \returns pointer to IPlugInternal or 0
     */
    virtual const IPlugInternal* GetChild(int num) const = 0;

    /**
     * \fn IPlugin* FindNode(const TagItem* spec)
     * \brief Searches for a node with a specified name; 
     * \details This function searches recursively for a node and creates one if not found, unless 
     * tags say otherwise. Multiple namespaces can be specified, but only one name can be selected.
     * \param spec - array of tags specifying what to look for
     * \returns pointer to IPlugin or 0, if \ref PLO_NoLoad was specified
     */
    virtual const IPlugInternal* FindNode(const struct TagItem* spec) = 0;

};

/**
 * @}
 */

#endif

