include ../config



FILES = *.c ds/*.c arb/*.c *.S ../common/*.c

LINKER_SCRIPT=-Xlinker --script=kernel.ld 


CFLAGS= -O0 -static -Wall -nostartfiles -nostdlib 		\
		-nodefaultlibs -nostdinc -fno-builtin   		\
		-I../common/ -DKERNEL_VERSION="\"$(KERNEL_VERSION)\""


all:   
	$(CC) $(CFLAGS) $(FILES) $(LINKER_SCRIPT) -o $(KERNEL_BIN)


clean:
	rm -f $(KERNEL_BIN)


test:
	cd tests && ./runtests.pl



