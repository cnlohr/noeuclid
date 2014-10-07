all : noneuclid

#-DFAKEMAP means don't run any of the TCC stuff.
CFLAGS:=-I. -g -O2 -DGL_GLEXT_PROTOTYPES  -Itcc  #-DFAKEMAP
CXXFLAGS:=$(CFLAGS)
LDFLAGS:= -lGL -lGLU -lglut -lz -g -lpthread tcc/libtcc-$(shell uname -m).a -ldl

noneuclid : noneuclid.o OGLParts.o GLUTCore.o RTHelper.o Map.o tccexports.o tccengine.o os_generic.o linmath.o
	g++ -o $@ $^ $(LDFLAGS)


WINCFLAGS:= -m32 -I. -g -O2  -I../WindowsBuild/freeglut -I/usr/i586-mingw32msvc/include/GL -DGLEXT 
WINCPP:=i686-w64-mingw32-g++  #i586-mingw32msvc-g++
WINC:=i686-w64-mingw32-gcc  #i586-mingw32msvc-gcc

%.obj : %.cpp
	$(WINCPP) -c -o $@ $^ $(WINCFLAGS)

%.obj : %.c
	$(WINC) -c -o $@ $^ $(WINCFLAGS) 

noneuclid.exe : noneuclid.obj OGLParts.obj GLUTCore.obj RTHelper.obj Map.obj tccexports.obj tccengine.obj os_generic.obj ../WindowsBuild/GLEXTHandler.obj
	$(WINCPP)  -o $@ $^ $(WINCFLAGS) 


clean :
	rm -fr *.o tcc/*.o *~ noneuclid
