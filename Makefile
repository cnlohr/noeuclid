CXXFLAGS=-std=c++11 -DGLEW_STATIC -DGL_GLEXT_PROTOTYPES -g -Wall
LDFLAGS=-g -Wall
LDLIBS=-lGLEW -lGLU -lGL -lsfml-graphics -lsfml-window -lsfml-system -ltcc -ldl
LINK.o = $(LINK.cc)

SRCS=$(wildcard *.cpp)
OBJS=$(subst .cpp,.o,$(SRCS))

all: noeuclid

noeuclid: $(OBJS)

clean:
	rm -f $(OBJS) noeuclid
