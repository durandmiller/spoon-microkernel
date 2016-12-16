MAKEDIR=config
include $(MAKEDIR)/make.system


help:    
	@echo "Options: "
	@echo
	@echo "           all - compile the whole system and install"
	@echo "        floppy - build floppy.img"
	@echo "           iso - build cdrom.iso"
	@echo
	@echo " documentation - compile the documentation (requires doxygen)"
	@echo 
	@echo "       install - install everything"
	@echo "         clean - clean everything"
	@echo


all:
	@$(MAKE) -C kernel
	@$(MAKE) -C kernel install
	@$(MAKE) -C unix
	@$(MAKE) -C unix install
	@$(MAKE) -C devices
	@$(MAKE) -C devices install
	@$(MAKE) -C vfs
	@$(MAKE) -C vfs install
	@$(MAKE) -C gui
	@$(MAKE) -C gui install
	@$(MAKE) -C applications
	@$(MAKE) -C applications install

compile: all

documentation: FORCE
	@$(MAKE) -C documentation

FORCE:

install:
	mkdir -p $(BASE)/system
	@$(MAKE) -C kernel install
	@$(MAKE) -C unix install
	@$(MAKE) -C devices install
	@$(MAKE) -C vfs install
	@$(MAKE) -C gui install
	@$(MAKE) -C applications install



clean:
	@$(MAKE) -C kernel clean
	@$(MAKE) -C unix clean
	@$(MAKE) -C devices clean
	@$(MAKE) -C vfs clean
	@$(MAKE) -C gui clean
	@$(MAKE) -C applications clean
	@$(MAKE) -C documentation clean
	@rm -f ./debug.log
	@rm -f ./floppy.img
	@rm -f ./cdrom.iso
	

floppy:	all
	@./scripts/make_floppy.pl


iso:	floppy
	@./scripts/make_iso.pl
	


