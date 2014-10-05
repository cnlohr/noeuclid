//UNUSED

#include "CPUTrace.h"
#include "linmath.h"


CPUTrace::CPUTrace( class Map * m );
{
	this->m = m;
}

float CPUTrace::Trace( const float * pos, const float * dir, float * posout )
{
	//Yick this is a scary function, but it is necessairy.
	//Be careful to keep this in lock-step with "Pass1.frag"

/*	struct RGBA * GeoTex = m->GLTextureData[0];
	struct RGBA * AddTex = m->GLTextureData[1];
	struct RGBA * MovTex = m->GLTextureData[2];
*/
	//Use m->TexCell(i,x,y,z);

	float lpos[3];
	float ldir[3];

	copy3d( lpos, &xyzpos[0] );
	copy3d( ldir, &xyzpos[3] );

	//Into the mouth of the beast...
	

}
