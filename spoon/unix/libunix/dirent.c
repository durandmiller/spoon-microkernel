#include <stdlib.h>
#include <dirent.h>

#include "os/os.h"

int            closedir(DIR *dir)
{
	os_freakout( "none of the dirent.h(*) are implemented" );
	return -1;
}


DIR           *opendir(const char *dir)
{
	os_freakout( "none of the dirent.h(*) are implemented" );
	return NULL;
}

struct dirent *readdir(DIR *dir)
{
	os_freakout( "none of the dirent.h(*) are implemented" );
	return NULL;
}


int            readdir_r(DIR *dir, struct dirent *entry, struct dirent **result)
{
	os_freakout( "none of the dirent.h(*) are implemented" );
	return -1;
}

void           rewinddir(DIR *dir)
{
	os_freakout( "none of the dirent.h(*) are implemented" );
}

void           seekdir(DIR *dir, long int offset)
{
	os_freakout( "none of the dirent.h(*) are implemented" );
}

long int       telldir(DIR *dir)
{
	os_freakout( "none of the dirent.h(*) are implemented" );
	return -1;
}


