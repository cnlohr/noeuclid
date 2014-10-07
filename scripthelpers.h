#ifndef _SCRIPTHELPERS_H
#define _SCRIPTHELPERS_H

#include "noneuclidtcc.h"

#include "Common.h"


#define DEFAULT_BRIGHT 190
#define DEFAULT_DENSITY 128
#define WALL_BLOCK 12
#define WHITE_BLOCK 19
#define GOAL_BLOCK 24
#define DEADGOAL_BLOCK 25
#define DEFAULT_EMPTY_BLOCK 0
#define SPAAACE_CELL 255

#define CLAMP( x, mi, ma ) ( ((x)<(mi))?(mi):( ((x)>(ma))?(ma):(x) ) )

//File for useful scripts.

//Defines outside extents.

void ClearCell( short x, short y, short z )
{
	ChangeCell( 0, x, y, z, 0, DEFAULT_BRIGHT, 0, DEFAULT_EMPTY_BLOCK );
}

void ClearRange( short x, short y, short z, short sx, short sy, short sz )
{
	int i, j, k;
	for( i = x; i < x+sx; i++ )
	for( j = y; j < y+sy; j++ )
	for( k = z; k < z+sz; k++ )
	{
		QuickCell( 0, i, j, k, 0, DEFAULT_BRIGHT, 0, DEFAULT_EMPTY_BLOCK );
	}
	UpdateZone( x, y, z, sx+1, sy+1, sz+1 );
}

void PaintRange( short x, short y, short z, short sx, short sy, short sz, int cell, int density )
{
	int i, j, k;
	for( i = x; i < x+sx; i++ )
	for( j = y; j < y+sy; j++ )
	for( k = z; k < z+sz; k++ )
	{
		QuickCell( 0, i, j, k, 1, DEFAULT_BRIGHT, density, cell );
	}
	UpdateZone( x, y, z, sx+1, sy+1, sz+1 );
}

void MakeEmptyBox( short x, short y, short z, short sx, short sy, short sz, short cell, short defden, short bright, int force_empty )
{

	//Will cause an update over our whole area.
	if( force_empty )
		ClearRange( x, y, z, sx, sy, sz );
	else
		UpdateZone( x, y, z, sx+1, sy+1, sz+1 );


	short j, k;
	for( j = x; j <= x + sx; j++ )
		for( k = y; k <= y + sy; k++ )
		{
			QuickCell( 0, j, k, z, 1, bright, defden, cell );
			QuickCell( 0, j, k, z+sz, 1, bright, defden, cell );
		}

	for( j = z; j <= z + sz; j++ )
		for( k = x; k <= x + sx; k++ )
		{
			QuickCell( 0, k, y, j, 1, bright, defden, cell );
			QuickCell( 0, k, y+sy, j, 1, bright, defden, cell );
		}

	for( j = z; j <= z + sz; j++ )
		for( k = y; k <= y + sy; k++ )
		{
			QuickCell( 0, x, k, j, 1, bright, defden, cell );
			QuickCell( 0, x+sx, k, j, 1, bright, defden, cell );
		}

}

void MakeJumpSection( short x, short y, short z, short sx, short sy, short sz, int xofs, int yofs, int zofs )
{
	int newalloc = AllocAddInfo(4);

	AlterAddInfo( newalloc+0, 1, 0, 0, 0 );
	AlterAddInfo( newalloc+1, 0, 1, 0, 0 );
	AlterAddInfo( newalloc+2, 0, 0, 1, 0 );
	AlterAddInfo( newalloc+3, xofs, yofs, zofs, 0 );

	short i, j, k;
	for( i = x; i <= x + sx; i++ )
	for( j = y; j <= y + sy; j++ )
	for( k = z; k <= z + sz; k++ )
	{
		QuickCell1GBAOnly( i, j, k, 0, newalloc%AddSizeStride, newalloc/AddSizeStride );
	}
	UpdateZone( x, y, z, sx+1, sy+1, sz+1 );
	MarkAddDataForReload();
}

void SetWarpSpaceArea( short x, short y, short z, short sx, short sy, short sz, float xcomp, float ycomp, float zcomp )
{
	//0..255, 0..255, 0..255, 0..255
	//Bit compression = xyz/w

	float maxcomp = (xcomp>ycomp)?((xcomp>zcomp)?xcomp:zcomp):((ycomp>zcomp)?ycomp:zcomp);
	if( maxcomp < 1.0 ) maxcomp = 1.0;
	float cxc = xcomp / maxcomp;
	float cyc = ycomp / maxcomp;
	float czc = zcomp / maxcomp;
	float wterm = 1./maxcomp;
	CLAMP( cxc, 0, 1 );
	CLAMP( cyc, 0, 1 );
	CLAMP( czc, 0, 1 );
	CLAMP( wterm, 0, 1 );

	unsigned char xt = cxc * 255;
	unsigned char yt = cyc * 255;
	unsigned char zt = czc * 255;
	unsigned char wt = wterm * 255;

//	float xyzpres = sqrt( xcomp*xcomp + ycomp*ycomp + zcomp*zcomp );
	
	short i, j, k;
	for( i = x; i <= x + sx; i++ )
	for( j = y; j <= y + sy; j++ )
	for( k = z; k <= z + sz; k++ )
	{
		QuickCell( 2, i, j, k, xt, yt, zt, wt  );
	}
	UpdateZone( x, y, z, sx+1, sy+1, sz+1 );
}

int IsPlayerInRange( float x, float y, float z, float sx, float sy, float sz )
{
	if( gPositionX >= x && gPositionX <= x+sx &&
		gPositionY >= y && gPositionY <= y+sy &&
		gPositionZ >= z && gPositionZ <= z+sz ) return 1;
	return 0;
}
 


#endif

