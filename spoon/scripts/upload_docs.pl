#!/usr/bin/perl -w

#  This is a little script which will upload the documentation to
#  the website.


use strict;
use File::Copy;

my $TOPDIR = $ENV{'PWD'};
my $LFTP = `which lftp`;
my $SERVER = shift;
my $USERNAME = shift;

my $REMOTE_DIR = "htdocs/spoon/documentation";
my $LOCAL_DIR = "./documentation/docs";



die "Environment variable PWD is not set\n" 
		if ( ! $TOPDIR );

die "Unable to find lftp\n" 
		if ( ! $LFTP );

die "usage:  script.pl <server> <username>\n"
		if ( (! $SERVER) || (! $USERNAME) );


if ( ! -e $TOPDIR."/scripts/upload_docs.pl" ) 
{
	print "This isn't the main root of the spoon source tree.\n";
	print "Please go to the main root and run this script again.\n";
	print "\n";
	print "by running:  ./scripts/make_floppy.pl\n";
	exit -1;
}

chomp( $LFTP );

print "Using lftp at $LFTP with $SERVER, $USERNAME\n";



my $COMMAND = $LFTP." -c \"open ".$SERVER."; user ".$USERNAME."; cd ".$REMOTE_DIR."; lcd ".$LOCAL_DIR."; mirror -n --delete --reverse ./ ./;\"";


system( $COMMAND );





