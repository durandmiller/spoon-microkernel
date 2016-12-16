MAKEDIR=../../../../config
include $(MAKEDIR)/make.base

# I put this together from the make file config system I have. That's the story.

LIBRARYROOT= $(BASE)/system/lib
INCLUDEROOT= -I$(BASE)/system/include	-I../../include/ 
CFLAGS = -g -fno-builtin -nostdinc -static -Wall -march=i686 -L$(LIBRARYROOT)



all:
		gcc $(INCLUDEROOT) $(CFLAGS) -c *.c *.S;
		mv ./*.o ../../




