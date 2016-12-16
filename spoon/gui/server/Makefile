MAKEDIR=../../config
include $(MAKEDIR)/make.app

DESCRIPTION=GUI server
APPNAME=gui_server
INSTALLPATH=$(BASE)/system
CODE="C"
FILES=*.c base/*.c

	
EXTRA_LDFLAGS=$(BASE)/system/lib/libunix.a $(BASE)/system/lib/libvfs.a



unix_os:
	gcc -DUNIX $(FILES) -o $(APPNAME)



