#ifndef __SCSI_GETPERFORMANCE_H
#define __SCSI_GETPERFORMANCE_H

#include "SCSICommand.h"
#include "uniform.h"

class Perf_Header
{
protected:
    aLong	length;
    struct _1 : public aLong
    {
	bool hasWrite()  { return getField(25, 1) == 1; }
	bool hasExcept() { return getField(24, 1) == 1; }
    } flags;
public:
    void Dispose()
    {
	/*
	** allocated as an array
	** so must be freed as one
	*/
	delete [] this;
    }
};

class Perf_PerformanceData : public Perf_Header
{
protected:
    struct 
    {
	aLong   start_lba;
	aLong   start_performance;
	aLong   end_lba;
	aLong   end_performance;
    } descriptors [0];


public:
    bool    getPerfItem(int index, uint32 &stblk, uint32 &sspd, uint32 &endblk, uint32 &espd);
    int32   getPerfCount();
};

class Perf_WriteSpeeds : public Perf_Header
{
protected:
    struct 
    {
	struct _1 : public aLong
	{
	    bool    isMRW()	{ return getField(24, 1); }
	    bool    isExact()	{ return getField(25, 1); }
	    int	    speedType() { return getField(27, 2); }
	} entryFlags;

	aLong	end_lba;
	aLong	read_speed;
	aLong	write_speed;
    } descriptors [0];


public:
    bool    getSpeedItem(int index, uint32 &rspd, uint32 &wspd);
    int32   getSpeedCount();
};

class scsi_GetPerformance : public SCSICommand
{
public:
    enum Perf_Type
    {
	PerfType_PerformanceData = 0,
	PerfType_UnusableArea,
	PerfType_DefectStatus,
	PerfType_WriteSpeeds,
	PerfType_DBI,
	PerfType_DBICache
    };

protected:
    Perf_Type	    type;
    bool	    write;
    Perf_Header *perform_data;

protected:
    Perf_Header*  readPerformance(Perf_Type t, bool write);
public:

    scsi_GetPerformance(DriveIO &, DriveStatus&);
    virtual ~scsi_GetPerformance();

    bool onInit();

    Perf_PerformanceData* readPerformanceData(bool write);
    Perf_WriteSpeeds* readWriteSpeeds();
};


#endif

