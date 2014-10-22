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



#include <string>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <unordered_map>

using namespace std;
string ZLibUncompress(const string & in);



template<class T> class Vec3 {
public:
    T x, y, z;

    const Vec3<T> operator+(const Vec3& b) const {
        return Vec3{x + b.x, y + b.y, z + b.z};
    }
    
    const Vec3<T> operator-(const Vec3& b) const {
        return Vec3{x - b.x, y - b.y, z - b.z};
    }

    bool operator==(const Vec3& b) const {
        return x == b.x && y == b.y && z == b.z;
    }

    const Vec3<T> operator*(const T s) const {
        return Vec3{x*s, y*s, z * s};
    }
    
    const Vec3<T> operator/(const T s) const {
        return Vec3{x/s, y/s, z/s};
    }

    void operator/=(const T s) {
        x /= s;
        y /= s;
        z /= s;
    }

    void operator+=(const Vec3& b) {
        x += b.x;
        y += b.y;
        z += b.z;
    }

    T len() {
        return sqrt(len2());
    }
    
    T len2() {
        return x * x + y * y + z * z;
    }
    
    const Vec3<T> norm() {
        return this/len();
    }

    const Vec3<T> dot(const Vec3& b) {
        return Vec3{x * b.x, y * b.y, z * b.z};
    }
    
    const Vec3<T> cross(const Vec3& b) {
        return Vec3{y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x};
    }
    
    friend istream &operator>>(istream &inp, Vec3 &v) {
        inp>>v.x>>v.y>>v.z;
        return inp;
    }
};
typedef Vec3<short> Vec3s;
typedef Vec3<int> Vec3i;
typedef Vec3<unsigned int> Vec3iu;
typedef Vec3<float> Vec3f;
typedef Vec3<double> Vec3d;

struct RGBAf {

    RGBAf() : r(0), g(0), b(0), a(0) {
    }

    RGBAf(Vec3f i) : r(i.x), g(i.y), b(i.z), a(0) {
    }

    RGBAf(Vec3f i, float a) : r(i.x), g(i.y), b(i.z), a(a) {
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
    
    Vec3f vec() {
        return {r,g,b};
    }
};

struct RGBA {
    unsigned char r, g, b, a;
};
extern unordered_map<string, int> aliases;
struct BlockType {
    unsigned char t = 0;
    operator char() {return t;}
    friend istream &operator>>(istream &inp, BlockType& p) {
        int x; inp>>x;
        if(inp.fail()) { // string alias?
            inp.clear();
            string alias; inp >> alias;
            if(aliases.count(alias)==0) throw invalid_argument("Invalid alias " + alias);
            p.t=aliases[alias];
        } else {
            p.t = (unsigned char)x;
        }
        return inp;
    }
};



struct CollisionProbe {
    RGBAf Position; //x,y,z,unused
    RGBAf Direction; //x,y,z,maxdist
    RGBAf AuxRotation; //x, y, z, unused (Used for probing for direction changes)


    RGBAf NewDirection; //Passed out unphased by Pass1, but, for Pass1Physics, uses TestDirection
    RGBAf Normal; //normal x,y,z,                         w:  [actual distance]
    RGBAf InAreaWarp; //dx, dy, dz, (Current compression)     w:  [proj/apparent dist]
    RGBAf TargetLocation; //Hit x,y,z, (position in 3D space) w:  [Perceived distance]

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

Vec3f quatrotatevector(const float * quat, Vec3f v);

//From: http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
//From: http://www.cg.info.hiroshima-cu.ac.jp/~miyazaki/knowledge/teche52.html

std::vector<float> quatfrommatrix(Vec3f r1, Vec3f r2, Vec3f r3);
void quatfrommatrix( float * q, float * mat4 );
#endif

