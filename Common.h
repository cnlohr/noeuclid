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
using byte = unsigned char;


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
    
    bool operator!=(const Vec3& b) const {
        return !(x == b.x && y == b.y && z == b.z);
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
        return (*this)/len();
    }

    const T dot(const Vec3& b) {
        return x * b.x + y * b.y + z * b.z;
    }
    
    const Vec3<T> cross(const Vec3& b) {
        return Vec3{y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x};
    }
    
    friend istream &operator>>(istream &inp, Vec3 &v) {
        inp>>v.x>>v.y>>v.z;
        return inp;
    }
    friend ostream &operator<<(ostream &oup, Vec3 &v) {
        oup<<"("<<v.x<<" "<<v.y<<" "<<v.z<<")";
        return oup;
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
    
    friend istream &operator>>(istream &inp, RGBAf &c) {
        inp>>c.r>>c.g>>c.b>>c.a;
        return inp;
    }
};

struct Quaternion {
    Quaternion(float f1=0,float f2=0, float f3=0, float f4=0): 
        f{f1,f2,f3,f4} {};
    Quaternion(RGBAf col) : f{col.r,col.g,col.b,col.a} {};
    float f[4];
    Quaternion cross3d(Quaternion &b) {
        return {
            f[1] * b[2] - f[2] * b[1],
            f[2] * b[0] - f[0] * b[2],
            f[0] * b[1] - f[1] * b[0], 0};
    }
    
    Quaternion normalize() const {
        return this->scale(this->invsqmagnitude());
    }
    Quaternion scale(float scale) const {
        return {f[0]*scale,f[1]*scale,f[2]*scale,f[3]*scale};
    }
    Quaternion rotateabout(Quaternion b) const {
        Quaternion q1 = this->normalize();
        Quaternion q2 = b.normalize();
        return {
            (q1[0] * q2[0])-(q1[1] * q2[1])-(q1[2] * q2[2])-(q1[3] * q2[3]),
            (q1[0] * q2[1])+(q1[1] * q2[0])+(q1[2] * q2[3])-(q1[3] * q2[2]),
            (q1[0] * q2[2])-(q1[1] * q2[3])+(q1[2] * q2[0])+(q1[3] * q2[1]),
            (q1[0] * q2[3])+(q1[1] * q2[2])-(q1[2] * q2[1])+(q1[3] * q2[0])
        };
    }
    Quaternion conjugate() const {
        return {f[0],-f[1],-f[2],-f[3]};
    }
    float invsqmagnitude() const {
        const Quaternion& q = *this;
        return 1. / ((q[0] * q[0])+(q[1] * q[1])+(q[2] * q[2])+(q[3] * q[3]));
    }
    const float& operator[](int index) const {
        return f[index];
    }
    float& operator[](int index) {
        return f[index];
    }
    
    Quaternion reciprocal() const {
        return this->conjugate().scale(this->invsqmagnitude());
    }
    
    Vec3f rotateVector(Vec3f v) const {
        //XXX IS THIS RIGHT? IT LOOKS WRONG!!!
        Quaternion vquat = {(v.len2() < 0.001) ? 1.f : 0.f,v.x,v.y,v.z};
        vquat = this->rotateabout(vquat).rotateabout(this->reciprocal());
        return {vquat[1],vquat[2],vquat[3]};
    }
    
    void toMatrix(float *matrix44) {
        Quaternion q = this->normalize();
        //Reduced calulation for speed
        float xx = 2 * q[1] * q[1];
        float xy = 2 * q[1] * q[2];
        float xz = 2 * q[1] * q[3];
        float xw = 2 * q[1] * q[0];

        float yy = 2 * q[2] * q[2];
        float yz = 2 * q[2] * q[3];
        float yw = 2 * q[2] * q[0];

        float zz = 2 * q[3] * q[3];
        float zw = 2 * q[3] * q[0];

        //opengl major
        matrix44[0] = 1 - yy - zz;
        matrix44[1] = xy - zw;
        matrix44[2] = xz + yw;
        matrix44[3] = 0;

        matrix44[4] = xy + zw;
        matrix44[5] = 1 - xx - zz;
        matrix44[6] = yz - xw;
        matrix44[7] = 0;

        matrix44[8] = xz - yw;
        matrix44[9] = yz + xw;
        matrix44[10] = 1 - xx - yy;
        matrix44[11] = 0;

        matrix44[12] = 0;
        matrix44[13] = 0;
        matrix44[14] = 0;
        matrix44[15] = 1;
    }
    static Quaternion fromEuler(Vec3f euler) {
        euler/=2; //x=roll,y=pitch,z=yaw

        float cx = cos(euler.x);
        float sx = sin(euler.x);
        float cy = cos(euler.y);
        float sy = sin(euler.y);
        float cz = cos(euler.z);
        float sz = sin(euler.z);

        //Correct according to
        //http://en.wikipedia.org/wiki/Conversion_between_Quaternions_and_Euler_angles
        Quaternion q;
        q[0] = cx * cy * cz + sx * sy*sz; //q1
        q[1] = sx * cy * cz - cx * sy*sz; //q2
        q[2] = cx * sy * cz + sx * cy*sz; //q3
        q[3] = cx * cy * sz - sx * sy*cz; //q4

        return q.normalize();
    }
    //From: http://www.cg.info.hiroshima-cu.ac.jp/~miyazaki/knowledge/teche52.html
    static Quaternion fromMatrix(float * mat4) {
        Quaternion q;
        #define M4F(x,y) mat4[y*4+x]
        #define SIGN(x) (x<0?-1:1)
	q[0] = ( M4F(0,0) + M4F(1,1) + M4F(2,2) + 1.0f) / 4.0f;
	q[1] = ( M4F(0,0) - M4F(1,1) - M4F(2,2) + 1.0f) / 4.0f;
	q[2] = (-M4F(0,0) + M4F(1,1) - M4F(2,2) + 1.0f) / 4.0f;
	q[3] = (-M4F(0,0) - M4F(1,1) + M4F(2,2) + 1.0f) / 4.0f;
	if(q[0] < 0.0f) q[0] = 0.0f;
	if(q[1] < 0.0f) q[1] = 0.0f;
	if(q[2] < 0.0f) q[2] = 0.0f;
	if(q[3] < 0.0f) q[3] = 0.0f;
	q[0] = sqrt(q[0]);
	q[1] = sqrt(q[1]);
	q[2] = sqrt(q[2]);
	q[3] = sqrt(q[3]);
	if(q[0] >= q[1] && q[0] >= q[2] && q[0] >= q[3]) {
		q[0] *= +1.0f;
		q[1] *= SIGN(M4F(2,1) - M4F(1,2));
		q[2] *= SIGN(M4F(0,2) - M4F(2,0));
		q[3] *= SIGN(M4F(1,0) - M4F(0,1));
	} else if(q[1] >= q[0] && q[1] >= q[2] && q[1] >= q[3]) {
		q[0] *= SIGN(M4F(2,1) - M4F(1,2));
		q[1] *= +1.0f;
		q[2] *= SIGN(M4F(1,0) + M4F(0,1));
		q[3] *= SIGN(M4F(0,2) + M4F(2,0));
	} else if(q[2] >= q[0] && q[2] >= q[1] && q[2] >= q[3]) {
		q[0] *= SIGN(M4F(0,2) - M4F(2,0));
		q[1] *= SIGN(M4F(1,0) + M4F(0,1));
		q[2] *= +1.0f;
		q[3] *= SIGN(M4F(2,1) + M4F(1,2));
	} else if(q[3] >= q[0] && q[3] >= q[1] && q[3] >= q[2]) {
		q[0] *= SIGN(M4F(1,0) - M4F(0,1));
		q[1] *= SIGN(M4F(2,0) + M4F(0,2));
		q[2] *= SIGN(M4F(2,1) + M4F(1,2));
		q[3] *= +1.0f;
	} else {
		//printf("coding error\n");
		q[0] = 1;
		q[1] = 0;
		q[2] = 0;
		q[3] = 0;
	}
	q = q.normalize();
        return q;
    }
    
    static Quaternion fromAxisAngle(Vec3f axis, float radians) {
         Vec3f v = axis.norm();

        float sn = sin(radians / 2.0f);
        Quaternion q;
        q[0] = cos(radians / 2.0f);
        q[1] = sn * v.x;
        q[2] = sn * v.y;
        q[3] = sn * v.z;

        return q.normalize();
    }
};

struct Vec3range;
struct Vec3it {
    const Vec3range& c;
    Vec3<int> i;
    bool operator!=(const Vec3it& o) {
        return i.x!=o.i.x&&i.y!=o.i.y&&i.z!=o.i.z;
    }
    Vec3i operator*() const {
        return i;
    }
    const Vec3it& operator++();
};
// usage: for(Vec3 v: Vec3range{{20,40,2}, {25,45,4}}) doStuff(v);
struct Vec3range {
    Vec3<int> s,e;
    Vec3it begin() const { return {*this,s};}
    Vec3it end() const { return {*this,e};}
};

struct RGBA {
    byte r, g, b, a;
};
extern unordered_map<string, int> aliases;
struct BlockType {
    byte t = 0;
    operator char() {return t;}
    friend istream &operator>>(istream &inp, BlockType& p) {
        int x; inp>>x;
        if(inp.fail()) { // string alias?
            inp.clear();
            string alias; inp >> alias;
            if(aliases.count(alias)==0) throw invalid_argument("Invalid alias " + alias);
            p.t=aliases[alias];
        } else {
            p.t = (byte)x;
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
#endif