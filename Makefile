CXXFLAGS=-std=c++11 -DGLEW_STATIC -DGL_GLEXT_PROTOTYPES -DGLUT_DISABLE_ATEXIT_HACK -DGL_GLEXT_PROTOTYPES -g -Wall -O2
LDFLAGS=-g -Wall
LDLIBS=-lGLEW -lglut -lGLU -lGL -ltcc -ldl
LINK.o = $(LINK.cc)

SRCS=$(wildcard *.cpp)
OBJS=$(subst .cpp,.o,$(SRCS))

all: noeuclid

noeuclid: $(OBJS)

clean:
	rm -f $(OBJS) noeuclid
