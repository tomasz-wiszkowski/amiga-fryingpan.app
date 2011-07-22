
OBJS = \
        Globals.o			\
        Plug.o				\
	Main.o				\
	Xlate.o				\
	ISOStructures.o			\
	RRStructures.o			\
	ClISO.o				\
	Structure/ClElement.o		\
	Structure/ClRoot.o		\
	Structure/ClDirectory.o		\
	Structure/ClFile.o		\
	Structure/ClImportedFile.o	\
	Structure/ClName.o		\
	Xlators/Translator.o		\
	Xlators/TransDChars.o		\
	Xlators/TransAChars.o		\
	Xlators/TransQChars.o		\
	Xlators/TransRChars.o		\
	CustomISOConfigPage.o		\
	CustomISOPopUp.o

   	
Program			= ISOBuilder.plugin

OBJDIRS			= Xlators Structure
TARGETS			= aros os3 morphos os4

BUILD_LIBRARY	= 0

LIBOPTS			= -lGeneric -lC
OWNSTARTUP		= 1

	include ../Generic/Makefile.mk
