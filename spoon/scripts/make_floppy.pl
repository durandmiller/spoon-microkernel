#!/usr/bin/perl -w

#  I think this is going to be a lot of work for something
#  which will be constantly changing.. let's hope this works
#  well enough.

use strict;
use File::Copy;

my $TOPDIR = $ENV{'PWD'};

die "Environment variable PWD is not set\n" 
		if ( ! $TOPDIR );


if ( ! -e $TOPDIR."/scripts/make_floppy.pl" ) 
{
	print "This isn't the main root of the spoon source tree.\n";
	print "Please go to the main root and run this script again.\n";
	print "\n";
	print "by running:  ./scripts/make_floppy.pl\n";
	exit -1;
}


#Find the base directory..

my $BUILD_DIR = "";


	open(FD, "< $TOPDIR/config/make.base") or die ("unable to open $TOPDIR/config/make.base\n");
	while (<FD>)
	{
		my $line = $_;
		chomp( $line );
		if ( $line =~ /^\s*BASE=\s*(.*)/ )
		{
			$BUILD_DIR = $1;
			$BUILD_DIR =~ s/\$\(MAKEDIR\)/$TOPDIR\/config/g;
		}
	}
	close(FD);


# Got it.

die "Unable to find BASE information in $TOPDIR/config/make.base\n" if ( $BUILD_DIR eq "" );


die "GRUB not found at /sbin/grub or $TOPDIR/config/grub\n" 	
	if ( (! -e "/sbin/grub") && (! -e "$TOPDIR/config/grub") );
	
die "sudo not found in /usr/bin/sudo\n"	if ( ! -e "/usr/bin/sudo" );
die "dd not found in /bin/dd\n" 		if ( ! -e "/bin/dd" );
die "mkfs not found in /sbin/mkfs\n" 	if ( ! -e "/sbin/mkfs" );
die "mount not found in /bin/mount\n" 	if ( ! -e "/bin/mount" );
die "umount not found in /bin/umount\n" if ( ! -e "/bin/umount" );
die "gzip not found in /bin/gzip\n" 	if ( ! -e "/bin/gzip" );

die "./tmp.floppy directory already exists! Please unmount it and delete it first\n" if ( -e "./tmp.floppy" );

die "/boot/stage1, /boot/grub/stage1 or $TOPDIR/config/stage1 not present\n"
   if ( (! -e "/boot/stage1") &&  (! -e "/boot/grub/stage1") &&
	(! -e "$TOPDIR/config/stage1") );

die "/boot/stage2, /boot/grub/stage2 or $TOPDIR/config/stage2 not present\n"
   if ( (! -e "/boot/stage2") && (! -e "/boot/grub/stage2") &&
	(! -e "$TOPDIR/config/stage2") );

# Just a quick, silly test to see if the user did a make all

die "$BUILD_DIR/system/kernel not found. Did you do a \"make all\" in the main source root?\n"
	if ( ! -e "$BUILD_DIR/system/kernel" );


print "Checking for sudo access... Please enter your password at the sudo prompt, if it asks you.\n";

die "sudo either doesn't like your password or your username. Are you a valid sudo user?\n"
				if ( system("/usr/bin/sudo -v") != 0 );

print "The password is correct or sudo access is still granted. Awesome.\n";

# Make the floppy image

  die "dd /dev/zero for 1.44M to ./floppy.img failed!\n" if 
  ( system("/bin/dd bs=1474560 count=1 if=/dev/zero of=./floppy.img") != 0 );
  die "making ext2 on ./floppy.img failed!\n" if 
  ( system("echo -e \"y\\n\" | /sbin/mkfs -t ext2 ./floppy.img") != 0 );
  mkdir( "./tmp.floppy" );
  die "unable to mount ./floppy.img onto ./tmp.floppy" if 
  ( system("/usr/bin/sudo /bin/mount -t ext2 -o loop ./floppy.img ./tmp.floppy") != 0 );


my $STATE	=	0;
my $GRUB_CLEAN	=	0;


  my $fd;
  	open($fd,"< $TOPDIR/config/floppy.conf") 
	or die "unable to open $TOPDIR/config/floppy.conf\n";


	while (<$fd>)
	{
	   my $line = $_;
	   chomp($line);
	
	   if ( $line =~ /\[FILES\]/ ) { $STATE = 1; }
	   elsif ( $line =~ /\[GRUB\]/ ) { $STATE = 2; }
	   elsif ( ! ($line =~ /^\s*$/) )
	   {

	     if ( $STATE == 1 )
	     {
		if ( $line =~ /^(.*)\/(.*?)$/ )
		{
		  my $path = $1;
		  # ---------- CREATE THE TREE
		  my @hier;
		  my $bob = $path;
		  	while ( $bob =~ /^(.*)\/(.*?)$/ )
			{
			  push @hier,$1;
			  $bob = $1;
			}
		 
		 	while ( $bob = pop @hier )
			{
		  		mkdir("./tmp.floppy\/".$bob);
			}
		  	mkdir("./tmp.floppy\/".$path);
		  # ------------ DONE
		
		  my $filename = $line;
		  while ( $filename =~ s/.*\/// ) { }
		  print "gzipping $line --> ./tmp.floppy\/".$path."/$filename\n";
		   
		  die "copy failed!\n" if 
		   ( system("gzip -c $BUILD_DIR/$line > ./tmp.floppy/$path/$filename") != 0 );
		}
	     }
	     elsif ( $STATE == 2) 
	     {
	     	my $GRUB;
		if ( $GRUB_CLEAN == 0 )
		{
		  mkdir("./tmp.floppy/grub/");
		  open($GRUB,"> ./tmp.floppy/grub/grub.conf")
		   or die "Unable to open ./tmp.floppy/grub/grub.conf\n";
		  $GRUB_CLEAN = 1;
		}
		else
		{
		  open($GRUB,">> ./tmp.floppy/grub/grub.conf")
		   or die "Unable to open for appending ./tmp.floppy/grub/grub.conf\n";
		}
	    	print $GRUB $line . "\n";
		close($GRUB);
	     }
	     
	   }
	
	}

	close($fd);



  copy("./tmp.floppy/grub/grub.conf","./tmp.floppy/grub/menu.lst");
  
  copy("/boot/grub/stage1","./tmp.floppy/grub/") if ( -e "/boot/grub/stage1");
  copy("/boot/stage1","./tmp.floppy/grub/") if ( -e "/boot/stage1");
  copy("$TOPDIR/config/stage1","./tmp.floppy/grub/") if ( -e "$TOPDIR/config/stage1");

  copy("/boot/grub/stage2","./tmp.floppy/grub/") if ( -e "/boot/grub/stage2");
  copy("/boot/stage2","./tmp.floppy/grub/") if ( -e "/boot/stage2");
  copy("$TOPDIR/config/stage2","./tmp.floppy/grub/") if ( -e "$TOPDIR/config/stage2");


  die "unable to unmount ./tmp.floppy" if 
  ( system("/usr/bin/sudo /bin/umount ./tmp.floppy") != 0 );
  rmdir("./tmp.floppy");

  # ---------------------------------------

   my $GD;

	if ( -e "$TOPDIR/config/grub" )
	{
		open($GD,"| $TOPDIR/config/grub") or 
		die "Unable to start piping into $TOPDIR/config/grub\n";
	}
	else
	{
		open($GD,"| /sbin/grub") or 
		die "Unable to start piping into /sbin/grub\n";
	}

	print $GD "device (fd0) $TOPDIR/floppy.img\n";
	print $GD "root (fd0)\n";
	print $GD "setup (fd0)\n";
	print $GD "quit\n";

	close($GD);

#---------------------------------------------------
print "****************************************************\n";
print "I'm pretty sure that everything worked.\n\n";
print "  ./floppy.img\n\n";
print "That's now a bootable floppy image made according\n";
print "to the ./config/floppy.conf file.\n\n";
print "If you want to write it to a real floppy:\n\n";
print "   dd if=./floppy.img of=/dev/fd0\n";
print "\n";
print "good luck.\n"


