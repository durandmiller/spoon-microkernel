include ../../../includes.make


FILES = *.cpp
INSTALLPATH = /spoon/system/drivers/video


all:	
	g++ $(INCLUDEROOT) $(CCFLAGS) -c $(FILES) 
	ld *.o  $(LDFLAGS) -o vga 


compile: all
	@echo done
	

clean:
	rm -f ./*.o
	rm -f ./vga


install:
	mkdir -p $(INSTALLPATH)
	cp -f vga $(INSTALLPATH)/vga




