#-nostdlib -nodefaultlibs -fno-builtin

SHARED_CFLAGS=-D_TEST_ -O3 -fPIC -Wall -shared
SHARED_CFLAGS=-O3 -fPIC -Wall -shared
INCLUDEROOT=-I../../


all: 
	gcc $(INCLUDEROOT) $(SHARED_CFLAGS) *.c ../../*.c -o libmalloc.so


clean:
	rm -f ./*.o ./*.so





