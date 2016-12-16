# Please see LICENSE for licensing information.


# --------- FLAGS AND VARIABLES --------------------

CFLAGS=-O3 -nostdlib -nodefaultlibs -fno-builtin -fPIC -Wall
INCLUDEROOT=-I./


FILES=arb.c kmem_cache.c


# ---------  GENERIC MAKE RULES --------------------

all: 
	@echo "Makefile for the libarb library."
	@echo "Please see LICENSE for licensing information."
	@echo 
	@echo "Output should be: libarb.a "
	@echo "                  libarb.so"
	@echo 
	@echo "Usage: make [ compile | clean ] "
	@echo 
	@echo
	@echo "Please see the README for example usage"


clean:
	rm -f ./*.o
	rm -f ./*.a
	rm -f ./*.so

compile:
	gcc $(INCLUDEROOT) $(CFLAGS) -static -c $(FILES)
	ar -rcv libarb.a  *.o
	gcc $(INCLUDEROOT) $(CFLAGS) -shared $(FILES) -o libarb.so




