# Please see LICENSE for licensing information.


# --------- FLAGS AND VARIABLES --------------------

CFLAGS=-O2 -nostdlib -nodefaultlibs -fno-builtin -fPIC -Wall 
INCLUDEROOT=-I./



# ---------  GENERIC MAKE RULES --------------------

all: 
	@echo "Makefile for the liballoc library."
	@echo "Please see LICENSE for licensing information."
	@echo 
	@echo "Output should be: liballoc.a "
	@echo "                  liballoc.so"
	@echo 
	@echo "Usage: make [ compile | clean | <platform> ] "
	@echo 
	@echo "Currently supported platforms:"
	@echo 
	@echo "      linux"
	@echo "      linux_info"
	@echo "      linux_debug"
	@echo
	@echo
	@echo "Please see the README for example usage"


clean:
	rm -f ./*.o
	rm -f ./*.a
	rm -f ./*.so

compile:
	gcc $(INCLUDEROOT) $(CFLAGS) -static -c liballoc.c
	ar -rcv liballoc.a  *.o
	gcc $(INCLUDEROOT) $(CFLAGS) -shared liballoc.c -o liballoc.so

linux:
	gcc $(INCLUDEROOT) $(CFLAGS) -static -c liballoc.c linux.c
	ar -rcv liballoc.a  *.o
	gcc $(INCLUDEROOT) $(CFLAGS) -shared liballoc.c linux.c -o liballoc.so

linux_info:
	gcc -DINFO $(INCLUDEROOT) $(CFLAGS) -static -c liballoc.c linux.c
	ar -rcv liballoc.a  *.o
	gcc -DINFO $(INCLUDEROOT) $(CFLAGS) -shared liballoc.c linux.c -o liballoc.so
	@echo WARNING: Generally, printing debugging and information causes liballoc to become unstable. This may not work.


linux_debug:
	gcc -DDEBUG $(INCLUDEROOT) $(CFLAGS) -static -c liballoc.c linux.c
	ar -rcv liballoc.a  *.o
	gcc -DDEBUG $(INCLUDEROOT) $(CFLAGS) -shared liballoc.c linux.c -o liballoc.so
	@echo WARNING: Generally, printing debugging and information causes liballoc to become unstable. This may not work.



