/*
 * FryingPan - Amiga CD/DVD Recording Software (User Intnerface and supporting Libraries only)
 * Copyright (C) 2001-2008 Tomasz Wiszkowski Tomasz.Wiszkowski at gmail.com
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


#ifndef _ENGINE_ENGINE_H_
#define _ENGINE_ENGINE_H_

#include <Generic/LibrarySpool.h>
#include "../IEngine.h"
#include "Jobs/Job.h"
#include <DTLib/ISpec.h>
#include <DTLib/IData.h>
#include <Generic/Generic.h>
#include <Generic/DistributorT.h>
#include <Generic/ConfigParser.h>
#include <Generic/String.h>
#include <Generic/RWSyncT.h>
#include <Generic/Thread.h>
#include <Generic/DynList.h>
#include <libdata/Optical/IOptItem.h>
#include <Generic/Property.h>
#include "Globals.h"
    
class Event;

class Engine : public IEngine
{
public:
    enum Command
    {
	Cmd_NoOperation   =  0,
	Cmd_ExecuteJob,
	Cmd_SendEvent,
	Cmd_Update,
	Cmd_SetWriteSpeed,
	Cmd_SetReadSpeed,
	Cmd_UpdateLayout
    };

protected:
    enum HookType
    {
	HType_Any,
	HType_Audio,
	HType_Data,
	HType_Session
    };

protected:
    Globals		      &g;

    RWSyncT<DistributorT<EngineMessage, IEngine*> >  handlers;

    String                     sDriveName;
    ConfigParser              *Config;

    String                     sDevice;
    int                        lUnit;
    RWSyncT<iptr>              Drive;

    String                     sDOSDevice;
    bool                       bDOSInhibit;

    String                     sDriveVendor;
    String                     sDriveProduct;
    String                     sDriveVersion;

    DRT_Mechanism              eMechanismType;
    DRT_Profile                eCurrentProfile;
    DRT_SubType                eDiscSubType;
    String                     sVendor;
    uint32                     eReadableMedia;
    uint32                     eWritableMedia;
    uint32                     eCapabilities;
    bool                       bRegistered;
    bool                       bDiscPresent;
    bool                       bRecordable;
    bool                       bErasable;
    bool                       bFormatable;
    bool                       bOverwritable;
    bool                       bWriteProtectable;
    bool                       bWriteProtected;
    DiscSpeed                 *readSpeeds;
    DiscSpeed                 *writeSpeeds;
    uint16                     readSpeed;
    uint16                     writeSpeed;
    uint32                     lDiscSize;

    RWSyncT<Job*>              hJob;
    String                     sJobName;
    const IOptItem            *pCurrentDisc;
    RWSyncT<const TagItem*>    pDriveUpdateTags;
    IOptItem                  *pImportedSession;    // in fact this holds the complete structure (disc and below)
    IData                     *pSessImportReader;   // :P
    /*
     * DTLib :)
     */

    RWSyncT< VectorT<IData*> > hTracks;
    String                        hLayoutStatus;
    uint32                        hLayoutSize;
    DbgHandler*			dbg;

protected:
    Thread                    *thrOperations;

protected:
    /*
     * callback hooks for threads and interop
     */
    HookT<Engine, Thread*, void*>       hHkThrOperations;
    HookT<Engine, Command, Event*>      hHkHndOperations;
    HookT<Engine, iptr, const TagItem*> hHkDriveUpdate;

protected:
    virtual iptr			mainOperations(Thread*, void*);
    virtual iptr			hndlOperations(Command, Event*);
    virtual iptr			onDriveUpdate(iptr drive, const TagItem* tags);
    virtual DbgHandler*			getDebug() const;
    virtual void			setDebug(DbgHandler*);

protected:
    virtual void			readContents();
    virtual bool			setInhibited(String& device, bool state);
    virtual void			do_update();
    void				do_layoutupdate();
    virtual void			do_setwritespeed();
    virtual void			do_setreadspeed();

public:
					Engine(Globals *glob, int which);
    virtual				~Engine();                           

    virtual void			readSettings();
    virtual void			writeSettings();

    virtual void			registerHandler(const Hook*);                // IEngine*, Message*
    virtual void			unregisterHandler(const Hook*);              // IEngine*, Message*

    virtual void			update();
    virtual void			notify(EngineMessage msg);

    virtual const char*			getName();
    virtual void			setName(const char* name);

    /*
     * device info & stuff
     */

    virtual bool               openDevice(const char* name, int lun);
    virtual const char        *getDevice();
    virtual int                getUnit();
    virtual void               closeDevice();
    virtual bool               isOpened();

    virtual const char        *getDriveVendor();
    virtual const char        *getDriveProduct();
    virtual const char        *getDriveVersion();

    virtual DRT_Mechanism      getMechanismType();
    virtual iptr             getReadableMedia();
    virtual iptr             getWritableMedia();

    virtual iptr             getCapabilities();

    virtual DiscSpeed         *getReadSpeeds();
    virtual DiscSpeed         *getWriteSpeeds();
    virtual uint16             getWriteSpeed();
    virtual uint16             getReadSpeed();
    virtual void               setWriteSpeed(uint16 speed);
    virtual void               setReadSpeed(uint16 speed);

    virtual bool               isRegistered();

    virtual bool               isDiscInserted();
    virtual bool               isDVD();
    virtual bool               isCD();
    virtual bool               isBD();
    virtual DRT_Profile        getDiscType();
    virtual DRT_SubType        getDiscSubType();
    virtual const char        *getDiscVendor();
    virtual bool               isRecordable();
    virtual bool               isErasable();
    virtual bool               isFormatable();
    virtual bool               isOverwritable();
    virtual uint32             getDiscSize();

    virtual const char        *getActionName();
    virtual const char        *getLayoutInfo();
    virtual uint32             getLayoutSize();
    virtual unsigned short     getActionProgress();
    virtual void               loadTray();
    virtual void               ejectTray();
    virtual void               closeSession();
    virtual void               closeDisc();
    virtual void               closeTracks();
    virtual void               repairDisc();
    virtual void               quickErase();
    virtual void               completeErase();
    virtual void               quickFormat();
    virtual void               completeFormat();
    virtual void               structureDisc();
    virtual void               updateAll();

    virtual const IOptItem    *getContents();
    virtual void               layoutTracks(bool masterize, bool close);
    virtual void               downloadTrack(const IOptItem*, const char*);
    virtual void               createImage(const char*);
    virtual void               addTrack(IData*);
    virtual void               remTrack(IData*);

    virtual RWSyncT< VectorT<IData*> > &tracks();

    virtual const IOptItem    *getSessionContents();

    virtual void               recordDisc(bool masterize, bool closedisc);
    virtual void               recordSession();

    virtual void               setDOSDevice(const char*);
    virtual const char        *getDOSDevice();
    virtual void               setDOSInhibit(bool flag);
    virtual bool               getDOSInhibit();
    virtual const char        *findMatchingDOSDevice();
    virtual bool               isDiscWriteProtectable();
    virtual bool               isDiscWriteProtected();
    virtual void               setDiscWriteProtected(bool state);
   
    virtual const TagItem*        obtainDriveUpdateTags();
    virtual void                  releaseDriveUpdateTags();
    virtual iptr		  obtainDrive();
    virtual void		  releaseDrive();
};

#endif
