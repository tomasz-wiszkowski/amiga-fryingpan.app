#ifndef __DRIVE_H
#define __DRIVE_H

#include "Commands.h"
#include "Various.h"
#include "CfgHardware.h"
#include <Generic/RWSyncT.h>
#include <Generic/Thread.h>
#include <Generic/HookT.h>
#include <Generic/Debug.h>
#include <Generic/CallT.h>
#include <Generic/Set.h>
#include "DriveStatus.h"
#include "SCSI/scsi_GetConfiguration.h"

using namespace GenNS;

extern class DriveSpool *Spool;
class GenNS::Thread;
class DriveClient;
class DriveMsg;
class scsi_GetConfiguration;

class Drive
{
    DEFINE_DEBUG;
private:
    /*
    ** new stuff
    */
    RWSyncT< VectorT<DriveClient*> >	users;

    /*
    ** really twisted stuff
    */
    Call2T<void, Drive, const Port*, GenNS::Msg*>	handle;
    Call1T<void, Drive, Thread*>			task;
    Thread					        thread;

    /*
    ** disc stuff. detection and other...
    */
    bool		    is_disc_in_drive;
    RWSyncT<class Disc*>    current_disc;

private:
    /*
    ** a copy of notification items to not distribute over and over same message
    */
    DriveStatus		    last_result;
    DriveIO		    driveio;
    cmd_Inquiry		    inquiry;
    cmd_TestUnitReady	    tur;
    cmd_Mode		    mode_io;
    scsi_GetConfiguration   features;

protected:

    String                        drive_name;
    int                           unit_number;
    class cmd_ReadDiscInfo       *discinfo;               //
    Page<Page_Write>              pw;
    Page<Page_Capabilities>       pc;
    RWSync                        drive_lock;


    DRT_Mechanism                 eMechanismType;
    Set                           eCapabilities;
    bool                          bRegistered;
    bool                          bLockInterOperations;   // true if we dont want additional operations inbetween our calls

    CfgHardware                  *hwconfig;

    uint32                        media_write_capabilities;
    uint32                        media_read_capabilities;
    uint32                        data_write_capabilities;
    uint32                        data_read_capabilities;
    uint16                        drive_buffer_size;         // in kb

    uint32                        current_drive_profile;
    uint16                        selected_read_speed;
    uint16                        selected_write_speed;
    bool			  update;


    /*
     * tag update distribution
     */
protected:

    bool    ProcInit();
    void    ProcMsgs(Thread*);
    void    ProcExit();
    void    HandleMessages(const Port*, GenNS::Msg*);

protected:
    Drive(const char* pDeviceName, int lUnitNum);

public:
    ~Drive();

    static Drive        *GetDrive(const char *device, int unit);
    const String        &GetDeviceName();
    int                  GetUnitNumber();

    void                 AnalyseDisc();
    void                 AnalyseDrive();

    static iptr          ScanDevice(char* sDeviceName);
    static iptr          FreeScanResults(ScanData*pData);


    /** Dispatch all controls.
     */
    bool		ControlDrive(DRT_Control type);

    //--------------------------------------------------------------------------------
    DriveIO&		    GetDriveIO()
    {  
	return driveio;                    
    };

    //--------------------------------------------------------------------------------
    DriveStatus&	    GetDriveStatus()
    {
	return last_result;
    }

    //--------------------------------------------------------------------------------
    scsi_GetConfiguration&  GetFeatures()		
    { 
	return features; 
    }
    DbgHandler          *GetDebug()
    {  return DEBUG_ENGINE;                };

    ULONG                MediaReadCapabilities()
    {  return media_read_capabilities;     };

    ULONG                MediaWriteCapabilities()
    {  return media_write_capabilities;    };

    ULONG                DataReadCapabilities()
    {  return data_read_capabilities;      };

    ULONG                DataWriteCapabilities()
    {  return data_write_capabilities;     };

    ULONG                DriveBufferSize()
    {  return drive_buffer_size;           };

    ULONG                CurrentProfile()
    {  return (int)current_drive_profile;  };

    CfgHardware         *GetHardwareConfig()
    {  return hwconfig;                    };


    iptr                             GetDriveAttrs(iptr, iptr);

    Page<Page_Write>                &GetWritePage();
    Page<Page_Capabilities>         &GetCapabilitiesPage();

    DriveStatus&		    SetPage(Page_Header*);


    /*
    ** now the external interface
    */
public:
    iptr              GetDriveAttrs(TagItem* tag);
    iptr              GetDriveAttrs(iptr tag);
    void              SendMessage(DriveMsg*);

    /** Send response to client with most recent status
     */
    void	    notify(IDriveStatus&);

    /*
    ** Getters
    */
    const IOptItem* GetDiscContents();
    bool	    IsDiscPresent();
    bool	    IsDiscErasable();
    bool	    IsDiscFormattable();
    bool	    IsDiscOverwritable();
    bool	    IsDiscFormatted();
    bool	    IsDiscWritable();
public:	
    /**
     * to be re-thought
     */
    DriveClient*    getClient();
    bool	    freeClient(DriveClient*);

};


#endif
