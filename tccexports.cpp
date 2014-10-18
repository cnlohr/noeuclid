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

#include "tccexports.h"
#include "tccengine.h"

#include <stdio.h>
#include <stdlib.h>
#include "noeuclidtcc.h"
#include "Map.h"
#include "RTHelper.h"


#define IMPHELPFN( x ) TCCESetSym(tce, #x, (void*)x );
#define IMPHELPVA( x ) TCCESetSym(tce, #x, (void*)&x );

extern double worldDeltaTime;
int AddSizeStride;

int pickables_in_inventory;



void PopulateTCCE(TCCEngine * tce )
{
	IMPHELPFN( printf );
	IMPHELPFN( sin );
	IMPHELPFN( cos );
	IMPHELPFN( fmodf );
	IMPHELPFN( sqrtf );
	IMPHELPFN( pow );
	IMPHELPFN( rand );

	IMPHELPFN( ChangeCell );
	IMPHELPFN( QuickCell );
	IMPHELPFN( QuickCell1GBAOnly );
	IMPHELPFN( UpdateZone );
	IMPHELPFN( AllocAddInfo );
	IMPHELPFN( AlterAddInfo );
	IMPHELPFN( MarkAddDataForReload );


	IMPHELPVA( worldDeltaTime );
	IMPHELPVA( AddSizeStride );
	AddSizeStride = ADDSIZEX;
//	IMPHELPVA( Pitch );
//	IMPHELPVA( Yaw );

	IMPHELPVA( gPositionX );
	IMPHELPVA( gPositionY );
	IMPHELPVA( gPositionZ );
	IMPHELPVA( gDirectionX );
	IMPHELPVA( gDirectionY );
	IMPHELPVA( gDirectionZ );
	IMPHELPVA( gTargetNormalX );
	IMPHELPVA( gTargetNormalY );
	IMPHELPVA( gTargetNormalZ );
	IMPHELPVA( gTargetCompressionX );
	IMPHELPVA( gTargetCompressionY );
	IMPHELPVA( gTargetCompressionZ );
	IMPHELPVA( gTargetHitX );
	IMPHELPVA( gTargetHitY );
	IMPHELPVA( gTargetHitZ );
	IMPHELPVA( gTargetActualDistance );
	IMPHELPVA( gTargetProjDistance );
	IMPHELPVA( gTargetPerceivedDistance );
	IMPHELPVA( gDialog );
	IMPHELPVA( gFocused );
	IMPHELPVA( gMouseLastClickButton );
	IMPHELPVA( gTimeSinceOnGround );

	IMPHELPVA( gRenderMixval );
	IMPHELPVA( gRenderDensityLimit );
	IMPHELPVA( gRenderDensityMux );
	IMPHELPVA( gOverallUpdateNo );

	IMPHELPVA( gDaytime );
	IMPHELPVA( pickables_in_inventory );
	IMPHELPVA( gKeyMap );


	IMPHELPVA( GameTimer );
	IMPHELPVA( GameAttempt );

}

extern RTHelper * gh;

void ChangeCell( int t, int x, int y, int z, unsigned char r, unsigned char g, unsigned char b, unsigned char a )
{
	gh->TMap->TexCell(t, x, y, z ) = RGBA( r,g,b,a );
	gh->TMap->TackChange( x, y, z );
}

void QuickCell( int t, int x, int y, int z, unsigned char r, unsigned char g, unsigned char b, unsigned char a )
{
	gh->TMap->TexCell(t, x, y, z ) = RGBA( r,g,b,a );
}

void QuickCell1GBAOnly( int x, int y, int z, unsigned char g, unsigned char b, unsigned char a )
{
	gh->TMap->TexCell(1, x, y, z ).g = g;
	gh->TMap->TexCell(1, x, y, z ).b = b;
	gh->TMap->TexCell(1, x, y, z ).a = a;
}


void UpdateZone( int x, int y, int z, int sx, int sy, int sz )
{
	gh->TMap->TackMultiChange( x,y,z, sx, sy, sz );
}

int AllocAddInfo( int nr )
{
	return gh->AllocAddInfo( nr );
}

void AlterAddInfo( int pos, float x, float y, float z, float a )
{
	gh->AdditionalInformationMapData[pos] = RGBAf( x, y, z, a );
}

void MarkAddDataForReload()
{
	gh->MarkAddInfoForReload();
}


