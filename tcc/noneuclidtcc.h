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

#ifndef _NONEUCLID_H
#define _NONEUCLID_H

#ifdef __cplusplus
extern "C" {
#endif

extern double worldDeltaTime;

extern float gDaytime;

extern float gRenderMixval;
extern float gRenderDensityLimit;
extern float gRenderDensityMux;

extern float gPositionX;
extern float gPositionY;
extern float gPositionZ;
extern float gDirectionX;
extern float gDirectionY;
extern float gDirectionZ;
extern float gTargetNormalX;
extern float gTargetNormalY;
extern float gTargetNormalZ;
extern float gTargetCompressionX;
extern float gTargetCompressionY;
extern float gTargetCompressionZ;
extern float gTargetHitX;
extern float gTargetHitY;
extern float gTargetHitZ;
extern float gTargetActualDistance;
extern float gTargetProjDistance;
extern float gTargetPerceivedDistance;
extern char gDialog[1024];
extern unsigned char gKeyMap[256];
extern unsigned char gFocused;
extern int gMouseLastClickButton;

extern int gOverallUpdateNo;


void ChangeCell( int t, int x, int y, int z, unsigned char r, unsigned char g, unsigned char b, unsigned char a );
void QuickCell( int t, int x, int y, int z, unsigned char r, unsigned char g, unsigned char b, unsigned char a );
void QuickCell1GBAOnly( int x, int y, int z, unsigned char g, unsigned char b, unsigned char a );
void UpdateZone( int x, int y, int z, int sx, int sy, int sz );

#ifndef _TCCEXPORTS_H
double sin( double f );
double cos( double f );
double pow( double base, double exp );
#endif

#ifdef __cplusplus
};
#endif

#endif

