


all:
	@echo "Attempting to compile the documentation"
	mkdir -p ./docs/
	doxygen doxy.liballoc
	doxygen doxy.libcomms
	doxygen doxy.libsmk
	doxygen doxy.devfs
	doxygen doxy.vfs
	doxygen doxy.gui
	doxygen doxy.libunix
	cat ./misc/alpha.html > ./docs/index.html
	cat ./misc/index.html >> ./docs/index.html
	cat ./misc/omega.html >> ./docs/index.html
	cat ./misc/alpha.html > ./docs/makefiles.html
	cat ./misc/makefiles.html >> ./docs/makefiles.html
	cat ./misc/omega.html >> ./docs/makefiles.html
	cp ./misc/spoon.png ./docs/spoon.png
	cp ./misc/main.css ./docs/main.css



clean:
	rm -rf ./docs/


install:
	rm -rf /var/www/localhost/htdocs/documentation/*
	cp -rf ./docs/* /var/www/localhost/htdocs/documentation/




