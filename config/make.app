include $(MAKEDIR)/make.system


AUTHOR=Durand Miller
EMAIL=clutter@djm.co.za

CODE="C"
FILES=*.c




all:	
	@if [ $(CODE) == "C" ]; then 						\
		gcc -I$(INCLUDEROOT) $(CFLAGS) -c $(FILES);		\
	elif [ $(CODE) == "C++" ]; then 					\
		g++ -I$(INCLUDEROOT) $(CCFLAGS) -c  $(FILES);		\
	fi;
	@ld *.o $(LDFLAGS) $(EXTRA_LDFLAGS) -o $(APPNAME);					


info:
	@echo $(DESCRIPTION)
	@echo $(AUTHOR) \<$(EMAIL)\>
	@echo
	@echo Resulting binary: $(APPNAME)
	@echo Installation directory: $(INSTALLPATH)
	@echo



clean:
	@rm -f ./*.o ./$(APPNAME)


install:
	@if [ ! -d $(INSTALLPATH) ]; then \
		mkdir -p $(INSTALLPATH);		\
	fi;
	@cp -f ./$(APPNAME) $(INSTALLPATH)/$(APPNAME)



	

