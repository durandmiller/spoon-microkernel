# Makefile for the UNIX library.
# Please see LICENSE for licensing information.
# 


# ---------	CURRENTLY SUPPORTED PLATFORMS ---------- 

PLATFORMS=`find ./os/ -type d | sed "s/.*\///" | sort`

# --------- FLAGS AND VARIABLES --------------------

CFLAGS=-D__i386__ -O2 -nostdlib -nodefaultlibs -nostdinc -fno-builtin -static -Wall 
INCLUDEROOT=-I./os/$@  -I./include/unix/

STDFILES=*.c lsb/*.c support/*.c sys/*.c math/*.c pthread/*.c nonstd/trio/*.c


# ---------  GENERIC MAKE RULES --------------------

all:
	@echo "Makefile for the UNIX library."
	@echo "Please see LICENSE for licensing information."
	@echo 
	@echo "Include files: ./include/unix/ "
	@echo "Output should be: libunix.a "
	@echo 
	@echo "Usage: make [ all | clean | <platform> ] "
	@echo 
	@echo "Currently supported platforms:"
	@echo
	@echo $(PLATFORMS)
	@echo
	@echo


clean:
	rm -f ./*.o ./*.a


# -------------- OS DEPENDENT BUILDS ---------------------------


%: 
	@if [ ! -d ./os/$@ ]; then echo "There is no such platform. Supported platforms are: $(PLATFORMS)"; exit 255; fi;
	@if [ -e ./os/$@/Makefile ]; then														\
			echo "Platform specific makefile for $@ found. Using it.";						\
			gcc $(INCLUDEROOT) -I./os/$@/ $(CFLAGS) $(STDFILES) -c;	  			\
			make -C ./os/$@;																\
	else																					\
			gcc $(INCLUDEROOT) -I./os/$@/ $(CFLAGS) $(STDFILES) os/$@/*.c -c;	\
	fi;
	ar -rc libunix.a  *.o;															
		







