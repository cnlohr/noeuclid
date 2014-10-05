//BIG NOTE on SUBTRACING:
//
//Subtracing is when we do everything all round.  This is achieved
//by effectively sliding all geometry up by .5 on all axes.  This
//effectively encapsulates all geometry in at least 1/2 of a block
//of virtual geometry.
//
//We then trace into this virtual geometry using the normal voxel
//method, and then once we might have a hit drop out.
//
//That's when subtrace takes over.  It starts by setting the corners
//of the current virtual voxel to be the blocks that the corners
//are in the center of (Remember, everything is offset by .5 in
//each direction).  Once This is done, it can step through the volume
//in small steps and compute the density (in accordance with mixfn.)
//Once it thinks it may have found the surface, it drops out and
//does a binary search refinement to find the actual surface.
//If no surface is found, it will drop back to the largescale ray
//tracer. If a surface is found, it will continue executing normally.

//This file assumes 8-character tabs.

//Direction of ray for this pixel (shooting off from Inital Camera)
//This is not necessairly normalized.
varying vec3 RayDirection;

//Normalized ray direction
vec3 dir;  //Original Direction
vec3 rdir; //Modified Direction

//Actual [REAL] value for the position of the camera.  This is
//before any of the tricks described below are applied.
varying vec3 InitialCamera;


//So, because of floating point error, we must always keep the
//camera close to the origin.  We can fix this by virtually
//moving the camera around. This means we have the camera between
//0..1 and we shift the entire map by whole voxels to give the
//illusion the camera is moving.

//Actual camera position [0..1]
vec3 CurrentCamera;

//Amount to slide texture by [0..1]
vec3 CameraOffset;

//Virtual offset of camera (in whole units)
vec3 FloorCameraPos;


//Geometry Texture.  This contains the following information:
//Red Channel:   Block Type < not ????
//Green Channel: Metadata / Cell Type? Or something like that?
//Blue Channel: "Density" of block according to addtex (use this to render)
//Alpha Channel: Actual Cell to Draw 
uniform sampler3D GeoTex;

//Additional Information Texture.  Like above, one pixel per voxel

//Red Channel:   Density of Block - largescale trace only. (is there a block here or not)
//Green Channel: [unassigned]
//Blue Channel:  additional info map lookup
//Alpha Channel: additional info map lookup.
uniform sampler3D AddTex;

//MovTex is the non-euclidian point.
uniform sampler3D MovTex;

uniform sampler2D AdditionalInformationMap;

//Cell Attribute Map
//This contains detailed information about all cells.  See it's use for
//more information.  TileAttributes.txt contains what is put into this
//texture.
uniform sampler2D AttribMap;


//Use this if on ATI (it's a bug)
//ATI Cards for some reasons address all of the textures by a miniscule amount
//sideways... This corrects that bug.
//#define ATI
#ifdef ATI
const vec2 offset=-vec2( 1./8192.);
const vec3 lshw = -vec3( 1./65536.);
#else
const vec2 offset=vec2( 0. );
const vec3 lshw = vec3( 0. );
#endif

//3D position of expected location along ray.
vec3 ptr;
vec3 optr;  //Original ptr.

//Last read voxel from AddTex.  It contains 'This' cell's information.
vec4 lastvox;

//lastmov is for the noneuclidian portion.
vec4 lastmov;
vec3 lastlastmov;

//Cellhit (for offset to cell with actual block data in it)
//This is only really used in subtrace mode.
// [0..1],[0..1],[0..1] for where in the cell contact was made.
vec3 cellhit = vec3( 0.5 ); 

//Similar - used in subtrace, but contains the offset for the selected point.
vec3 CellPoint = vec3( 0. );


//Size of voxel texture in pixels.
uniform float msX;
uniform float msY;
uniform float msZ;

//Multiplier to convert from world space coordinates into voxel map coordinates
vec3 msize = vec3( 1./msX, 1./msY, 1./msZ );

//total elapsed time.  Don't be shocked if this resets to zero. Considering
//this because over time, it could accumulate floating point error.
uniform float time;

//Normal of the surface we're hitting
vec3 normal = vec3( 0., 0., 1. );

//Total number of steps we've taken so far.
int step;

//Maximum steps we're permitted
const int maxsteps = 190;

//Maximum distance from camera we can go before it's treated as a "forever"
//const float maxdist = 256.;
varying float maxdist;

varying vec3 AuxRotation;
vec3 ARotation;


//For subtracing (these are where the local block pairs information goes).
//think of the values in these variables as the four points of a cube.
vec4 vecbot = vec4( 0., 0., 0., 0. );
vec4 vectop = vec4( 0., 0., 0., 0. );

//Local Distances - used for large-scale voxel tracing.
//This contains the farthest distance we can go on a given
//axis before we must consider checking to see if we hit something
vec3 dists;

//For Physics
//varying float doPhysics;
#define doPhysics 0.0

const float linearstep = .1;
const int binaryRefinements = 8;

//For pseudo z-buffering things...
float TotalDistanceTraversed;
float PerceivedDistanceTraversed;

uniform float mixval; //.9
uniform float densitylimit; //.2
uniform float densitymux; //1.0

//This is the shape mixing function for subtrace.
vec3 mixfn( vec3 ins, float mixa )
{
//Wonky function...
//	return mix( cos( ins*3.14159 + 3.14159 )/2.0+0.5, ins, mixa );

//Sharp edges...
//	return ins;

//Nice, smooth function we decided to use.
	vec3 coss = cos( ins*3.14159 + 3.14159 );
	vec3 sins = sign( coss );
	coss = abs( coss );
	coss = pow( coss, vec3( mixa ) );
	coss *= sins;

	return coss / 2.0 + 0.5;

}



//Large-scale traversal function
void TraverseIn( )
{
	//Load the firsxt voxel in.
	lastvox = texture3D( AddTex, ( floor(ptr) )*msize  + CameraOffset );
	lastmov = texture3D( MovTex, ( floor(ptr) )*msize  + CameraOffset );

	if( lastvox.a > 0.0 )
	{
		vec3 ra = texture2D( AdditionalInformationMap, vec2( lastvox.b+(0./255.), lastvox.a ) ).rgb;
		vec3 rb = texture2D( AdditionalInformationMap, vec2( lastvox.b+(1./255.), lastvox.a ) ).rgb;
		vec3 rc = texture2D( AdditionalInformationMap, vec2( lastvox.b+(2./255.), lastvox.a ) ).rgb;
		mat3 rm = mat3( ra, rb, rc );

		dir = dir * rm;
		ARotation = ARotation * rm;
		rdir = normalize( dir * lastmov.xyz );
		vec3 offset = texture2D( AdditionalInformationMap, vec2( lastvox.b+(3./255.), lastvox.a ) ).rgb;
		vec3 lp = CameraOffset/msize + ptr + offset;
		ptr = lp * rm - CameraOffset/msize; //-lp * rm;// + ( ptr + FloorCameraPos - pivot ) * rm;
	}

	//come up with vector to neutralize the sign on all computations
	//for traversal.  This makes it possible to always treat it like
	//we're tracing where all three direction components are positive.
	lastlastmov = (lastmov.xyz/lastmov.w);
	rdir = normalize( dir * lastlastmov );
	vec3 dircomps = -sign( rdir ); //+1 if negative, -1 if positive

	//Floor behaves: -0.5 -> -1 / 0.5 -> 0
	//Frac behaves:  -0.1 -> .9, -0.5 -> .5 / -1 becomes 0

	for( ; step < maxsteps && /*length( ptr-optr )*/PerceivedDistanceTraversed<maxdist; step++ )
	{
		//Find the distance to the edges of our local cube.  These
		//are always positive values from 0 to 1.

		vec3 nextsteps = fract( ptr * dircomps  );

		//Find out how many units the intersection point between us and
		//the next intersection is in ray space.
		dists = nextsteps / abs(rdir);

		//Find the closest axis.  We do this so we don't overshoot a hit.
		float mindist = 0.;
		mindist = min(dists.x,dists.y);
		mindist = min(mindist,dists.z);
		lastlastmov = (lastmov.xyz/lastmov.w);

		//Go there, plus a /tiny/ amount to prevent ourselves from hitting
		//an infinite loop.
		vec3 motion = (mindist+0.01) * rdir;
		ptr += motion;
		PerceivedDistanceTraversed += length( motion / lastlastmov );//motion * rdir;
		TotalDistanceTraversed += length( motion );

		//Load the new voxel
		lastvox = texture3D( AddTex, floor( ptr )*msize  + CameraOffset );
		lastmov = texture3D( MovTex, floor( ptr )*msize  + CameraOffset );

		//Jump?
/*		if( length( lastvox.gba ) > 0.01 )
		{
			ptr += vec3( lastvox.gba ) * 255.0;
		}
*/
		if( lastvox.a > 0 )
		{
			vec3 ra = texture2D( AdditionalInformationMap, vec2( lastvox.b+(0./255.), lastvox.a ) ).rgb;
			vec3 rb = texture2D( AdditionalInformationMap, vec2( lastvox.b+(1./255.), lastvox.a ) ).rgb;
			vec3 rc = texture2D( AdditionalInformationMap, vec2( lastvox.b+(2./255.), lastvox.a ) ).rgb;
			mat3 rm = mat3( ra, rb, rc );

			dir = dir * rm;
			ARotation = ARotation * rm;
			rdir = normalize( dir * lastmov.xyz );
			dircomps = sign( rdir );
			vec3 offset = texture2D( AdditionalInformationMap, vec2( lastvox.b+(3./255.), lastvox.a ) ).rgb;
			vec3 lp = CameraOffset/msize + ptr + offset;
			ptr = lp * rm - CameraOffset/msize; //-lp * rm;// + ( ptr + FloorCameraPos - pivot ) * rm;
		}

		rdir = normalize( dir * lastmov.xyz );
		
		//If it's a hit, we're good!  Return 
		if( lastvox.r > .1 )
		{
			return;
		}

	}

	//We ran out of runs, so we must pass a sentinal value into ptr.
	//-5000 means no geometry intersection.
	rdir = vec3( -5000. );

}

//You can make tileattributes update densities immediately by using the alternate (commented) code here.

float Density( vec3 ltexptr )
{
	return texture3D( GeoTex, ltexptr*msize + CameraOffset ).b * densitymux;
//	vec2 ltm = texture3D( AddTex, ltexptr*msize +CameraOffset ).rg;
//	return texture2D( AttribMap, vec2( ltm.g + (7./128.), ltm.r ) ).r;
}


void main()
{
	vec3 npos;

	//maxdist += .866; //0.5, 0.5, 0.5 offset.

	//Configure all variables as described in the beginning.
	CurrentCamera = mod( InitialCamera, 1.0 );
	FloorCameraPos = floor( InitialCamera );
	CameraOffset = mod( (FloorCameraPos * msize), 1.0 ) + lshw;
	ptr = CurrentCamera + vec3(0.5);
	optr = ptr;
	dir = normalize(RayDirection);
	step = 0;
	vec4 firstmov = texture3D( MovTex, ( floor(ptr) )*msize  + CameraOffset );
	PerceivedDistanceTraversed = 0.0;

	//For subtracing
	vec3 nlc;
	vec3 dists;
	float minq;
	float minm=10000.;
	float mixtot=0.;
	vec3 lc = vec3( 0. );
	vec3 texptr;
	bool bfound;
	float fmarch;
	npos = vec3( 0. );
	vec3 sshitpos = vec3(0.);
	vec4 closesthitstretchandtex = vec4( 0. );
	vec3 ssrealhitpos = vec3( 0. );
	float closestt = 1000000.;
	vec3 ssclose = vec3( 0. );

	float First = 1.0;
	TotalDistanceTraversed = 0.0;

	ARotation = AuxRotation;

	do
	{
		if( First > 0.5 )
		{
			lastvox = texture3D( AddTex, ( floor(ptr) )*msize  + CameraOffset );
			lastmov = texture3D( MovTex, ( floor(ptr) )*msize  + CameraOffset );

			if( lastvox.a > 0.0 )
			{
				dir = vec3( -dir.y, dir.x, dir.z );
				ptr += texture2D( AdditionalInformationMap, vec2( lastvox.b+(3./255.), lastvox.a ) ).rgb;
			}

			lastlastmov = (lastmov.xyz/lastmov.w);
			rdir = normalize( dir * lastlastmov );
			if( lastvox.r < .1 )
				TraverseIn( );
		}
		else
			TraverseIn();

		First = 0.0;

		//We ran off the end of the map.
		if( rdir.x < -4000. || (PerceivedDistanceTraversed > maxdist))
		{
			//May want to do something funny here... Right now we
			//just set the sky to blue.
			if( doPhysics > 0.5 )
			{
				//XXX WRONG XXX TODO (or I think it's wrong)
				gl_FragData[0] = vec4( vec3(1234.,0,0), TotalDistanceTraversed );
				gl_FragData[1] = vec4( lastlastmov, dot( normalize( lastlastmov ), normalize( dir ) ) );
				//XXX TODO: Figure out where we actually are!

				//Probably a physics overshoot... Back off.
				if( PerceivedDistanceTraversed > maxdist )
				{
					float overshoot = PerceivedDistanceTraversed - maxdist;
					ptr -= overshoot * ( dir * lastlastmov );
					PerceivedDistanceTraversed -= overshoot;
				}

				gl_FragData[2] = vec4( (InitialCamera+ptr-optr).xyz, PerceivedDistanceTraversed );
				gl_FragData[3] = vec4( dir, 0.0 );
			}
			else
			{
				gl_FragData[0] = vec4( 0., 0., 0., step );
				gl_FragData[1] = vec4( 0., 0., 0., -1. );

			}
			return;
		}





		if( lastvox.r > .99 )
		{


			//Calculate parameters for /this/ cell
			lc = fract(ptr.xyz);
			texptr = floor(ptr);

			//Handle inside-cell interpolation -> Find the values for the four surrounding cells.
	/*		float ptA = texture3D( AddTex, ( texptr - vec3(0.0,0.0,0.0) + 0.0 )*msize +CameraOffset ).r>0.?1.:0.;
			float ptB = texture3D( AddTex, ( texptr - vec3(1.0,0.0,0.0) + 0.0 )*msize +CameraOffset ).r>0.?1.:0.;
			float ptC = texture3D( AddTex, ( texptr - vec3(0.0,1.0,0.0) + 0.0 )*msize +CameraOffset ).r>0.?1.:0.;
			float ptD = texture3D( AddTex, ( texptr - vec3(1.0,1.0,0.0) + 0.0 )*msize +CameraOffset ).r>0.?1.:0.;
			float ptE = texture3D( AddTex, ( texptr - vec3(0.0,0.0,1.0) + 0.0 )*msize +CameraOffset ).r>0.?1.:0.;
			float ptF = texture3D( AddTex, ( texptr - vec3(1.0,0.0,1.0) + 0.0 )*msize +CameraOffset ).r>0.?1.:0.;
			float ptG = texture3D( AddTex, ( texptr - vec3(0.0,1.0,1.0) + 0.0 )*msize +CameraOffset ).r>0.?1.:0.;
			float ptH = texture3D( AddTex, ( texptr - vec3(1.0,1.0,1.0) + 0.0 )*msize +CameraOffset ).r>0.?1.:0.;
			vectop = min(vec4( ptA, ptB, ptC, ptD )*1000.,1.);
			vecbot = min(vec4( ptE, ptF, ptG, ptH )*1000.,1.);
	*/


			float ptA = Density( texptr - vec3(0.0,0.0,0.0) ); 
			float ptB = Density( texptr - vec3(1.0,0.0,0.0) ); 
			float ptC = Density( texptr - vec3(0.0,1.0,0.0) ); 
			float ptD = Density( texptr - vec3(1.0,1.0,0.0) ); 
			float ptE = Density( texptr - vec3(0.0,0.0,1.0) ); 
			float ptF = Density( texptr - vec3(1.0,0.0,1.0) ); 
			float ptG = Density( texptr - vec3(0.0,1.0,1.0) ); 
			float ptH = Density( texptr - vec3(1.0,1.0,1.0) ); 

			vectop = vec4( ptA, ptB, ptC, ptD );
			vecbot = vec4( ptE, ptF, ptG, ptH );

			npos = vec3( 0. );
			bfound = false;
			//Find the distance from one side of the cube to the other...
			nlc = (sign(rdir)+1.0)/2.0 - sign(rdir)* lc;
			dists = nlc / abs(rdir);
			minq = -1000.;
			minm = 0.0;
			mixtot = 0.0;

			float mindist = 0.;
			if( dists.x <= dists.y && dists.x <= dists.z )
				mindist = dists.x;
			else if( dists.y <= dists.x && dists.y <= dists.z )
				mindist = dists.y;
			else
				mindist = dists.z;

			bfound = false;

			//Linearly search through the block, trying to find the intersection.
			for( fmarch = 0.0; fmarch <= mindist+linearstep*.5; fmarch+=linearstep )
			{
				npos = lc + rdir * (fmarch);

				//You may notice here - we don't actually shoot a ray through.
				//We warp the ray as a function of mixfn.
				vec3 tnpos = mixfn( npos, mixval );
				mixtot = mix( 	mix( mix(vecbot.a, vecbot.b, tnpos.x ),
						mix( vecbot.g, vecbot.r, tnpos.x ), tnpos.y ),
						mix( mix(vectop.a, vectop.b, tnpos.x ),
						mix( vectop.g, vectop.r, tnpos.x ), tnpos.y ),
						tnpos.z );
				if( mixtot > densitylimit ) { bfound = true; minm = fmarch; break; }
				if( mixtot > minq ) { minq = mixtot; minm = fmarch; }
			}

			if( bfound )
				break;
		}
		//If not found, keep going on...
		step+=2;
	} while( step < maxsteps && (closestt > 10000. ) );


	vec3 startptr = ptr;
	if( (step >= maxsteps && closestt > 10000. ) || (PerceivedDistanceTraversed > maxdist) )
	{

		if( doPhysics > 0.5 )
		{
			gl_FragData[0] = vec4( vec3(12.,0,0), TotalDistanceTraversed );
			gl_FragData[1] = vec4( lastlastmov, dot( normalize( lastlastmov ), normalize( dir ) ) );

			//Probably a physics overshoot... Back off.
			if( PerceivedDistanceTraversed > maxdist )
			{
				float overshoot = PerceivedDistanceTraversed - maxdist;
				ptr -= overshoot * ( dir * lastlastmov );
				PerceivedDistanceTraversed -= overshoot;
			}

			gl_FragData[2] = vec4( (InitialCamera+ptr-optr).xyz, PerceivedDistanceTraversed );
			gl_FragData[3] = vec4( dir, 0.0 );
			return;
		}
		else
		{
			gl_FragData[0] = vec4( 0., 0., 0., step );
			gl_FragData[1] = vec4( 0., 0., 0., -100. );
			return;
		}
	}

	{
		//Binary search the remaining space.
		//This means we're a hit, and just want to get our XYZ location of the hit very precice.
		minm -= linearstep*0.5;
		fmarch = minm;

		float fmultiplier = linearstep * 0.5;
		for( int i = 0; i < binaryRefinements; i++ )
		{
			npos = lc + rdir * (fmarch);
			npos = 1.-npos;
			vec3 tnpos = mixfn( npos, mixval );
			mixtot = mix( 	mix( mix( vectop.r, vectop.g, tnpos.x ),
					mix(vectop.b, vectop.a, tnpos.x ), tnpos.y ),
					mix( mix( vecbot.r, vecbot.g, tnpos.x ),
					mix(vecbot.b, vecbot.a, tnpos.x ), tnpos.y ),
					tnpos.z );

			float fmuxsign = .5;
			if( mixtot > densitylimit ) { bfound = true; fmuxsign = -.5; }
			fmultiplier=abs(fmultiplier) * fmuxsign;
			fmarch += fmultiplier;
		}

		//Find normal...
		//We do this by inching a very small amount in each direction
		//to compute the gradiant of the density of the metasurface.
		//The normal is actually the normalized gradient.
		vec3 npx = npos + vec3( 0.01, 0., 0. );
		npx = mixfn( npx, mixval );

		float mixtotx = mix( 	mix( mix( vectop.r, vectop.g, npx.x ), mix(vectop.b, vectop.a, npx.x ), npx.y ),
					mix( mix( vecbot.r, vecbot.g, npx.x ), mix(vecbot.b, vecbot.a, npx.x ), npx.y ), npx.z );

		vec3 npy = npos + vec3( 0.0, 0.01, 0. );
		npy = mixfn( npy, mixval );

		float mixtoty = mix( 	mix( mix( vectop.r, vectop.g, npy.x ), mix(vectop.b, vectop.a, npy.x ), npy.y ),
					mix( mix( vecbot.r, vecbot.g, npy.x ), mix(vecbot.b, vecbot.a, npy.x ), npy.y ), npy.z );

		vec3 npz = npos + vec3( 0., 0., 0.01 );
		npz = mixfn( npz, mixval );

		float mixtotz = mix( 	mix( mix( vectop.r, vectop.g, npz.x ), mix(vectop.b, vectop.a, npz.x ), npz.y ),
					mix( mix( vecbot.r, vecbot.g, npz.x ), mix(vecbot.b, vecbot.a, npz.x ), npz.y ), npz.z );
		normal = normalize( vec3( mixtotx, mixtoty, mixtotz ) - mixtot );

		ptr += rdir * fmarch;
		PerceivedDistanceTraversed += length( (rdir*fmarch)/lastlastmov);
	}

	if( doPhysics > 0.5 )
	{
		gl_FragData[0] = vec4( normal.xyz, TotalDistanceTraversed );
		gl_FragData[1] = vec4( lastlastmov, dot( normalize( lastlastmov ), normalize( dir ) ) );

		//Probably a physics overshoot... Back off.
		if( PerceivedDistanceTraversed > maxdist )
		{
			float overshoot = PerceivedDistanceTraversed - maxdist;
			ptr -= overshoot * ( dir * lastlastmov );
			PerceivedDistanceTraversed -= overshoot;
		}

		gl_FragData[2] = vec4( (InitialCamera+ptr-optr).xyz, PerceivedDistanceTraversed );
		gl_FragData[3] = vec4( dir, 0.0 );
	}
	else
	{

//		fmarch = 10000.;
		float ldts = distance( ptr, startptr ) + TotalDistanceTraversed;
		if( closestt < 1000. && ( closestt < ldts || !bfound) )
		{
			normal = normalize(ssrealhitpos - ssclose);

			gl_FragData[0] = vec4( sshitpos, step );
			gl_FragData[1] = vec4( normal, closesthitstretchandtex.a );
			return;
		}

		gl_FragData[0] = vec4( ptr, step );
//		gl_FragData[0] = vec4( (ptr-optr).xyz, step );
		gl_FragData[1] = vec4( normal.xyz, 1. );
	}
	return;
}



