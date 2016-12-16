include ../../../includes.make


FILES = *.cpp

INSTALLPATH = /spoon/system/drivers/storage/

compile: 
	g++ $(INCLUDEROOT) $(CCFLAGS)   -c $(FILES) 
	ld *.o $(LDFLAGS) -o ramdisk

bootdisk: disk.o
	g++ -I$(INCLUDEROOT) $(CCFLAGS) -D_BOOTDISK_  -c $(FILES) 
	ld *.o $(LDFLAGS) -o ramdisk 

disk.o: memdisk
	./convert.sh

clean:
	rm -f ./*.o
	rm -f ./ramdisk

install:
	cp -f ramdisk $(INSTALLPATH)/
	sync


mount:
	mkdir tmpdir
	mount -o loop ./memdisk ./tmpdir

umount:
	umount ./tmpdir
	rmdir ./tmpdir
	touch ./memdisk

image:
	dd if=/dev/zero of=./memdisk bs=1024 count=1024

	
