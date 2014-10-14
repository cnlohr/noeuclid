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

#ifndef _SCRIPTHELPERS_H
#define _SCRIPTHELPERS_H

#include "noeuclidtcc.h"

#include "Common.h"

#define PICKABLE_CELL 114
#define DEFAULT_BRIGHT 190
#define DEFAULT_DENSITY 125
#define WALL_BLOCK 12
#define WHITE_BLOCK 19
#define GOAL_BLOCK 24
//#define DEADGOAL_BLOCK 25
#define DEADGOAL_BLOCK 7
#define DEFAULT_EMPTY_BLOCK 0
#define SPAAACE_CELL 255

#define CLAMP( x, mi, ma ) ( ((x)<(mi))?(mi):( ((x)>(ma))?(ma):(x) ) )

//Read an array with looping ends
char loopingarrayaccess( char * map, int w, int h, int x, int y)
{
	x = ((x%w)+w)%w;
	y = ((y%h)+h)%h;
	return map[x+y*w];
}

//Make a triangle wave.
float swoovey( float f, float siny )
{
	if( f < 0 ) f *= -1;
	f = fmodf( f, 1.0 );
	float fs = sin( f * 3.14159 * 2 );
//	if( f > .5 ) f = 1 - f;
//	f = f * 4.0 - 1.0;
//	if( f < 0 ) f = -pow( -f, siny );
//	else f = pow( f, siny );
//	return f;

	if( fs < 0 ) fs = -pow( -fs, siny );
	else fs = pow( fs, siny );
	return fs;
}

typedef void (*ClickCellCBType)( int left, float x, float y, float z, float dist );
ClickCellCBType ClickCellCB;

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

void MakeJumpSection( short x, short y, short z, short sx, short sy, short sz, int xofs, int yofs, int zofs, float * farray )
{
	int newalloc = AllocAddInfo(4);

	if( farray )
	{
		AlterAddInfo( newalloc+0, farray[0], farray[1], farray[2], 0 );
		AlterAddInfo( newalloc+1, farray[3], farray[4], farray[5], 0 );
		AlterAddInfo( newalloc+2, farray[6], farray[7], farray[8], 0 );
	}
	else
	{
		AlterAddInfo( newalloc+0, 1, 0, 0, 0 );
		AlterAddInfo( newalloc+1, 0, 1, 0, 0 );
		AlterAddInfo( newalloc+2, 0, 0, 1, 0 );
	}
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
 

#define MAX_PICKABLES 256
extern int pickables_in_inventory;
//pickable block helper.
struct PickableBlock
{
	int x, y, z;
	float phasing;  //If +1 or 0, block is truly here.  If -1, phasing out.  Note: 0 = do not re-update.
	float density;  //between 0 and 1.
	char in_use;
} PBlocks[MAX_PICKABLES];

//Returns id if pickable is there.
//Returns -1 if no block.
//Returns -2 if block tween incomplete.
int GetPickableAt( int x, int y, int z )
{
	int i;
	for( i = 0; i < MAX_PICKABLES; i++ )
	{
		struct PickableBlock * pb = &PBlocks[i];
		if( pb->in_use && pb->x == x && pb->y == y && pb->z == z )
		{
			return (pb->density > .99)?i:-2;
		}
	}
	return -1;
}

void DissolvePickable( int pid )
{
	struct PickableBlock * pb = &PBlocks[pid];
	pb->phasing = -1;
}

//If -1, no pickables left.
//If -2, Pickable already there.
//Otherwise returns which pickable!
//initial_density should be 0 unless you want to shorten (+) or lengthen (-) tween.
int PlacePickableAt( int x, int y, int z, float initial_density )
{
	int i;

	for( i = 0; i < MAX_PICKABLES; i++ )
	{
		struct PickableBlock * pb = &PBlocks[i];
		if( !pb->in_use )
		{
			break;
		}
	}
	if( i == MAX_PICKABLES )
		return -2;

	struct PickableBlock * pb = &PBlocks[i];
	pb->x = x; pb->y = y; pb->z = z;
	pb->phasing = 1;
	pb->in_use = 1;
	pb->density = initial_density;
}

void ClearPicableBlocks()
{
	int i;
	for( i = 0; i < MAX_PICKABLES; i++ )
	{
		struct PickableBlock * pb = &PBlocks[i];
		pb->x = pb->y = pb->z = 0;
		pb->phasing = 0;
		pb->density = 0;
		pb->in_use = 0;
	}
	pickables_in_inventory = 0;
}

//Redraw 
void UpdatePickableBlocks()
{
	int i;
	for( i = 0; i < MAX_PICKABLES; i++ )
	{
		struct PickableBlock * pb = &PBlocks[i];

		if( !pb->in_use ) continue;

		pb->density += worldDeltaTime * pb->phasing;

		if( pb->density >= 1.0 && pb->phasing > 0 )
		{
			pb->density = 1.0;
			pb->phasing = 0;
		}
		if( pb->density <= 0 && pb->phasing < 0 )
		{
			pb->density = 0.0;
			pb->phasing = 0.0;
			pb->in_use = 0;
		}
		float drawden = (pb->density<0)?0.0:((pb->density>1.0)?1.0:pb->density);

		if( drawden > .01 )
		{
			PaintRange( pb->x, pb->y, pb->z, 1, 1, 1, PICKABLE_CELL, drawden * 200 );
		}
		else
		{
			ClearCell( pb->x, pb->y, pb->z );
		}

	}
}

void PickableClick( int left, float x, float y, float z, float dist )
{
	if( dist > 4 ) return;
	int lx = x;
	int ly = y;
	int lz = z;
	if( left )
	{
		if( pickables_in_inventory > 0 )
		{
			PlacePickableAt( lx, ly, lz, 0.0 );
			pickables_in_inventory--;
		}
		else
		{
			printf( "No blocks.\n" );
		}
	}
	else
	{
		int r = GetPickableAt( lx, ly, lz );

		if( r >= 0 )
		{
			pickables_in_inventory++;
			PBlocks[r].phasing = -1;
		}

	}
}


//Define all lava/death blocks.
//If the user touches one, he dies.
#define MAX_DEATH_BLOCKS 8192

struct DeathBlock
{
	int x, y, z;
	int in_use;
} DeathBlocks[MAX_DEATH_BLOCKS];

void AddDeathBlock( int x, int y, int z )
{
	int i;

	//Don't add multiple.
	for( i = 0; i < MAX_DEATH_BLOCKS; i++ )
	{
		struct DeathBlock * db = &DeathBlocks[i];
		if( db->x == x && db->y == y && db->z == z )
		{
			return;
		}
	}

	for( i = 0; i < MAX_DEATH_BLOCKS; i++ )
	{
		struct DeathBlock * db = &DeathBlocks[i];
		if( db->in_use == 0 )
		{
			db->in_use = 1;
			db->x = x;
			db->y = y;
			db->z = z;
			break;
		}
	}
}

int IsOnDeathBlock( int x, int y, int z )
{
	int i;
	for( i = 0; i < MAX_DEATH_BLOCKS; i++ )
	{
		struct DeathBlock * db = &DeathBlocks[i];
		if( db->x == x && db->y == y && ( db->z == z || db->z == z-1 ) )
		{
			return 1;
		}
	}
	return 0;
}


#endif

