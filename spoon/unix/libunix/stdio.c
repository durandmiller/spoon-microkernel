#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <support/support.h>

#include "os/os.h"


FILE __decl_stdin  = { 0, (void*)0, 0, 0, 0, 0 };
FILE __decl_stdout = { 1, (void*)1, 0, 0, 0, 0 };
FILE __decl_stderr = { 2, (void*)2, 0, 0, 0, 0 };



FILE *stdin = &__decl_stdin;
FILE *stdout = &__decl_stdout;
FILE *stderr = &__decl_stderr;


/** Returns the integer file descriptor of the stream. If the stream
 * is invalid, it should return -1 and set errno to EBADF
 */
int      fileno(FILE *file)
{
	if ( file == NULL )
	{
		errno = EBADF;
		return -1;
	}

	return file->fd;
}


FILE* 	fopen(const char* filename, const char* mode)
{
	int fd;
	unsigned int flags = support_fmodes2flags(mode);

	fd = open( filename, flags, S_DEFAULT );
	if ( fd < 0 ) return NULL;

	return support_retrieve_file( fd );
}


FILE* 	freopen(const char* filename, const char* mode, FILE* stream)
{
	int fd;
	int rc;
	void *data = stream->data;
	unsigned int flags = support_fmodes2flags(mode);
	

	if ( os_close( data, &rc ) == NOTIMPLEMENTED ) 
			os_freakout( "os_close not implemented" );
	
	if ( rc != 0 ) 
	{
		free( stream );
		return NULL;
	}
	

	if ( os_open( filename, flags, 
			S_DEFAULT,
			&data, &fd ) == NOTIMPLEMENTED ) 
			os_freakout( "os_open not implemented" );
	if ( fd < 0 )
	{
		free( stream );
		return NULL;
	}

	stream->data 		= data;
	stream->fd 		= fd;
	stream->error 		= 0;
	stream->eof 		= 0;
	stream->pos		= 0;
	stream->mode 		= flags;
		
	return stream;
}



 
int 	fflush(FILE* stream)
{
	return 0;
}


char    *fgets(char *s, int size, FILE *stream)
{
	int count = 0;

	while ( count < (size-1) )
	{
		s[count] = (char)fgetc( stream );


		if ( s[count] == EOF ) break;
		count += 1;
		if ( s[count-1] == '\n' ) break;
	}

	s[count] = 0;
	if ( count == 0 ) return NULL;
	return s;
}



 
int 	fclose(FILE* stream)
{
	return close( stream->fd );
}


FILE    *fdopen(int fd, const char *mode)
{
	/// \todo Honour mode changes.
	
	return support_retrieve_file( fd );
}
 
int 	remove(const char* filename)
{
	int rc;

	if ( os_delete( filename, &rc ) == NOTIMPLEMENTED )
			os_freakout( "os_delete not implemented" );
		
	return rc;
}


int      rename(const char *oldpath, const char *newpath)
{
#warning	implement me
	os_freakout( "rename has not been implemented" );
	return -1;
}

 

 
FILE* 	tmpfile()
{
	char name[L_tmpnam];

	if ( tmpnam( name ) != name ) return NULL;
	
	/// \todo Ensure that this tmpfile gets deleted when the application closes or the file is closed.
	return fopen( name, "wb+" );
}

 
char* 	tmpnam(char s[L_tmpnam])
{
	char m[L_tmpnam];
	char *buf;
	int i;

	if ( s != NULL ) buf = s;
				else buf = m;
	

	for ( i = 0; i < L_tmpnam - 1; i++ )
	{
		char c = rand() % 52;

		if ( c < 26 ) buf[i] = 'a' + c;	
				else  buf[i] = 'A' + (c-26);	
	}
		
	buf[ L_tmpnam - 1] = 0;
	return buf;
}

 
 

 
 
int 	fputc(int c, FILE* stream)
{
	int rc;
	unsigned char ch = (unsigned char)c;
	
	if ( os_write( stream->data, stream->pos, 
						&ch, 1, &rc ) == NOTIMPLEMENTED )
					os_freakout( "os_write not implemented" );
	
	if ( rc != 1 )
	{
#warning set errno
		stream->error = 1;
		return EOF;
	}

#warning on success, mark st_ctime and st_mtime for update when the next successful completion of fflush or close is done or exit or abort.
	stream->pos += 1;
	return c;
}

int      fputs(const char *str, FILE *stream)
{
	int rc;
	int len = strlen(str);

	if ( os_write( stream->data, stream->pos, str, len, &rc ) == NOTIMPLEMENTED )
			os_freakout( "os_write not implemented" );

	if ( rc < len ) 
	{
#warning set errno
		stream->error = 1;
		return EOF;
	}

#warning on success, mark st_ctime and st_mtime for update when the next successful completion of fflush or close is done or exit or abort.
	stream->pos += rc;
	return rc;
}




size_t 	fread(	void* /* restrict */ ptr, size_t size, 
				size_t nmemb, FILE* /* restrict */ stream)
{
	int rc = 0;
	int alreadyRead = 0;
	ssize_t totalSize = size * nmemb;

	if ( feof( stream ) != 0 ) 
		return EOF;

	// C99 sanity check.
	if ( size == 0 ) return 0;
	if ( nmemb == 0 ) return 0;
	if ( totalSize <= 0 ) return 0;
	

	// Support a single pushback from ungetc
	if ( stream->pushedBack != EOF )
	{
		((char*)ptr)[0] = (char)(stream->pushedBack);
		ptr += 1;

		stream->pushedBack = EOF;
		stream->pos += 1;

		alreadyRead = 1;

		// Just in case they only wanted 1.
		if ( (totalSize - alreadyRead) == 0 )
			return (totalSize - alreadyRead) / size;
	}

	if ( os_read( stream->data, stream->pos, 
					ptr, (totalSize-alreadyRead), &rc ) == NOTIMPLEMENTED )
					os_freakout( "os_read not implemented" );

	if ( (rc == EOF) || ((rc+alreadyRead) == 0) )
	{
		stream->eof = 1;
		return 0;
	}

	if ( (rc+alreadyRead) != totalSize )
	{
		stream->eof = 1;
	}

	// Apparently successful.
	stream->pos += rc;

	return ((rc+alreadyRead)/size);
}

size_t 	fwrite(const void* /* restrict */ ptr, size_t size, size_t nmemb, FILE* /* restrict */ stream)
{
	int rc;
	size_t totalSize = size * nmemb;

	// C99 sanity check.
	if ( size == 0 ) return 0;
	if ( nmemb == 0 ) return 0;
	if ( totalSize <= 0 ) return 0;
	
	if ( os_write( stream->data, stream->pos, 
						ptr, totalSize, &rc ) == NOTIMPLEMENTED )
					os_freakout( "os_write not implemented" );
	
	if ( (rc == EOF) || (rc == 0) )
	{
		stream->eof = 1;
		return EOF;
	}

	if ( rc != totalSize )
	{
#warning confirm this action
		stream->error = 1;
		return EOF;
	}

	stream->pos += rc;
	return (rc/size);
}	



int      getc(FILE *stream)
{
	return fgetc(stream);
}

int      getchar(void)
{
	return getc(stdin);
}



int 	fseek(FILE* stream, long offset, int origin)
{
	int rc = -1;


	switch (origin)
	{
		case SEEK_SET:
		case SEEK_CUR:
		case SEEK_END:
			break;
	}
	

	return rc;
}


long int ftell(FILE* stream)
{
	return stream->pos;
}
 
 
void 	rewind(FILE* stream)
{
	fseek(stream, 0L, SEEK_SET); 
	stream->error = 0;
}

 

void 	clearerr(FILE* stream)
{
	stream->error = 0;
	stream->eof = 0;
}


int		feof(FILE* stream)
{
	if ( stream->eof != 0 ) return 1;
	return 0;
}


int		ferror(FILE* stream)
{
	if ( stream->error != 0 ) return 1;
	return 0;
}


int      fgetc(FILE *stream)
{
	unsigned char c;
	int rc = fread( &c, 1, 1, stream );

	if ( rc != 1 )
		return EOF;

	return (int)c;
}

 
void 	perror(const char* s)
{
	fprintf(stderr, "%s: %s\n", (s != NULL ? s : ""), strerror(errno));
}

 

int      putc(int c, FILE *stream)
{
	return fputc( c, stream );
}

int      putchar(int c)
{
	return putc( c, stdout );
}

int      puts(const char *s)
{
	if ( fputs( s, stdout ) == EOF ) return EOF;
	if ( fputs( "\n", stdout ) == EOF ) return EOF;
	return 1;
}



int      ungetc(int c, FILE *stream)
{
	if ( stream->pushedBack != EOF )
		os_freakout( "ungetc called multiple times without a read. implement this support for multiple push backs, please." );

	stream->pushedBack = c;

#warning can you push back on a new stream?
	stream->pos -= 1;

	stream->error = 0;
	stream->eof = 0;		// Makes sense, right?
	return c;
}





