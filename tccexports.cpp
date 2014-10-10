#include "tccexports.h"
#include "tccengine.h"

#include <stdio.h>
#include <stdlib.h>
#include "noneuclidtcc.h"
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


