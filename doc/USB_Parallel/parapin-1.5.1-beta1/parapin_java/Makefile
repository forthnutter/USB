# make file for parapin Java bindings, $Revision: 1.5 $

PROJECT = net_sf_parapin_ParallelPort
VERSION = `./version`
ANT=`which ant2`
PKGNAME = parapin-$(VERSION)
RELDIR = generated/$(PKGNAME)
CFLAGS = -Wall $(DEBUG)
INCLUDES = -I/usr/java/jdk/include -I/usr/java/jdk/include/linux -I..
PARTS = src/$(PROJECT).c

.SILENT:

# Targets...

build: $(PROJECT)
	ant build-examples mkrel

$(PROJECT): $(PARTS)
	[ -d $(RELDIR)/lib ] || mkdir -p $(RELDIR)/lib
	$(CC) $(CFLAGS) $(INCLUDES) -shared -o $(RELDIR)/lib/lib$(PROJECT).so $(PARTS)

install: all
	install -D $(RELDIR)/lib/$(PKGNAME).jar /usr/share/java
	install -D $(RELDIR)/lib/lib$(PROJECT).so /usr/local/lib
	install -D doc/LICENSE /usr/share/doc/$(PKGNAME)-java/copyright
	cp --archive $(RELDIR)/doc/* /usr/share/doc/$(PKGNAME)-java

clean:
	ant clean
	rm -f *.o $(RELDIR)/lib/lib$(PROJECT).so
	rm -Rf /usr/share/doc/$(PKGNAME)-java
	rm -f /usr/local/lib/lib$(PROJECT).so
	rm -f /usr/share/java/$(PKGNAME).jar
