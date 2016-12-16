include ../config

LIBNAME=libsmk.a
FILES=*.c *.S ../common/*.c
CFLAGS=-fno-builtin -nostdinc -static -Wall 



all:	
	$(CC) -I../common -I./include/  $(CFLAGS) -c $(FILES);
	$(AR) -rc $(LIBNAME)  *.o


clean:
	rm -f $(LIBNAME) ./*.o




