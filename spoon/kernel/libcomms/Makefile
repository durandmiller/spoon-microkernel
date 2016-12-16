include ../config

LIBNAME=libcomms.a
FILES=*.c
CFLAGS=-fno-builtin -nostdinc -static -Wall 



all:	
	$(CC) -I../common -I../libsmk/include -I./include  $(CFLAGS) -c $(FILES);
	$(AR) -rc $(LIBNAME)  *.o


clean:
	rm -f $(LIBNAME) ./*.o




