#ifndef _MAP_H
#define _MAP_H

#include <stdio.h>
#include <math.h>
#include <list>
#include "Common.h"
#include <OGLParts.h>


//MAP Contains "GLTextureData"


using namespace std;
class RTHelper;
/*
class DisplayUpdateChunk
{
public:
	int x;
	int y;
	int z;
	int sx;
	int sy;
	int sz;

	RGBA * UpdatedData[2];
};*/

class CellUpdate
{
public:
	CellUpdate () : x(0), y(0), z(0), sx(1), sy(1), sz(1) { }
	int x, y, z;
	int sx, sy, sz;
};

class Map
{
public:
	Map( string filename, RTHelper * p, bool fakemode );
	~Map(); 
	void FakeIt();
	void DefaultIt();
	void RecalculateAccelerationStructure( int ix, int iy, int iz, int sx, int sy, int sz );
	void Draw();

	list< CellUpdate > ListUpdates;
	void TackChange( int x, int y, int z ) { CellUpdate cu; cu.x = x; cu.y = y; cu.z = z; ListUpdates.push_back( cu ); }
	void TackMultiChange( int x, int y, int z, int sx, int sy, int sz ) { CellUpdate cu; cu.x = x; cu.y = y; cu.z = z; cu.sx = sx; cu.sy = sy; cu.sz = sz; ListUpdates.push_back( cu ); }


//	void PushUpdate( const DisplayUpdateChunk & u );
//	list< DisplayUpdateChunk > DisplayUpdatesInternal;
//	list< DisplayUpdateChunk > DisplayUpdates;


	inline RGBA & TexCell( unsigned i, int x, int y, int z )
	{
		x = ((unsigned long)x)%GLH_SIZEX;
		y = ((unsigned long)y)%GLH_SIZEY;
		z = ((unsigned long)z)%GLH_SIZEZ;
		return GLTextureData[i][x+y*GLH_SIZEX+z*GLH_SIZEX*GLH_SIZEY];
	}

	bool m_bTriggerFullRecalculate;
	bool m_bReloadFullTexture;

private:
	void UpdateCellSpecific( int x, int y, int z, int sx, int sy, int sz );

	void SetCellInternal( unsigned x, unsigned y, unsigned z, unsigned cell, unsigned color );

/*
	void BlockUpdate( const DisplayUpdateChunk & c );
	void BlockUpdatePart2( const DisplayUpdateChunk & c );
	DisplayUpdateChunk MakeDUC( int x, int y, int z, int sx, int sy, int sz );
*/

	//Comp 0("GeoTex"): 
		//Red Channel:   Block Type < not ????
		//Green Channel: Metadata / Cell Type? Or something like that?
		//Blue Channel: "Density" of block according to addtex (use this to render)
		//Alpha Channel: Actual Cell to Draw 
	//Comp 1("AddTex"): [possible hit?, jumpx, jumpy, jumpz]
	//Comp 2("MovTex"): [x] [y] [z] [w] (Universe-warping)
	RGBA * GLTextureData[3]; 
	unsigned int  i3DTex[3];

	bool doSubtrace;
	RTHelper * parent;

public:
	void ChangeSubtrace ( bool bSubtrace ) { doSubtrace = bSubtrace; m_bTriggerFullRecalculate = true; }
	bool GetSubtrace() { return doSubtrace; }

	void UpdateSphereTexture();
};

#endif
