all : noeuclid

#-DFAKEMAP means don't run any of the TCC stuff.
CFLAGS:=-g -Wall -Wno-sign-compare -O2 -DGL_GLEXT_PROTOTYPES  -Itcc  #-DFAKEMAP
CXXFLAGS:=$(CFLAGS)
LDFLAGS:= -lGL -lGLU -lglut -lz -g -lpthread -ltcc  -ldl

noeuclid : noeuclid.o OGLParts.o GLUTCore.o RTHelper.o Map.o tccexports.o tccengine.o os_generic.o linmath.o
	g++ -o $@ $^ $(LDFLAGS)


WINCFLAGS:= -m32 -I. -g -O2 -Iwindows -Iwindows/glew -Iwindows/freeglut -I/usr/i586-mingw32msvc/include/GL  -DGLEW_STATIC  -DGLEXT
WINCPP:=i686-w64-mingw32-g++  #i586-mingw32msvc-g++
WINC:=i686-w64-mingw32-gcc  #i586-mingw32msvc-gcc
WINLDFLAGS:= \
	-g \
	windows/glew/glew-i586.a \
	-lkernel32 -lm \
	windows/libtcc.dll \
	/usr/i586-mingw32msvc/lib/libopengl32.a \
	/usr/i586-mingw32msvc/lib/libglu32.a \
	./windows/freeglut-i686.dll \
	./windows/libtcc1.a \



%.obj : %.cpp
	$(WINCPP) -c -o $@ $^ $(WINCFLAGS)

%.obj : %.c
	$(WINC) -c -o $@ $^ $(WINCFLAGS) 

windows/noeuclid.exe : noeuclid.obj OGLParts.obj GLUTCore.obj RTHelper.obj Map.obj tccexports.obj tccengine.obj os_generic.obj linmath.obj
	$(WINCPP)  -o $@ $^ $(WINCFLAGS) $(WINLDFLAGS)


clean :
	rm -fr *.obj *.o tcc/*.o *~ noeuclid windows/noeuclid.exe
