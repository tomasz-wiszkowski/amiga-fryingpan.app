#Wrapper.o						\

OBJS = \
	Plug.o							\
	Engine.o						\
	Globals.o						\
	Event.o							\
	Jobs/Job.o						\
	Jobs/JobControl.o				\
	Jobs/JobWriteProtect.o			\
	Jobs/JobMediumAction.o			\
	Jobs/JobDownload.o				\
	Jobs/JobUpload.o				\
	Jobs/JobUploadSession.o			\
	Jobs/JobLayout.o				\
	Jobs/JobUpdate.o				\
	Jobs/JobCreateISO.o
	
Program			= engine.plugin

TARGETS			= aros os3 morphos os4
OBJDIRS			= Jobs
MAKEDIRS		= 

BUILD_LIBRARY	= 0
PREP			= -DDEBUG

INCOPTS			= 
LIBOPTS			= -lLibrary -lGeneric -lC

OWNSTARTUP		= 1
AROS_LIBOPTS	= 
M68K_LIBOPTS	=
MOS_LIBOPTS		=
OS4_LIBOPTS		=

	include ../../Generic/Makefile.mk

// vim: noet ts=4
