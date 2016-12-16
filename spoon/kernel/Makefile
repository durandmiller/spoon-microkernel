MAKEDIR=../config
include $(MAKEDIR)/make.system

include ./config


SUBS=src libsmk libcomms

all:
	@$(foreach dir,$(SUBS), $(MAKE) -C $(dir); )  


clean:
	@$(foreach dir,$(SUBS), $(MAKE) -C $(dir) clean; )  


install:
	mkdir -p $(INCLUDEROOT)
	mkdir -p $(SYSTEMROOT)
	mkdir -p $(LIBRARYROOT)
	cp -rf ./common/* $(INCLUDEROOT)
	cp -rf ./libsmk/include/* $(INCLUDEROOT)
	cp -rf ./libcomms/include/* $(INCLUDEROOT)
	cp -rf ./src/$(KERNEL_BIN) $(SYSTEMROOT)
	cp -rf ./libcomms/libcomms.a $(LIBRARYROOT)
	cp -rf ./libsmk/libsmk.a $(LIBRARYROOT)


	


