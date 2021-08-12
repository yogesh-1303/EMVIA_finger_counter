INCLUDE_DIRS = 
LIB_DIRS = 
CC=g++

CDEFS=

CFLAGS= -O0 -g $(INCLUDE_DIRS) $(CDEFS)

LIBS=
CPPLIBS= -L/usr/lib -lopencv_core -lopencv_flann -lopencv_video -Wno-literal-suffix

HFILES= 
CFILES= 
CPPFILES= skeletal_grndup.cpp

SRCS= ${HFILES} ${CFILES}
CPPOBJS= ${CPPFILES:.cpp=.o}

all:	skeletal_grndup

clean:
	-rm -f *.o *.d cvtest*.ppm cvtest*.pgm test*.ppm test*.pgm
	-rm -f skeletal_grndup

distclean:
	-rm -f *.o *.d

skeletal_grndup: skeletal_grndup.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o `pkg-config --libs opencv` $(CPPLIBS) $(LIBS)


depend:

.c.o:
	$(CC) $(CFLAGS) -c $<

.cpp.o:
	$(CC) $(CFLAGS) -c $<
