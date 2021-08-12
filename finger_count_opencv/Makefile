INCLUDE_DIRS = 
LIB_DIRS = 
CC=g++

CDEFS=
#CFLAGS= -O3 -mssse3 $(INCLUDE_DIRS) $(CDEFS)
CFLAGS= -O3 -MMD -pipe -std=gnu++17 -g $(INCLUDE_DIRS) $(CDEFS)
#CFLAGS= -O0 -g $(INCLUDE_DIRS) $(CDEFS)
LIBS= -lrt -pthread
CPPLIBS= -L/usr/lib -lopencv_core -lopencv_flann -lopencv_video -Wno-literal-suffix

HFILES= 
CFILES= 
CPPFILES= finger_count.cpp finger_count_speedup.cpp finger_count_speedup2.cpp

SRCS= ${HFILES} ${CFILES}
CPPOBJS= ${CPPFILES:.cpp=.o}

all:	finger_count finger_count_speedup finger_count_speedup2 

clean:
	-rm -f *.o *.d cvtest*.ppm cvtest*.pgm test*.ppm test*.pgm
	-rm -f finger_count
	-rm -f finger_count_speedup
	-rm -f finger_count_speedup2
#-rm -f prototype_vid

distclean:
	-rm -f *.o *.d

finger_count: finger_count.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o `pkg-config --libs opencv` $(CPPLIBS) $(LIBS)

finger_count_speedup: finger_count_speedup.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o `pkg-config --libs opencv` $(CPPLIBS) $(LIBS)
	
finger_count_speedup2: finger_count_speedup2.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o `pkg-config --libs opencv` $(CPPLIBS) $(LIBS)


depend:

.c.o:
	$(CC) $(CFLAGS) -c $<

.cpp.o:
	$(CC) $(CFLAGS) -c $<
