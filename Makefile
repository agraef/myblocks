
SDKPATH = BLOCKS-SDK/SDK
SDKLIB = $(SDKPATH)/Build/Linux/Debug/libBLOCKS-SDK.a

INCLUDES = -I$(SDKPATH)
LIBS = -lpthread -ldl -lasound

CFLAGS = -g -O2
CPPFLAGS = $(INCLUDES) -DJUCE_DEBUG=1

all: testblocks

testblocks: testblocks.o myblocks.o $(SDKLIB) 
	g++ -g -o $@ $^ $(LIBS)

$(SDKLIB):
	$(MAKE) -C $(SDKPATH)/Build/Linux

# Checkout the submodules (SDK etc.), you need to do this first!
checkout:
	git submodule update --init

clean:
	rm -f *.o testblocks

# Also get rid of the submodules.
realclean: clean
	git submodule deinit --all -f
