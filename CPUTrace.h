//UNUSED.

#ifndef _CPUTRACE_H
#define _CPUTRACE_H

#include "Map.h"

class CPUTrace
{
public:
	CPUTrace( class Map * m );

	//Pass: x, y, z, dirx, diry, dirz
//	float Trace( float * xyzposdir );
	float Trace( const float * pos, const float * dir, float * posout );
private:
	Map * m;
};

#endif

