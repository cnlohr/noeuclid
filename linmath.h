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

#ifndef _LINMATH_H
#define _LINMATH_H

#ifdef __cplusplus
extern "C" {
#endif

//Yes, I know it's kind of arbitrary.
#define DEFAULT_EPSILON 0.001


//NOTE: Inputs may never be output with cross product.
void cross3d( float * out, const float * a, const float * b );

void sub3d( float * out, const float * a, const float * b );

void add3d( float * out, const float * a, const float * b );

void scale3d( float * out, const float * a, float scalar );

void normalize3d( float * out, const float * in );

float dot3d( const float * a, const float * b );

//Returns 0 if equal.  If either argument is null, 0 will ALWAYS be returned.
int compare3d( const float * a, const float * b, float epsilon );

void copy3d( float * out, const float * in );




//Quaternion things...
// [wxyz]
// w = the cos term
// xyz = the sin terms

void quatsetnone( float * q );
void quatcopy( float * qout, const float * qin );
void quatfromeuler( float * q, const float * euler );
void quattoeuler( float * euler, const float * q );
void quatfromaxisangle( float * q, const float * axis, float radians );
float quatmagnitude( const float * q );
float quatinvsqmagnitude( const float * q );
void quatnormalize( float * qout, const float * qin );  //Safe for in to be same as out.
void quattomatrix( float * matrix44, const float * q );
void quatgetconjugate( float * qout, const float * qin );
void quatgetreciprocal( float * qout, const float * qin );
void quatsub( float * qout, const float * a, const float * b );
void quatadd( float * qout, const float * a, const float * b );
void quatrotateabout( float * qout, const float * a, const float * b );  //same as quat multiply, not piecewise multiply.
void quatscale( float * qout, const float * qin, float s );
float quatinnerproduct( const float * qa, const float * qb );
void quatouterproduct( float * outvec3, float * qa, float * qb );
void quatevenproduct( float * q, float * qa, float * qb );
void quatoddproduct( float * outvec3, float * qa, float * qb );
void quatslerp( float * q, const float * qa, const float * qb, float t );
void quatrotatevector( float * vec3out, const float * quat, const float * vec3in );

void quatfrommatrix( float * q, float * mat4 );

#ifdef __cplusplus
};
#endif


#endif


