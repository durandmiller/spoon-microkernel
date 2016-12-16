#-nostdlib -nodefaultlibs -fno-builtin

SHARED_CFLAGS=-D_TEST_ -O3 -fPIC -Wall
INCLUDEROOT=-I../../


all: 
	gcc $(INCLUDEROOT) $(SHARED_CFLAGS) *.c ../../*.c -o ./simple


clean:
	rm -f ./*.o ./simple





