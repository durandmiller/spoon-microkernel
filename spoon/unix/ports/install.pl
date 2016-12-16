#!/usr/bin/perl -w

use strict;


my $NAME;
my $DESCRIPTION;
my $LOCATION;
my $FILE;
my $EXTRACTION;



my $request = shift;


mkdir("downloads");
mkdir("cache");

	load_list();


	exit(0);


sub load_list
{
	open(FD,"< ./supported.lst")
		or die "Unable to open supported list";

	while (<FD>)
	{
		my $line = $_;
		chomp($line);

		$line =~ s/#.*//g;

		if ( $line =~ /^(.*?),(.*?),(.*)\/(.*),(.*)\s*$/ )
		{
			$NAME = $1;
			$DESCRIPTION = $2;
			$LOCATION = $3."\/".$4;
			$FILE = $4;
			$EXTRACTION = $5;

			process_package();
		}
	}

	close(FD);
}


sub process_package
{
	fetch_app();
	die "$NAME: Unable to get source package\n" if ( download_app() != 0 );
	die "$NAME: Unable to extract source package\n" if ( extract_app() != 0 );
	die "$NAME: Unable to patch source\n" if ( patch_app() != 0 );
	die "$NAME: Unable to compile application\n" if ( compile_app() != 0 );
}



sub fetch_app
{
	return 0 if (   -e "cache/$EXTRACTION.downloaded" );

	if ( -e "/usr/portage/distfiles/$FILE" )
	{
		print "Fetching $NAME ($DESCRIPTION) - $FILE\n";
		my $result = system( "cd downloads && cp -f /usr/portage/distfiles/$FILE ./ && touch ../cache/$EXTRACTION.downloaded" ); 
		print "Failed to fetch $FILE\n" if ( $result != 0 );
		return $result;
	}
	
	
	if ( -e "/usr/portage/distfiles/$EXTRACTION.tar.bz2" )
	{
		print "Fetching $NAME ($DESCRIPTION) - $FILE (BZ2)\n";
		my $result = system( "cd downloads && bunzip2 -c /usr/portage/distfiles/$EXTRACTION.tar.bz2 | gzip -c > ./$FILE && touch ../cache/$EXTRACTION.downloaded" ); 
		print "Failed to fetch $FILE\n" if ( $result != 0 );

		return $result;
	}

	return -1;
}

sub download_app
{
	return 0 if ( -e "cache/$EXTRACTION.downloaded" );

	print "Downloading $NAME ($DESCRIPTION) - $FILE\n";
	my $result = system( "cd downloads && wget -c -N $LOCATION && touch ../cache/$EXTRACTION.downloaded" ); 
	print "Failed to download $FILE\n" if ( $result != 0 );

	return $result;
}


sub extract_app
{
	return -1 if ( ! -e "cache/$EXTRACTION.downloaded" );
	return 0 if (   -e "cache/$EXTRACTION.extracted" );

	my $flags = "-xzvf";

	$flags = "-xjvf" if ( $FILE =~ /[.]bz2/ );

	print "Extracting $NAME\n";
	my $result = system( "cd cache && tar $flags ../downloads/$FILE && touch $EXTRACTION.extracted" ); 
	die "Unable to extract $FILE\n" if ( $result != 0 );

	return $result;
}


sub patch_app
{
	return -1 if ( ! -e "cache/$EXTRACTION.downloaded" );
	return -1 if ( ! -e "cache/$EXTRACTION.extracted" );
	return 0 if ( ! -e "patches/$EXTRACTION" );
	return 0 if (   -e "cache/$EXTRACTION.patched" );


	print "Patching $NAME ($DESCRIPTION)\n";
	my $result = system( "cd cache && patch -p0 < ../patches/$EXTRACTION && touch $EXTRACTION.patched" ); 
	die "Unable to extract $FILE\n" if ( $result != 0 );

	return $result;
}


sub compile_app
{
	return -1 if ( ! -e "cache/$EXTRACTION.downloaded" );
	return -1 if ( ! -e "cache/$EXTRACTION.extracted" );
	return -1 if ( (-e "patches/$EXTRACTION") && (! -e "cache/$EXTRACTION.patched") );
	return 0 if (   -e "cache/$EXTRACTION.compiled" );



	print "Compiling $NAME\n";
	my $result = system( "cd cache && ../../../scripts/PORT.sh ./$EXTRACTION && touch $EXTRACTION.compiled" ); 
	die "Unable to compile $FILE\nThis is not a supported application!" if ( $result != 0 );

	return $result;
}






