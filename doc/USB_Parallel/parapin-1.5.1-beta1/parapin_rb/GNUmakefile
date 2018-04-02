# make file for parapin Ruby bindings, $Revision: 1.1 $

.SILENT:

# Targets...

build: $(PROJECT)
	# the following command will generate a file called "Makefile"
	ruby extconf.rb
	make -f Makefile

install: build
	# TODO

clean:
	rm -f parapin.so parapin.o Makefile
