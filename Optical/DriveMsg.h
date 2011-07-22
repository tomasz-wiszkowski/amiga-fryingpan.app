#ifndef _OPTICAL_DRIVEMSG_H
#define _OPTICAL_DRIVEMSG_H

#include <Generic/Msg.h>
#include "Optical.h"
#include <utility/tagitem.h>
#include "IOptItem.h"

class DriveClient;
class IDriveStatus;
class Signal;

class DriveMsg : public GenNS::Msg
{
public:
    DriveClient*    owner;
    EOptCommand	    cmd;
    uint32	    seq;
    static uint32   seq_no;

    union
    {
	struct	/* control */
	{
	    DRT_Control	    control;
	} Control;

	struct	/* prepare disc for writing */
	{
	    DRT_Blank	    method;
	} Blank;

	struct	/* Close */
	{
	    DRT_Close	method;
	    uint16	track_number;
	} Close;

	struct	/* Attr set/get */
	{
	    const TagItem*  tags;
	} Attr;

	struct	/* Play */
	{
	} Play;

	struct	/* Repair */
	{
	} Repair;

	struct	/* Write */
	{
	    /*
	    ** TODO: implement testmode and method!
	    */
	    bool	    checkLayout;
	    bool	    testMode;
	    DRT_WriteMethod method;
	    IOptItem*	    item;
	} Write;

	struct	/* Read */
	{
	    IOptItem*	item;
	    uint32	block;
	    uint32	length;
	    void*	mem;
	} Read;

	struct	/* Speeds */
	{
	    /*
	    ** TODO: implement!
	    */
	    uint16  readSpeedIdx;
	    uint16  writeSpeedIdx;
	} Speed;

	struct  /* Notify */
	{
	    enum
	    {
		Progress_Max	=   32768
	    };
	    DRT_OperationStatus	status;
	    DRT_Operation	operation;
	    uint16		progress_major;
	    uint16		progress_minor;
	    EOpticalError	error;
	    uint32		scsi_error;
	    const char*		details;
	} Notify;

	struct	/* Result */
	{
	    DRT_OperationStatus	status;
	    EOpticalError	error;
	    uint32		scsi_error;
	    const char*		details;
	} Result;

    };

public:
    DriveMsg(DriveClient* o);


    virtual void Reply(IDriveStatus&r);

    uint32 GetSeqNo()
    {
	return seq;
    }

    void Send();
};

#endif

