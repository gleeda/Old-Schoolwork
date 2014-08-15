CC  =   gcc
CCFLAGS =   -fPIC -c -pthread

LDCONFIG = /sbin/ldconfig -n `pwd`

MAJOR = so
MINOR = 2
RELEASE = 2
VERSION = $(MAJOR).$(MINOR).$(RELEASE)
SETLPERMISSIONS = chmod 755

METALROOT = /usr
LIBDIR = $(METALROOT)/lib


METAL_BASENAME = libmetal
METAL_SONAME = $(METAL_BASENAME).$(MAJOR)
METALLIB = $(METAL_SONAME).$(MINOR)

OBJECT = metal.o

BUILDLIB = $(CC) -shared -Wl,-soname,$(METAL_SONAME) \
 -o $(METALLIB)

lib-post-install:
	@$(LDCONFIG) || echo Warning: "$(LDCONFIG)" was unsuccessful

metal.o:	metal.c	metal.h
	$(CC)	$(CCFLAGS)	metal.c

all:	metal.o
	$(BUILDLIB)	$(OBJECT)	-lc

	
install:    all
	$(MAKE)	lib-post-install
	mv	$(METALLIB)     $(LIBDIR)
	mv	$(METAL_SONAME)	$(LIBDIR)
	$(SETLPERMISSIONS)      $(LIBDIR)/$(METALLIB)

clean: 
	rm	-rf	*.o	*.so*
