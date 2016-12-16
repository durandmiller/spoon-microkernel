#!/usr/bin/perl -w

#  -----------------------------

use strict;


my $TOPDIR = $ENV{'PWD'};
die "Environment variable PWD is not set\n" if ( ! $TOPDIR );



if ( ! -e $TOPDIR."/scripts/make_iso.pl" ) 
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



my $COMMAND = "/usr/bin/mkisofs -f -l -A \"spoon install CD\"  -V \"spoon install CD\" -root /  -b system/floppy.img -o $TOPDIR/cdrom.iso $BUILD_DIR"; 


# -----------------------------



die "mkisofs not found in /usr/bin/mkisofs\n" 	if ( ! -e "/usr/bin/mkisofs" );

die "floppy.img not found in correct path ($TOPDIR).\n" .
    "Run make_floppy.pl in the scripts directory first.\n" 	
	if ( (! -e "$TOPDIR/floppy.img") );
	
# Make the iso image image

  system("cp -f $TOPDIR/floppy.img $BUILD_DIR/system/" );

  die "mkisofs failed! dunno why.\n" if 
  ( system($COMMAND) != 0 );

  system("rm -f $BUILD_DIR/system/floppy.img" );

print "\n";
print "****************************************************\n";
print "You should be able to boot the iso now and use\n";
print "the system. As always, let me know if you have any\n";
print "problems or just tell me what you think.\n\n";
print "\n";
print "./cdrom.iso\n";
print "\n";
print "good luck.\n";


