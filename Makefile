
INCLUDES = -ISDK
LIBS = -LSDK/Build/Linux/Debug -lBLOCKS-SDK -lpthread -ldl -lasound

CFLAGS = -g -O2
CPPFLAGS = $(INCLUDES) -DJUCE_DEBUG=1
LDFLAGS = $(LIBS)

all: testblocks

testblocks: testblocks.o myblocks.o
	g++ -g -o $@ $^ $(LIBS)

clean:
	rm -f *.o testblocks
