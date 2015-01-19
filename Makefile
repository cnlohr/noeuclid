all: noeuclid


SRCS=$(wildcard *.cpp)
OBJS=$(subst .cpp,.o,$(SRCS))

#otherwise... uncomment this
LDFLAGS:=$(LDFLAGS) -lsfml-graphics -lsfml-window -lsfml-system

CXXFLAGS:=$(CXXFLAGS) -std=c++11 -DGLEW_STATIC -DGL_GLEXT_PROTOTYPES -g -Wall -O2
LDFLAGS:=$(LDFLAGS) -g -Wall

LDLIBS:=$(LDFLAGS) $(LDLIBS) -lGLEW -lGLU -lGL -ltcc -ldl
LINK.o = $(LINK.cc)

noeuclid: $(OBJS)

clean:
	rm -f $(OBJS) noeuclid
