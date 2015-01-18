all: noeuclid


SRCS=$(wildcard *.cpp)
OBJS=$(subst .cpp,.o,$(SRCS))


#If we have a static SFML, i.e. download SFML2.2 to  folder SFML-2.2/
#I got mine from here: http://www.sfml-dev.org/download/sfml/2.2/
# 64-bit: http://www.sfml-dev.org/files/SFML-2.2-linux-gcc-64-bit.tar.gz
# 32-bit: http://www.sfml-dev.org/files/SFML-2.2-linux-gcc-32-bit.tar.gz
CXXFLAGS:=-ISFML-2.2/include
LDFLAGS:=$(LDFLAGS) ./SFML-2.2/lib/libsfml-graphics.so ./SFML-2.2/lib/libsfml-window.so ./SFML-2.2/lib/libsfml-system.so
#Don't forget to run using:  LD_LIBRARY_PATH=SFML-2.2/lib ./noeuclid


#otherwise... uncomment this
#LDFLAGS:=$(LDFLAGS) -lsfml-graphics -lsfml-window -lsfml-system

CXXFLAGS:=$(CXXFLAGS) -std=c++11 -DGLEW_STATIC -DGL_GLEXT_PROTOTYPES -g -Wall -O2
LDFLAGS:=$(LDFLAGS) -g -Wall

LDLIBS:=$(LDFLAGS) $(LDLIBS) -lGLEW -lGLU -lGL -ltcc -ldl
LINK.o = $(LINK.cc)

noeuclid: $(OBJS)

clean:
	rm -f $(OBJS) noeuclid
