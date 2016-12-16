MAKEDIR=../config
include $(MAKEDIR)/make.system



all:
	@$(MAKE) -s -C libunix empty;

clean:  
	@$(MAKE) -s -C libunix clean;


install:  
	mkdir -p $(LIBRARYROOT);
	@cp -f ./libunix/libunix.a $(LIBRARYROOT)
	@cp -rf ./libunix/include/unix/* $(INCLUDEROOT)
	@mkdir -p $(BASE)/unix;				
	@mkdir -p $(BASE)/unix/bin;			
	@mkdir -p $(BASE)/unix/lib;			
	@mkdir -p $(BASE)/unix/man;			
	@mkdir -p $(BASE)/unix/man/man1;	
	@mkdir -p $(BASE)/unix/man/man3;
	@mkdir -p $(BASE)/unix/include;		






