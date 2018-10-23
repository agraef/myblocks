
# This requires GNU make and pkg-config. You'll also need Lua 5.x to build the
# Lua module (tested with Lua 5.3).

# First run 'make checkout' to check out the submodule with the required
# BLOCKS SDK. Then run 'make' to build everything, or 'make testblocks' to
# just build the C test program (this doesn't need Lua). The C API is in
# myblocks.cc, the Lua interface in myblocks_lua.c. The generated Lua module
# is in myblocks.so after running 'make'.

# The testblocks.lua script shows how to use the provided operations. Also see
# the comments at the beginning of the script for instructions on how to use
# the test program (the C version of the program works the same).

# There's also a Pd external blocks.pd_lua available, along with a help patch
# blocks-help.pd. Pd-Lua is required to run this. You can also install the
# external (along with the help patch and the Lua module) with 'make install'.

# NOTE: The SDK will be built automatically, but you can also run 'make SDK'
# to have it built beforehand. Also, you can specify 'CONFIG=Release' if you
# want to build the release version of the SDK (CONFIG=Debug is the default).
# If for some reason you prefer to build the SDK manually, make sure that the
# SDK modules are compiled with -fPIC, otherwise building the Lua module will
# fail. The 'SDK' target takes care of all this automatically.

# Set this to Release for a release build of the SDK.
CONFIG = Debug

SDKPATH = BLOCKS-SDK/SDK
SDKLIB = $(SDKPATH)/Build/Linux/$(CONFIG)/libBLOCKS-SDK.a

INCLUDES = -I$(SDKPATH)
LIBS = -lpthread -ldl -lasound -lm
TARGETS = testblocks

# Check to see whether we have Lua installed. We only try to build the Lua
# module if that's the case.
LUALIBS := $(shell pkg-config --libs lua 2>/dev/null)
ifneq ($(LUALIBS),)
INCLUDES += $(shell pkg-config --cflags lua)
LIBS += $(LUALIBS)
TARGETS += myblocks.so
endif

CC = gcc
CFLAGS = -g -O2 -fPIC
CXX = gcc
CXXFLAGS = -g -O2 -fPIC
CPPFLAGS = $(INCLUDES)

ifeq ($(CONFIG),Release)
CPPFLAGS += -DJUCE_DEBUG=0
else
CPPFLAGS += -DJUCE_DEBUG=1
endif

# Compile targets for the test program and Lua module.
all: $(TARGETS)

myblocks.so: myblocks.o $(SDKLIB) myblocks_lua.o
	$(CXX) -shared -fPIC -o $@ $^ $(LIBS) -lstdc++

testblocks: testblocks.o myblocks.o $(SDKLIB)
	$(CXX) -g -o $@ $^ $(LIBS) -lstdc++

myblocks.o: myblocks.h

# Compile the SDK.
SDK: $(SDKLIB)

$(SDKLIB):
	$(MAKE) -C $(SDKPATH)/Build/Linux DEPFLAGS="-fPIC" CONFIG=$(CONFIG)

# Check out the SDK submodule. You need to do this first!
checkout:
	git submodule update --init

# Clean targets.
clean:
	rm -f *.o myblocks.so testblocks

# Also clean the SDK.
realclean: clean
	$(MAKE) -C $(SDKPATH)/Build/Linux CONFIG=$(CONFIG) clean

# Also get rid of the submodule.
distclean: clean
	git submodule deinit --all -f

# Installation of the Pd external. Adjust the variables below as needed.
prefix = /usr
installdir = $(prefix)/lib/pd-externals/blocks
installfiles = blocks.pd_lua blocks-*.pd myblocks.so examples

install:
	mkdir -p $(DESTDIR)$(installdir)
	cp -r $(installfiles) $(DESTDIR)$(installdir)

uninstall:
	rm -rf $(DESTDIR)$(installdir)
