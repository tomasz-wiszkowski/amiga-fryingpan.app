#ifndef _OPTICAL_DRIVECLIENT_H_
#define _OPTICAL_DRIVECLIENT_H_

#include "IDriveClient.h"
#include "Drive.h"
#include <Generic/Port.h>
#include <Generic/CallT.h>
#include <Generic/Msg.h>
#include <Generic/Signal.h>
#include <exec/tasks.h>
#include "IOptItem.h"

class IDriveStatus;

class DriveClient : public IDriveClient
{
    ICall2T<void, IDriveClient&, const IDriveStatus&>* notify_cb;
    ICall2T<void, IDriveClient&, const IDriveStatus&>* complete_cb;
    Drive&		drv;
    GenNS::RWSync	clients;
    GenNS::RWSync	sync;

protected:
    virtual ~DriveClient();

    DriveMsg* createMsg(EOptCommand cmd);

public:	/* non-interface functions */
    virtual Drive&	    getDrive();
    virtual void	    notify(IDriveStatus &s);
    virtual void	    complete(uint32 seqno, IDriveStatus &s);

    /* this is used internally to keep track on uses of the object */
    void		    acquire();
    void		    release();

public:
    DriveClient(Drive&);
    virtual void  	    load();
    virtual void	    eject();
    virtual void	    blank(DRT_Blank method);

    /*
    ** Getters
    */
    virtual const IOptItem *getDiscContents();
    virtual bool	    isDiscPresent();
    virtual bool	    isErasable();
    virtual bool	    isFormattable();
    virtual bool	    isOverwritable();
    virtual bool	    isWritable();
    virtual bool	    isFormatted();

    /*
    ** Helpers
    */
    virtual void	    dispose();
    virtual void	    setNotifyCallback(ICall2T<void, IDriveClient&, const IDriveStatus&>*);
    virtual void	    setCompleteCallback(ICall2T<void, IDriveClient&, const IDriveStatus&>*);
    virtual void	    waitComplete();
};

#endif

