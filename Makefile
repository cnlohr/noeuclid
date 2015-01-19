<<<<<<< HEAD
CXXFLAGS=-std=c++11 -DGLEW_STATIC -DGL_GLEXT_PROTOTYPES -DGLUT_DISABLE_ATEXIT_HACK -DGL_GLEXT_PROTOTYPES -g -Wall -O2
LDFLAGS=-g -Wall
LDLIBS=-lGLEW -lglut -lGLU -lGL -ltcc -ldl
LINK.o = $(LINK.cc)
=======
all: noeuclid

>>>>>>> SFML

SRCS=$(wildcard *.cpp)
OBJS=$(subst .cpp,.o,$(SRCS))

<<<<<<< HEAD
all: noeuclid
=======
#otherwise... uncomment this
LDFLAGS:=$(LDFLAGS) -lsfml-graphics -lsfml-window -lsfml-system

CXXFLAGS:=$(CXXFLAGS) -std=c++11 -DGLEW_STATIC -DGL_GLEXT_PROTOTYPES -g -Wall -O2
LDFLAGS:=$(LDFLAGS) -g -Wall

LDLIBS:=$(LDFLAGS) $(LDLIBS) -lGLEW -lGLU -lGL -ltcc -ldl
LINK.o = $(LINK.cc)
>>>>>>> SFML

noeuclid: $(OBJS)

clean:
	rm -f $(OBJS) noeuclid
