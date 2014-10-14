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

#include "linmath.h"
#include <math.h>

void cross3d( float * out, const float * a, const float * b )
{
	out[0] = a[1]*b[2] - a[2]*b[1];
	out[1] = a[2]*b[0] - a[0]*b[2];
	out[2] = a[0]*b[1] - a[1]*b[0];
}

void sub3d( float * out, const float * a, const float * b )
{
	out[0] = a[0] - b[0];
	out[1] = a[1] - b[1];
	out[2] = a[2] - b[2];
}

void add3d( float * out, const float * a, const float * b )
{
	out[0] = a[0] + b[0];
	out[1] = a[1] + b[1];
	out[2] = a[2] + b[2];
}

void scale3d( float * out, const float * a, float scalar )
{
	out[0] = a[0] * scalar;
	out[1] = a[1] * scalar;
	out[2] = a[2] * scalar;
}

void normalize3d( float * out, const float * in )
{
	float r = 1./sqrtf( in[0] * in[0] + in[1] * in[1] + in[2] * in[2] );
	out[0] = in[0] * r;
	out[1] = in[1] * r;
	out[2] = in[2] * r;
}

float dot3d( const float * a, const float * b )
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

int compare3d( const float * a, const float * b, float epsilon )
{
	if( !a || !b ) return 0;
	if( a[2] - b[2] > epsilon ) return 1;
	if( b[2] - a[2] > epsilon ) return -1;
	if( a[1] - b[1] > epsilon ) return 1;
	if( b[1] - a[1] > epsilon ) return -1;
	if( a[0] - b[0] > epsilon ) return 1;
	if( b[0] - a[0] > epsilon ) return -1;
	return 0;
}

void copy3d( float * out, const float * in )
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}



/////////////////////////////////////QUATERNIONS//////////////////////////////////////////
//Originally from Mercury (Copyright (C) 2009 by Joshua Allen, Charles Lohr, Adam Lowman)
//Under the mit/X11 license.




void quatsetnone( float * q )
{
	q[0] = 0; q[1] = 0; q[2] = 0; q[3] = 1;
}

void quatcopy( float * qout, const float * qin )
{
	qout[0] = qin[0];
	qout[1] = qin[1];
	qout[2] = qin[2];
	qout[3] = qin[3];
}

void quatfromeuler( float * q, const float * euler )
{
	float X = euler[0]/2.0f; //roll
	float Y = euler[1]/2.0f; //pitch
	float Z = euler[2]/2.0f; //yaw

	float cx = cosf(X);
	float sx = sinf(X);
	float cy = cosf(Y);
	float sy = sinf(Y);
	float cz = cosf(Z);
	float sz = sinf(Z);

	//Correct according to
	//http://en.wikipedia.org/wiki/Conversion_between_Quaternions_and_Euler_angles
	q[0] = cx*cy*cz+sx*sy*sz;//q1
	q[1] = sx*cy*cz-cx*sy*sz;//q2
	q[2] = cx*sy*cz+sx*cy*sz;//q3
	q[3] = cx*cy*sz-sx*sy*cz;//q4

	quatnormalize( q, q );
}

void quattoeuler( float * euler, const float * q )
{
	//According to http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles (Oct 26, 2009)
	euler[0] = atan2( 2 * (q[0]*q[1] + q[2]*q[3]), 1 - 2 * (q[1]*q[1] + q[2]*q[2] ) );
	euler[1] = asin( 2 * (q[0] *q[2] - q[3]*q[1] ) );
	euler[2] = atan2( 2 * (q[0]*q[3] + q[1]*q[2]), 1 - 2 * (q[2]*q[2] + q[3]*q[3] ) );
}

void quatfromaxisangle( float * q, const float * axis, float radians )
{
	float v[3];
	normalize3d( v, axis );
	
	float sn = sin(radians/2.0f);
	q[0] = cos(radians/2.0f);
	q[1] = sn * v[0];
	q[2] = sn * v[1];
	q[3] = sn * v[2];

	quatnormalize( q, q );
}

float quatmagnitude( const float * q )
{
	return sqrt((q[0]*q[0])+(q[1]*q[1])+(q[2]*q[2])+(q[3]*q[3]));
}

float quatinvsqmagnitude( const float * q )
{
	return 1./((q[0]*q[0])+(q[1]*q[1])+(q[2]*q[2])+(q[3]*q[3]));
}


void quatnormalize( float * qout, const float * qin )
{
	float imag = quatinvsqmagnitude( qin );
	quatscale( qout, qin, imag );
}

void quattomatrix( float * matrix44, const float * qin )
{
	float q[4];
	quatnormalize( q, qin );
	
	//Reduced calulation for speed
	float xx = 2*q[1]*q[1];
	float xy = 2*q[1]*q[2];
	float xz = 2*q[1]*q[3];
	float xw = 2*q[1]*q[0];
	
	float yy = 2*q[2]*q[2];
	float yz = 2*q[2]*q[3];
	float yw = 2*q[2]*q[0];
	
	float zz = 2*q[3]*q[3];
	float zw = 2*q[3]*q[0];

	//opengl major
	matrix44[0] = 1-yy-zz;
	matrix44[1] = xy-zw;
	matrix44[2] = xz+yw;
	matrix44[3] = 0;

	matrix44[4] = xy+zw;
	matrix44[5] = 1-xx-zz;
	matrix44[6] = yz-xw;
	matrix44[7] = 0;

	matrix44[8] = xz-yw;
	matrix44[9] = yz+xw;
	matrix44[10] = 1-xx-yy;
	matrix44[11] = 0;

	matrix44[12] = 0;
	matrix44[13] = 0;
	matrix44[14] = 0;
	matrix44[15] = 1;
}

void quatgetconjugate( float * qout, const float * qin )
{
	qout[0] = qin[0];
	qout[1] = -qin[1];
	qout[2] = -qin[2];
	qout[3] = -qin[3];
}

void quatgetreciprocal( float * qout, const float * qin )
{
	float m = quatinvsqmagnitude(qin);
	quatgetconjugate( qout, qin );
	quatscale( qout, qout, m );
}

void quatsub( float * qout, const float * a, const float * b )
{
	qout[0] = a[0] - b[0];
	qout[1] = a[1] - b[1];
	qout[2] = a[2] - b[2];
	qout[3] = a[3] - b[3];
}

void quatadd( float * qout, const float * a, const float * b )
{
	qout[0] = a[0] + b[0];
	qout[1] = a[1] + b[1];
	qout[2] = a[2] + b[2];
	qout[3] = a[3] + b[3];
}

void quatrotateabout( float * qout, const float * a, const float * b )
{
	float q1[4];
	float q2[4];

	quatnormalize( q1, a );
	quatnormalize( q2, b );

	qout[0] = (q1[0]*q2[0])-(q1[1]*q2[1])-(q1[2]*q2[2])-(q1[3]*q2[3]);
	qout[1] = (q1[0]*q2[1])+(q1[1]*q2[0])+(q1[2]*q2[3])-(q1[3]*q2[2]);
	qout[2] = (q1[0]*q2[2])-(q1[1]*q2[3])+(q1[2]*q2[0])+(q1[3]*q2[1]);
	qout[3] = (q1[0]*q2[3])+(q1[1]*q2[2])-(q1[2]*q2[1])+(q1[3]*q2[0]);
}

void quatscale( float * qout, const float * qin, float s )
{
	qout[0] = qin[0] * s;
	qout[1] = qin[1] * s;
	qout[2] = qin[2] * s;
	qout[3] = qin[3] * s;
}


float quatinnerproduct( const float * qa, const float * qb )
{
	return (qa[0]*qb[0])+(qa[1]*qb[1])+(qa[2]*qb[2])+(qa[3]*qb[3]);
}

void quatouterproduct( float * outvec3, float * qa, float * qb )
{
	outvec3[0] = (qa[0]*qb[1])-(qa[1]*qb[0])-(qa[2]*qb[3])+(qa[3]*qb[2]);
	outvec3[1] = (qa[0]*qb[2])+(qa[1]*qb[3])-(qa[2]*qb[0])-(qa[3]*qb[1]);
	outvec3[2] = (qa[0]*qb[3])-(qa[1]*qb[2])+(qa[2]*qb[1])-(qa[3]*qb[0]);
}

void quatevenproduct( float * q, float * qa, float * qb )
{
	q[0] = (qa[0]*qb[0])-(qa[1]*qb[1])-(qa[2]*qb[2])-(qa[3]*qb[3]);
	q[1] = (qa[0]*qb[1])+(qa[1]*qb[0]);
	q[2] = (qa[0]*qb[2])+(qa[2]*qb[0]);
	q[3] = (qa[0]*qb[3])+(qa[3]*qb[0]);
}

void quatoddproduct( float * outvec3, float * qa, float * qb )
{
	outvec3[0] = (qa[2]*qb[3])-(qa[3]*qb[2]);
	outvec3[1] = (qa[3]*qb[1])-(qa[1]*qb[3]);
	outvec3[2] = (qa[1]*qb[2])-(qa[2]*qb[1]);
}

void quatslerp( float * q, const float * qa, const float * qb, float t )
{
	float an[4];
	float bn[4];
	quatnormalize( an, qa );
	quatnormalize( bn, qb );
	float cosTheta = quatinnerproduct(an,bn);
	float sinTheta;

	//Careful: If cosTheta is exactly one, or even if it's infinitesimally over, it'll
	// cause SQRT to produce not a number, and screw everything up.
	if ( 1 - (cosTheta*cosTheta) <= 0 )
		sinTheta = 0;
	else
		sinTheta = sqrt(1 - (cosTheta*cosTheta));

	float Theta = acos(cosTheta); //Theta is half the angle between the 2 MQuaternions

	if(fabs(Theta) < DEFAULT_EPSILON )
		quatcopy( q, qa );
	else if(fabs(sinTheta) < DEFAULT_EPSILON )
	{
		quatadd( q, qa, qb );
		quatscale( q, q, 0.5 );
	}
	else
	{
		float aside[4];
		float bside[4];
		quatscale( bside, qb, sin( t * Theta ) );
		quatscale( aside, qa, sin((1-t)*Theta) );
		quatadd( q, aside, bside );
		quatscale( q, q, 1./sinTheta );
	}
}

void quatrotatevector( float * vec3out, const float * quat, const float * vec3in )
{
	float tquat[4];
	float vquat[4];
	float qrecp[4];

	vquat[0] = ((vec3in[0]*vec3in[0] + vec3in[1]*vec3in[1] + vec3in[2]*vec3in[2])<0.001)?1:0;  //XXX IS THIS RIGHT? IT LOOKS WRONG!!!
	vquat[1] = vec3in[0];
	vquat[2] = vec3in[1];
	vquat[3] = vec3in[2];

	quatrotateabout( tquat, quat, vquat );
	quatgetreciprocal( qrecp, quat );
	quatrotateabout( vquat, tquat, qrecp );

	vec3out[0] = vquat[1];
	vec3out[1] = vquat[2];
	vec3out[2] = vquat[3];
}


//From: http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
//From: http://www.cg.info.hiroshima-cu.ac.jp/~miyazaki/knowledge/teche52.html
void quatfrommatrix( float * q, float * mat4 )
{
/*
	float tr = M4F(0,0) + M4F(1,1) + M4F(2,2);
	printf( "TR:%f\n", tr );
	if (tr > 0)
	{ 
		float S = 0.5/sqrt(tr+1.0); // S=4*q[0] 
		printf( "A %f\n", S );
		q[0] = 0.25 / S;
		q[1] = (M4F(2,1) - M4F(1,2)) / S;
		q[2] = (M4F(0,2) - M4F(2,0)) / S; 
		q[3] = (M4F(1,0) - M4F(0,1)) / S; 
	}
	else if ((M4F(0,0) > M4F(1,1))&(M4F(0,0) > M4F(2,2)))
	{ 
		float S = sqrt(1.0 + M4F(0,0) - M4F(1,1) - M4F(2,2)) * 2; // S=4*q[1] 
		printf( "B %f\n", S );
		q[0] = (M4F(2,1) - M4F(1,2)) / S;
		q[1] = 0.25 * S;
		q[2] = (M4F(0,1) + M4F(1,0)) / S; 
		q[3] = (M4F(0,2) + M4F(2,0)) / S; 
	}
	else if (M4F(1,1) > M4F(2,2))
	{ 
		float S = sqrt(1.0 + M4F(1,1) - M4F(0,0) - M4F(2,2)) * 2; // S=4*q[2]
		printf( "C %f\n", S );
		q[0] = (M4F(0,2) - M4F(2,0)) / S;
		q[1] = (M4F(0,1) + M4F(1,0)) / S; 
		q[2] = 0.25 * S;
		q[3] = (M4F(1,2) + M4F(2,1)) / S; 
	}
	else
	{ 
		float S = sqrt(1.0 + M4F(2,2) - M4F(0,0) - M4F(1,1)) * 2; // S=4*q[3]
		printf( "D %f\n", S );
		q[0] = (M4F(1,0) - M4F(0,1)) / S;
		q[1] = (M4F(0,2) + M4F(2,0)) / S;
		q[2] = (M4F(1,2) + M4F(2,1)) / S;
		q[3] = 0.25 * S;
	}
*/
#define M4F(x,y) mat4[y*4+x]
#define SIGN(x) (((x)<0)?-1:1)
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
	quatnormalize( q, q );
}

