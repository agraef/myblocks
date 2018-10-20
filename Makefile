
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

clean:
	rm -f *.o testblocks
