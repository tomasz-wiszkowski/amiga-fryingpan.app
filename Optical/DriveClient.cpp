#include "Drive.h"
#include "DriveClient.h"
#include <Generic/Port.h>
#include "Drive.h"
#include "DriveSpool.h"
#include "DriveMsg.h"

DriveClient::~DriveClient()
{
    // ...
}

Drive& DriveClient::getDrive()
{
    return drv;
}

void DriveClient::acquire()
{
    clients.ObtainRead();
}

void DriveClient::release()
{
    clients.Release();
}


DriveClient::DriveClient(Drive& drive) :
    drv(drive)
{
    acquire();
    setNotifyCallback(0);
    setCompleteCallback(0);
}

void DriveClient::dispose()
{
    setNotifyCallback(0);
    setCompleteCallback(0);
    release();
    /*
    ** make sure all clients have quit first
    */
    clients.ObtainWrite();
    /*
    ** then make sure no more actions are pending
    */
    sync.ObtainWrite();
    DriveSpool::FreeDriveClient(this);
    delete this;
}

void DriveClient::setNotifyCallback(ICall2T<void, IDriveClient&, const IDriveStatus&>* newhandler)
{
    notify_cb = newhandler;
}

void DriveClient::setCompleteCallback(ICall2T<void, IDriveClient&, const IDriveStatus&>* newhandler)
{
    complete_cb = newhandler;
}

DriveMsg* DriveClient::createMsg(EOptCommand cmd)
{
    DriveMsg *m = new DriveMsg(this);
    m->cmd = cmd;

    return m;
}

void DriveClient::load()
{
    sync.ObtainRead();

    DriveMsg *m = createMsg(DRV_ControlUnit);
    m->Control.control = DRT_Unit_Load;
    m->Send();
}

void DriveClient::eject()
{
    sync.ObtainRead();

    DriveMsg *m = createMsg(DRV_ControlUnit);
    m->Control.control = DRT_Unit_Eject;
    m->Send();
}

void DriveClient::blank(DRT_Blank method)
{
    sync.ObtainRead();

    DriveMsg *m = createMsg(DRV_Blank);
    m->Blank.method = method;
    m->Send();
}

void DriveClient::notify(IDriveStatus &s)
{
    if (notify_cb)
	(*notify_cb)(*this, s);
}

void DriveClient::complete(uint32 seq_no, IDriveStatus &s)
{
    if (complete_cb)
	(*complete_cb)(*this, s);
    sync.Release();
}

void DriveClient::waitComplete()
{
    sync.ObtainWrite();
    sync.Release();
}

const IOptItem* DriveClient::getDiscContents()
{
    return drv.GetDiscContents();
}

bool DriveClient::isDiscPresent()
{
    return drv.IsDiscPresent();
}

bool DriveClient::isErasable()
{
    return drv.IsDiscErasable();
}

bool DriveClient::isFormattable()
{
    return drv.IsDiscFormattable();
}

bool DriveClient::isOverwritable()
{
    return drv.IsDiscOverwritable();
}

bool DriveClient::isWritable()
{
    return drv.IsDiscWritable();
}
bool DriveClient::isFormatted()
{
    return drv.IsDiscFormatted();
}

