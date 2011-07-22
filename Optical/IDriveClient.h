#ifndef _OPTICAL_IDRIVECLIENT_H_
#define _OPTICAL_IDRIVECLIENT_H_

#include "Optical.h"
#include <utility/hooks.h>
#include <Generic/Types.h>
#include <Generic/CallT.h>
#include "IOptItem.h"

/** Defines interface between the application and Optical that will be from some point
 *  the only way to access device.
 */

class IDriveStatus
{
public:
    enum Constants
    {
	Progress_Max = 1024
    };
    /** Currently executed operation status.
     *  When signals \b DRT_OpStatus_Busy, 
     *  you can rely on \b operation and \b progress fields.
     *  When set to \b DRT_OpStatus_Complete, all elements carry useful information.
     */
    DRT_OperationStatus	    status;

    /** Operation type. 
     *  \sa DRT_Operation
     */
    DRT_Operation	    operation;

    /** Major advance status of current operation. This can be understood as \b primary
     *  or \b major progress information (e.g. disc record progress).
     *  values: 0 - 1024
     */
    uint16		    progress_major;

    /** Minor advance status of current operation. This can be understood as \b secondary
     *  or \b minor progress information (e.g. track record progress).
     */
    uint16		    progress_minor;

    /** Error code, available upon completion of the operation. 
     *  \sa EOpticalError
     */
    EOpticalError	    error;

    /** SCSI Error code
    */
    uint32		    scsi_error;

    /** Detailed description of the problem (if error is not 'OK').
     *  Description should hold explanation why operation failed.
     */
    const char*		    description;
}; 

class IDriveClient
{
protected:
    /** Hidden destructor: you are not allowed to destruct object using delete.
     */
    virtual ~IDriveClient() {};

public:
    /** Registers notification callback.
     *  Notifications are sent using DriveOp structure.
     *	Theoretically any number of hooks can be registered.
     *	In practice this is limited by congestion speed (meaning you can still
     *	register as many as you want, but performance may suffer)
     *	\param hk points to a hook structure that will receive
     *	const IDrive pointer as first argument and const DriveOp structure as second.
     *	\note this is called from the context of Drive process and you \b must handle
     *	all notifications efficiently.
     */
    virtual void	    setNotifyCallback(ICall2T<void, IDriveClient&, const IDriveStatus&>*) = 0;

    /** Registers completion callback.
     *  Same as above, except called only upon completion of 
     *  commands invoked by this particular client
     */
    virtual void	    setCompleteCallback(ICall2T<void, IDriveClient&, const IDriveStatus&>*) = 0;

   /** Dispose this device. Device will be released once all applications dispose it.
     *  \sa obtain
     */
    virtual void	    dispose() = 0;

    /** Loads disc, if possible.
     *  \sa eject
     */
    virtual void	    load() = 0;

    /** Ejects disc, if possible.
     *  \sa load
     */
    virtual void	    eject() = 0;

    /** Erase (blank) disc.
     * Initializes rewritable disc to the state where no sessions and
     * no tracks are present.
     * \param method determines blanking method (blank / format / defaul)
     * \sa format
     */
    virtual void	    blank(DRT_Blank method) = 0;

    /** Check whether disc is in drive
     * Returns true if disc is in drive
     */
    virtual bool	    isDiscPresent() = 0;

    /** Check whether disc is erasable.
     * \sa isFormattable
     */
    virtual bool	    isErasable() = 0;

    /** Check whether disc is formattable.
     * \sa isErasable
     */
    virtual bool	    isFormattable() = 0;

    /** Check whether disc can be overwritten (i.e. works like floppy).
     * \sa isWritable
     */
    virtual bool	    isOverwritable() = 0;


    /** Check whether disc is writable.
     * \sa isOverwritable
     */
    virtual bool	    isWritable() = 0;

    /** Check whether disc is formatted.
     *  \sa isFormattable
     */
    virtual bool	    isFormatted() = 0;

    /** Get contents of the disc.
     * Returns pointer to the top-most element representing 
     * disc contents. You MUST call release() on that
     * object once you're done with your pointer
     * \sa
     */
    virtual const IOptItem *getDiscContents() = 0;



    /** Wait for operation to complete
     */
    virtual void	    waitComplete() = 0;
};

#endif
