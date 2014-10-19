all : noeuclid

#-DFAKEMAP means don't run any of the TCC stuff.
CFLAGS:=-std=c++11 -g -Wall -Wno-sign-compare -O2 -DGL_GLEXT_PROTOTYPES  -Itcc  #-DFAKEMAP
CXXFLAGS:=$(CFLAGS)
LDFLAGS:= -lGL -lGLU -lglut -lz -g -lpthread -ltcc  -ldl

noeuclid : noeuclid.o OGLParts.o GameMap.o Common.o RTHelper.o Map.o
	$(CXX) -o $@ $^ $(LDFLAGS)


WINCFLAGS:= -std=c++11 -m32 -I. -g -O2 -DGLEW_STATIC  -DGLEXT -DGLUT_DISABLE_ATEXIT_HACK
WINCPP:=i686-w64-mingw32-g++  #i586-mingw32msvc-g++
WINC:=i686-w64-mingw32-gcc  #i586-mingw32msvc-gcc
WINLDFLAGS:= -static -lglu32 -lkernel32 -lglew32 -lglut32 -lopengl32 


%.obj : %.cpp
	$(WINCPP) -c -o $@ $^ $(WINCFLAGS)

%.obj : %.c
	$(WINC) -c -o $@ $^ $(WINCFLAGS) 

windows/noeuclid.exe : noeuclid.obj OGLParts.obj RTHelper.obj Map.obj Common.obj GameMap.obj
	$(WINCPP)  -o $@ $^ $(WINCFLAGS) $(WINLDFLAGS)


clean :
	rm -fr *.obj *.o tcc/*.o *~ noeuclid windows/noeuclid.exe
