/*
	Copyright (c) 2014 <>< Charles Lohr
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
		* Redistributions of source code must retain the above copyright
		  notice, this list of conditions and the following disclaimer.
		* Redistributions in binary form must reproduce the above copyright
		  notice, this list of conditions and the following disclaimer in the
		  documentation and/or other materials provided with the distribution.
		* Neither the name of the <organization> nor the
		  names of its contributors may be used to endorse or promote products
		  derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _common_h
#define _common_h


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


#ifdef __cplusplus
#include <string>
using namespace std;
string ZLibUncompress( const string & in );


struct RGBAf
{
	RGBAf() : r(0), g(0), b(0), a(0) { }
	RGBAf( float ri, float gi, float bi, float ai ): r(ri), g(gi), b(bi), a(ai) { }
	float r, g, b, a;
	void FromStringArray( string * arr );
};

struct RGBA
{
	RGBA() : r(0), g(0), b(0), a(0) { }
	RGBA( unsigned char ri, unsigned char gi, unsigned char bi, unsigned char ai ): r(ri), g(gi), b(bi), a(ai) { }
	unsigned char r, g, b, a;
};
#else
struct RGBAf
{
	float r, g, b, a;
};

struct RGBA
{
	unsigned char r, g, b, a;
};
#endif


struct CollisionProbe
{
	struct RGBAf Position;   //x,y,z,unused
	struct RGBAf Direction;  //x,y,z,maxdist
	struct RGBAf AuxRotation; //x, y, z, unused (Used for probing for direction changes)


	struct RGBAf NewDirection;  //Passed out unphased by Pass1, but, for Pass1Physics, uses TestDirection
	struct RGBAf Normal;     //normal x,y,z,                         w:  [actual distance]
	struct RGBAf InAreaWarp; //dx, dy, dz, (Current compression)     w:  [proj/apparent dist]
	struct RGBAf TargetLocation; //Hit x,y,z, (position in 3D space) w:  [Perceived distance]

	int id;
};


#endif

