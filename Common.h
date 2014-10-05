#ifndef _common_h
#define _common_h

#include <string>

using namespace std;

#ifdef FAKEMAP
#define GLH_SIZEX  64
#define GLH_SIZEY  64
#define GLH_SIZEZ  64
#else
#define GLH_SIZEX  128
#define GLH_SIZEY  128
#define GLH_SIZEZ  128
#endif

#define ADDSIZEX 256
#define ADDSIZEY 256

#define MAX_SEEKER 1

#define SIGN(x) ((x>0)?1:((x<0)?-1:0))

struct RGBAf
{
	RGBAf() : r(0), b(0), g(0), a(0) { }
	RGBAf( float ri, float gi, float bi, float ai ): r(ri), g(gi), b(bi), a(ai) { }
	float r, g, b, a;
	void FromStringArray( string * arr );
};

struct RGBA
{
	RGBA() : r(0), b(0), g(0), a(0) { }
	RGBA( unsigned char ri, unsigned char gi, unsigned char bi, unsigned char ai ): r(ri), g(gi), b(bi), a(ai) { }
	unsigned char r, g, b, a;
};

string ZLibUncompress( const string & in );


#endif

