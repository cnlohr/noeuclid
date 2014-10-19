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


#include <string>
#include <cmath>
#include <cstdlib>

using std::string;
string ZLibUncompress(const string & in);

struct RGBAf {

    RGBAf() : r(0), g(0), b(0), a(0) {
    }

    RGBAf(float ri, float gi, float bi, float ai) : r(ri), g(gi), b(bi), a(ai) {
    }
    float r, g, b, a;

    void FromStringArray(string * arr) {
        r = atof(arr[0].c_str());
        g = atof(arr[1].c_str());
        b = atof(arr[2].c_str());
        a = atof(arr[3].c_str());
    };
};

struct RGBA {
    unsigned char r, g, b, a;
};

struct CollisionProbe {
    struct RGBAf Position; //x,y,z,unused
    struct RGBAf Direction; //x,y,z,maxdist
    struct RGBAf AuxRotation; //x, y, z, unused (Used for probing for direction changes)


    struct RGBAf NewDirection; //Passed out unphased by Pass1, but, for Pass1Physics, uses TestDirection
    struct RGBAf Normal; //normal x,y,z,                         w:  [actual distance]
    struct RGBAf InAreaWarp; //dx, dy, dz, (Current compression)     w:  [proj/apparent dist]
    struct RGBAf TargetLocation; //Hit x,y,z, (position in 3D space) w:  [Perceived distance]

    int id;
};

void cross3d(float * out, const float * a, const float * b);

void normalize3d(float * out, const float * in);

float dot3d(const float * a, const float * b);
void copy3d(float * out, const float * in);



/////////////////////////////////////QUATERNIONS//////////////////////////////////////////
//Originally from Mercury (Copyright (C) 2009 by Joshua Allen, Charles Lohr, Adam Lowman)
//Under the mit/X11 license.

void quatcopy(float * qout, const float * qin);

float quatinvsqmagnitude(const float * q);
void quatscale(float * qout, const float * qin, float s);
void quatnormalize(float * qout, const float * qin);

void quatfromeuler(float * q, const float * euler);

void quatfromaxisangle(float * q, const float * axis, float radians);

void quattomatrix(float * matrix44, const float * qin);

void quatgetconjugate(float * qout, const float * qin);

void quatrotateabout(float * qout, const float * a, const float * b);

void quatscale(float * qout, const float * qin, float s);

void quatgetreciprocal(float * qout, const float * qin);

void quatrotatevector(float * vec3out, const float * quat, const float * vec3in);


//From: http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
//From: http://www.cg.info.hiroshima-cu.ac.jp/~miyazaki/knowledge/teche52.html

void quatfrommatrix(float * q, float * mat4);


#endif

