#include "DriveMsg.h"
#include "DriveClient.h"

uint32 DriveMsg::seq_no = 0;

DriveMsg::DriveMsg(DriveClient* o) :
	Msg(),
	owner(o),
	seq(seq_no++)
{
    if (owner)
	owner->acquire();
}

void DriveMsg::Reply(IDriveStatus &r)
{
    if (owner)
    {
	owner->complete(seq, r);
	owner->release();
    }
}

void DriveMsg::Send()
{
    if (owner)
    {
	owner->getDrive().SendMessage(this);
    }
}


