#!/bin/bash


BASE=/home/durand/spoon/BUILD


DIR=$1
STATIC_LIBRARIES="$BASE/system/lib/libunix.a"

UNIX_LIBRARIES="$BASE/system/lib/*.a" 


if [ ! -d $DIR ]; then
	echo "That was not a directory.";
	exit -1;
fi;


echo "WARNING - DO NOT FORGET TO SET THE LIBS IN THE CONFIGURE SCRIPT"

CFLAGS="-static -nostdinc -nodefaultlibs -I$BASE/system/include/"  
CCFLAGS=$CFLAGS  
CPPFLAGS=$CFLAGS  
LDFLAGS="$UNIX_LIBRARIES $STATIC_LIBRARIES"

LIBS=$LDFLAGS
# -nostdlib


export CFLAGS
export CCFLAGS
export CPPFLAGS
export LDFLAGS
export LIBS


cd $DIR

if [ ! -e "./configure" ]; then
	echo "Configure script is missing! Trying a manual override."
	make clean;
	make linux;
	if [ "$?" != "0" ]; then
	    echo "Nope. That didn't work again."
		exit -1;
	fi;
else


	CFLAGS="$CFLAGS" CCFLAGS="$CCFLAGS" LDFLAGS="$LDFLAGS" 						\
						./configure --prefix=$BASE/unix

	if [ "$?" != "0" ]; then
		echo "Configure didn't work too well.... Please fix the problems."
		exit -1;
	fi;


	make clean; 
	make


	if [ "$?" != "0" ]; then
		echo "Didn't seem to compile properly...";
		exit -1;
	fi;


	make install


fi;


echo "Seems okay... good luck"

