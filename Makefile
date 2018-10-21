
# This requires GNU make. You'll also need Lua 5.x to build the Lua module
# (tested with Lua 5.3).

# First run 'make checkout' to check out the submodules with the required
# BLOCKS SDK and the Littlefoot examples. Then run 'make' to build everything,
# or 'make testblocks' to just build the C test program (this doesn't need
# Lua). See testblocks.lua for information on how to run the Lua version of
# the test program (the C version of the program works the same). The C API is
# in myblocks.cc, the Lua interface in myblocks_lua.c. The generated Lua
# module is in myblocks.so after running 'make'. The testblocks.lua script
# shows how to use the provided operations.

SDKPATH = BLOCKS-SDK/SDK
SDKLIB = $(SDKPATH)/Build/Linux/Debug/libBLOCKS-SDK.a

INCLUDES = -I$(SDKPATH) $(shell pkg-config --cflags lua)
LIBS = -lpthread -ldl -lasound $(shell pkg-config --libs lua)

CC = gcc
CFLAGS = -g -O2 -fPIC
CXX = gcc
CXXFLAGS = -g -O2 -fPIC
CPPFLAGS = $(INCLUDES) -DJUCE_DEBUG=1

all: myblocks.so testblocks

myblocks.so: myblocks.o $(SDKLIB) myblocks_lua.o
	$(CXX) -shared -fPIC -o $@ $^ $(LIBS) -lstdc++

testblocks: testblocks.o myblocks.o $(SDKLIB)
	$(CXX) -g -o $@ $^ $(LIBS) -lstdc++

myblocks.o: myblocks.h

$(SDKLIB):
	$(MAKE) -C $(SDKPATH)/Build/Linux CXX="g++ -fPIC"

# Checkout the submodules (SDK etc.), you need to do this first!
checkout:
	git submodule update --init

clean:
	rm -f *.o myblocks.so testblocks

# Also get rid of the submodules.
realclean: clean
	git submodule deinit --all -f
