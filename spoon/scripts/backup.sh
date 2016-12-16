#!/bin/sh


# ---------------------------------------------
#  This is an automatic backup script which
#  produces the following files as output:
#
#     smk.tar.gz & smk-$DATE.tar.gz
#     libunix.tar.gz & libunix-$DATE.tar.gz
#     spoon.tar.gz & spoon-$DATE.tar.gz
#
#  Where $DATE is %Y%m%d-%H%M%S and is generated
#  using the standard date command.
#
#  It uses the $TEMPDIR for working.
#  It uses the $BACKUPDIR for storage of the
#  output files.
#
# ---------------------------------------------

DATE=`date +"%Y%m%d-%H%M%S"`

TARFLAGS=czf


SPOONNAME=spoon-$DATE.tar.gz
SMKNAME=smk-$DATE.tar.gz
UNIXNAME=libunix-$DATE.tar.gz

TEMPDIR=/tmp/spoon
BACKUPDIR=~/backups

TOPDIR=$PWD
DIRECTORY=`echo $PWD | sed "s/.*\///g"`

# --------------------------------------------------------

# Request permission to do everything.

echo "Does this script have permission to do the following:";
echo;
echo " 1. Create the directories $BACKUPDIR and $TEMPDIR if";
echo "    non-existant.";
echo " 2. Remove everything in $TEMPDIR";
echo " 3. Create backups in $BACKUPDIR";
echo
echo " yes or no [y/N] ? "
read PERMISSION

if [ "$PERMISSION" == "y" ]; then PERMISSION=Y; fi;

if [ "$PERMISSION" != "Y" ]; then
	echo "Okay. No permission given. I won't do anything.";
	exit 0;
fi;

# ---------------------------------------------------------

# Create the directories

rm -rf $TEMPDIR
mkdir -p $TEMPDIR
mkdir -p $BACKUPDIR

# Check to see if it worked.

if [ ! -d $BACKUPDIR ]; then
	echo "Unable to create: $BACKUPDIR"
	exit -1;
fi;

if [ ! -d $TEMPDIR ]; then
	echo "Unable to create: $TEMPDIR"
	exit -1;
fi;


# ---------------------------------------------------------


# Clean the source tree

if [ ! -e ./scripts/backup.sh ]; then
	echo "This isn't the main root of the spoon source tree.";
	echo "Please go to the main root and run this script again.";
	exit -1;
fi;


make clean


# For the smk directory.
cp -rf $TOPDIR/config \
	   $TOPDIR/devices \
	   $TOPDIR/LICENSE \
	   $TOPDIR/kernel \
	   $TEMPDIR/



cd .. 
tar -$TARFLAGS $BACKUPDIR/snapshot.tar.gz --exclude=./$DIRECTORY/.git --exclude=./$DIRECTORY/BUILD --exclude=./$DIRECTORY/unix/ports/cache --exclude=./$DIRECTORY/unix/ports/downloads ./$DIRECTORY 
cd $TOPDIR
tar -$TARFLAGS $BACKUPDIR/libunix.tar.gz -C $TOPDIR/unix/  ./libunix
tar -$TARFLAGS $BACKUPDIR/smk.tar.gz -C $TEMPDIR/ \
			./kernel ./config ./devices \
			./LICENSE

# ---------------------------------------------------------
# Rename the files.

cp -f $BACKUPDIR/snapshot.tar.gz $BACKUPDIR/$SPOONNAME
cp -f $BACKUPDIR/libunix.tar.gz $BACKUPDIR/$UNIXNAME
cp -f $BACKUPDIR/smk.tar.gz $BACKUPDIR/$SMKNAME

# ---------------------------------------------------------
# Clean the temporary directory.
rm -rf $TEMPDIR

# ---------------------------------------------------------

echo "";
echo "The following files were created: ";
echo 
echo "$BACKUPDIR/smk.tar.gz"
echo "$BACKUPDIR/libunix.tar.gz"
echo "$BACKUPDIR/snapshot.tar.gz"
echo 
echo "$BACKUPDIR/$SMKNAME"
echo "$BACKUPDIR/$UNIXNAME"
echo "$BACKUPDIR/$SPOONNAME"
echo 
echo "done.";



