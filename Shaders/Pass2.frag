//#define SUBTRACE_OVERRIDE
//#define ATI

//Use this if on ATI (it's a bug)
//ATI Cards for some reasons address all of the textures by a miniscule amount
//sideways... This corrects that bug.
#ifdef ATI
const vec2 offset=-vec2( 1./8192.);
const vec3 lshw = -vec3( 0./65536.);
#else
const vec2 offset=vec2( 0. );
const vec3 lshw = vec3( 0. );
#endif
//Uncomment this to override subtrace (may speed it up on some systems)
//By enabling the override, the GLSL compiler can drop some code on the floor
//thus decreasing the shader's footprint.
//In practice, this doesn't affect too much.
#ifdef SUBTRACE_OVERRIDE
const float do_subtrace = false;
#else
uniform float do_subtrace;
#endif

//Size of voxel texture in pixels.
uniform float msX;
uniform float msY;
uniform float msZ;

//Multiplier to convert from world space coordinates into voxel map coordinates
vec3 msize = vec3( 1./msX, 1./msY, 1./msZ );

//total elapsed time.  Don't be shocked if this resets to zero. Considering
//this because over time, it could accumulate floating point error.
uniform float time;


varying vec3 RayDirection;
varying vec3 InitialCamera;
varying vec2 PosInTex;

uniform sampler3D GeoTex;
uniform sampler3D AddTex;
uniform sampler2D AttribMap;
uniform sampler2D Pass1A;
uniform sampler2D Pass1B;
uniform sampler2D NoiseMap;

//Variables and functions for Olano-like Perlin noise generation
vec4 gNoise2( vec2 idata );
vec4 gNoise3( vec3 idata );
vec4 gNoise4( vec4 idata );
vec4 bNoise2( vec2 PLoc );
float pNoise3( vec3 PLoc );
float pNoise2( vec2 PLoc );

vec3 dir, ptr, CameraOffset,FloorCameraPos,CurrentCamera;

uniform float ScreenX;
uniform float ScreenY;

vec3 SunPos;
vec3 SunColor;
vec3 MoonPos;
vec3 MoonColor;
float DayAmount;
float NightAmount;
float daytime; //0...2pi


void main()
{
	CurrentCamera = mod( InitialCamera, 1.0 );
	FloorCameraPos = floor( InitialCamera );
	CameraOffset = mod( (FloorCameraPos * msize), 1.0 ) + lshw;
	dir = normalize(RayDirection);

	vec4 Position = texture2D( Pass1A, PosInTex );
	vec4 Normal = texture2D( Pass1B, PosInTex );

	daytime = mod( time*.01, 6.28318 );
	SunPos = vec3( -cos( daytime ), 0., sin( daytime ) );

	//Night time
	NightAmount = -SunPos.z + .1;
	NightAmount = min( NightAmount, .5 );
	MoonColor = vec3( .5, .6, .7 ) * NightAmount;
	MoonPos = -SunPos;

	DayAmount = SunPos.z + .1;
	DayAmount = clamp( DayAmount*2., 0., 1. );
	SunColor = (vec3( 0.6, .6, .4 ) + DayAmount*0.)*DayAmount;

//Show positions of things
//	gl_FragColor = vec4( mod( Position.xyz, 1. ), 1. );
//	return;

	//Look for pixel-wide holes in the render, and fill them in. This slows it down for some reason so I commented it out.
/*	if( Normal.a < .5 )
	{
		for( float lx = PosInTex.x-1./ScreenX; lx < PosInTex.x+2./ScreenX; lx += 1.01/ScreenX )
		for( float ly = PosInTex.y-1./ScreenY; ly < PosInTex.y+2./ScreenY; ly += 1.01/ScreenY )
		{
			if( lx >= 1. || ly >= 1. || lx < 0. || ly < 0. ) continue;
			if( texture2D( Pass1B,  vec2( lx, ly ) ).a >= .5 )
			{
				Position = texture2D( Pass1A,  vec2( lx, ly )  );
				Normal = texture2D( Pass1B, vec2( lx, ly ) );
				break;
			}
		}
	}
*/
	if( Normal.a < .5 )
	{
//		gl_FragColor = vec4( CalcSky(), 1. );
		discard;
		return;
	}

	ptr = Position.xyz;
	vec3 CellPoint;

	vec3 ppmod = 1.-mod(ptr.xyz ,1.)/1.;
	vec3 GlobalPos = FloorCameraPos + ptr;

	//This may look unusual - but it's needed to find the actual cell we should draw
	//onto this location.  The reason for this is because sometimes the cell you draw
	//is physically in the 'wrong' place - but we want this effect.  Additionally, this
	//has the neat feature of adding some noise
	if( do_subtrace >= 0.5 )
	{
		vec3 texptr = floor( ptr.xyz );
		float ptA = texture3D( GeoTex, ( texptr - vec3(0.0,0.0,0.0) + 0.0 )*msize + CameraOffset ).r>0.?1.:0.;
		float ptB = texture3D( GeoTex, ( texptr - vec3(1.0,0.0,0.0) + 0.0 )*msize + CameraOffset ).r>0.?1.:0.;
		float ptC = texture3D( GeoTex, ( texptr - vec3(0.0,1.0,0.0) + 0.0 )*msize + CameraOffset ).r>0.?1.:0.;
		float ptD = texture3D( GeoTex, ( texptr - vec3(1.0,1.0,0.0) + 0.0 )*msize + CameraOffset ).r>0.?1.:0.;
		float ptE = texture3D( GeoTex, ( texptr - vec3(0.0,0.0,1.0) + 0.0 )*msize + CameraOffset ).r>0.?1.:0.;
		float ptF = texture3D( GeoTex, ( texptr - vec3(1.0,0.0,1.0) + 0.0 )*msize + CameraOffset ).r>0.?1.:0.;
		float ptG = texture3D( GeoTex, ( texptr - vec3(0.0,1.0,1.0) + 0.0 )*msize + CameraOffset ).r>0.?1.:0.;
		float ptH = texture3D( GeoTex, ( texptr - vec3(1.0,1.0,1.0) + 0.0 )*msize + CameraOffset ).r>0.?1.:0.;

		vec4 PN1 = vec4( pNoise3( GlobalPos.xyz + 1.0 ), pNoise3( GlobalPos.xyz + 2.0 ),
				 pNoise3( GlobalPos.xyz + 3.0 ), pNoise3( GlobalPos.xyz + 4.0 ) );
		vec4 PN2 = (1.-PN1);

		vec4 vectop = vec4( ptA, ptB, ptC, ptD ) * (PN1*.14+1.);
		vec4 vecbot = vec4( ptE, ptF, ptG, ptH ) * (PN2*.14+1.);

		vec4 vectopsA = vectop * vec4( 1.-ppmod.x,ppmod.x,1.-ppmod.x,ppmod.x ) * (1.-ppmod.z) * vec4( vec2( 1.-ppmod.y ), vec2(ppmod.y ) );
		vec4 vecbotsA = vecbot * vec4( 1.-ppmod.x,ppmod.x,1.-ppmod.x,ppmod.x ) * (ppmod.z) * vec4( vec2( 1.-ppmod.y ), vec2(ppmod.y ) );
		vec4 minsA = max( vectopsA, vecbotsA );
		vec2 minsB = max( minsA.xy, minsA.zw );
		float amin = max( minsB.x, minsB.y );
		if( vectopsA.x == amin ) CellPoint = vec3( 0., 0., 0. );
		else if( vectopsA.y == amin ) CellPoint = vec3( 1., 0., 0. );
		else if( vectopsA.z == amin ) CellPoint = vec3( 0., 1., 0. );
		else if( vectopsA.w == amin ) CellPoint = vec3( 1., 1., 0. );
		else if( vecbotsA.x == amin ) CellPoint = vec3( 0., 0., 1. );
		else if( vecbotsA.y == amin ) CellPoint = vec3( 1., 0., 1. );
		else if( vecbotsA.z == amin ) CellPoint = vec3( 0., 1., 1. );
		else CellPoint = vec3( 1., 1., 1. );
	}
	else
	{
		CellPoint = vec3(0.0);
	}

	ppmod = mod( ppmod -(do_subtrace*.5), 1.0 );
	vec4 ExtraData = texture3D( GeoTex, floor( ptr - CellPoint)*msize + CameraOffset );

	//Discard 0xFF. It's a "Done" flag.
	if( ExtraData.a >= 1.0 ) 
	{
		gl_FragColor = vec4( 0. );
		return;
	}

	vec3 OutColor;

	float ID = ExtraData.a;
	if( Normal.a >= 10. ) ID = (Normal.a - 10.)/256.;

	vec4 BaseColor = texture2D( AttribMap, vec2( (0./128.), ID ) );
	vec4 NoiseColor = texture2D( AttribMap, vec2( (1./128.), ID ) );
	vec4 NoiseSet = texture2D( AttribMap, vec2( (2./128.), ID ) );
	vec4 NoiseMux = texture2D( AttribMap, vec2( (3./128.), ID ) );
	vec4 CoreData = texture2D( AttribMap, vec2( (4./128.), ID ) ); //For trees, etc.
	vec4 TimeSettings = texture2D( AttribMap, vec2( (5./128.), ID ) );
	vec4 Speckles = texture2D( AttribMap, vec2( (6./128.), ID ) );
	vec4 ShaderEndingTerms = texture2D( AttribMap, vec2( (7./128.), ID ) );

	vec3 noiseplace = NoiseSet.xyz * vec3( GlobalPos ) + TimeSettings.xyz * time;
	float noise = pNoise3( noiseplace ) * NoiseMux.r + pNoise3( noiseplace * 2. ) * NoiseMux.g +
		      pNoise3( noiseplace * 4. ) * NoiseMux.b + pNoise3( noiseplace * 8. ) * NoiseMux.a + NoiseColor.a;

	float core = length( (ppmod.xyz-.5) * vec3( 1., 1., CoreData.y ) ) * CoreData.x;
	core = mod( core+noise, CoreData.z ) * CoreData.a;

	noise = noise+core;
	vec3 NoiseOut = noise * ((noise<Speckles.a)?Speckles.rgb:NoiseColor.rgb);
	noise = abs( noise );
	OutColor = BaseColor.rgb + NoiseOut;


	//Lighting color
	vec3 Sunamt = max( dot(Normal.xyz, SunPos),  0. ) * SunColor;
	vec3 Moonamt = max( dot(Normal.xyz, MoonPos),  0. ) * MoonColor;

	vec3 Lighting = Sunamt+Moonamt;

//Lighting  (tricky - need to find the next cell away from where we are to get lighting info)
/*	vec3 nrmmax = abs(Normal.xyz);
	vec3 nrmady;
	vec3 s = sign( Normal.xyz );
	if( nrmmax.x > nrmmax.y && nrmmax.x > nrmmax.z )
		nrmady = vec3( 1., 0., 0. ) * s;
	else if( nrmmax.y > nrmmax.z )
		nrmady = vec3( 0., 1., 0. ) * s;
	else
		nrmady = vec3( 0., 0., 1. ) * s;
*/
	//the idea is to use the normal to step back to the previous block to get the light info
	vec3 backPedal = normalize(Normal.xyz);
//	vec4 LightCell = texture3D( AddTex, floor( ptr +.5 + nrmady*.4 )*msize + CameraOffset );
	vec4 LightCell = texture3D( GeoTex, floor( ptr + backPedal )*msize + CameraOffset );
//	vec4 LightCell = texture3D( AddTex, floor( ptr )*msize  + CameraOffset );

	float skylight = pow( floor(mod(LightCell.g*16.,16.))/16., 2. );
	float blocklight = pow( mod( LightCell.g*255., 16. )/16., 2.) ;
//gl_FragColor = blocklight;
//return;
	Lighting = Lighting * skylight*0.8 + blocklight*.4;

//	Lighting = mix( Lighting, vec3(1.), BaseColor.a );

	vec3 FinalColor = vec3( 0., 0., 0. );

	FinalColor += OutColor * ShaderEndingTerms.b;

	OutColor = min( OutColor, vec3(2.,2.,2.) );
	OutColor *= Lighting;

	FinalColor += ShaderEndingTerms.r * OutColor;

	vec3 lightingDir = normalize(vec3(1,2,1));
	// light 1
	gl_FragColor.rgb = mix(
		OutColor,
		vec3(1,0.8,0.5) * clamp(dot(backPedal,lightingDir)*0.4,0.0,1.0),
		0.5);
	// light 2
	FinalColor += vec3(0.2,0.3,0.4) * clamp(backPedal.y*0.4,0.0,1.0) * ShaderEndingTerms.g;
//	gl_FragColor.rgb = vec3(LightCell.b);


	gl_FragColor = vec4( FinalColor, 1. );

	return;

}





























//Do not try to understand this code.  It's based (poorly) off of Dr. Olano's
//Modified perlin noise for evaluation on graphics hardware.

const float NoiseWidth  = 16.0;
const float NoiseHeight  = 16.0;

vec4 gNoise2( vec2 idata )
{
	vec4 iNoise = texture2D( NoiseMap, idata + offset );
	vec4 tNoise = texture2D( NoiseMap, iNoise.xy + floor(idata)/3.2456 + offset );
	return tNoise;
}
vec4 gNoise3( vec3 idata )
{
	vec4 iNoise = texture2D( NoiseMap, idata.xy +offset );
	vec4 iNoiseB = texture2D( NoiseMap, vec2(idata.z,idata.z) + offset );
	vec4 tNoise = texture2D( NoiseMap, offset + iNoise.xy + iNoiseB.xy + vec2( floor(idata.xy)/3.2456 )  + vec2( floor(idata.z) )/3.52456 );
	return tNoise;
}
float pNoise2( vec2 PLoc )
{
	vec2 MixDist = (PLoc*NoiseWidth - floor( PLoc*NoiseWidth ));
	vec2 iMixDist = -(1.0 - MixDist);

	//Original Perlin Noise
	vec2 FnMix = 3.0 * MixDist*MixDist - 2.0 * MixDist * MixDist * MixDist;

	//Improved Perlin Noise
//	vec2 MDSquared = MixDist*MixDist;
//	vec2 MDCubed = MDSquared*MixDist;
//	vec2 FnMix = 6.0 * MDCubed*MDSquared  - 15.0 * MDSquared * MDSquared + 10. * MDCubed;

	vec4 Upper = vec4(
		gNoise2( PLoc ).xy,
		gNoise2( PLoc + vec2( 1.0/NoiseWidth, 0.0 ) ).xy );
	vec4 Lower = vec4(
		gNoise2( PLoc + vec2( 0.0, 1.0/NoiseHeight ) ).xy,
		gNoise2( PLoc + vec2( 1.0/NoiseWidth, 1.0/NoiseHeight ) ).xy );
	vec4 US = (floor( Upper * 2.0 ) - 0.5)*2.0;
	vec4 LS = (floor( Lower * 2.0 ) - 0.5)*2.0;
	vec4 Mix = vec4(
		dot( US.xy, vec2( MixDist.x, MixDist.y) ),
		dot( US.zw, vec2( iMixDist.x, MixDist.y) ),
		dot( LS.xy, vec2( MixDist.x, iMixDist.y) ),
		dot( LS.zw, vec2( iMixDist.x, iMixDist.y) ) );


	return mix( mix( Mix.x, Mix.y, FnMix.x ), mix( Mix.z, Mix.w, FnMix.x ), FnMix.y );
}
float pNoise3( vec3 PLoc )
{
	vec3 MixDist = (PLoc*NoiseWidth - floor( PLoc*NoiseWidth ));
	vec3 iMixDist = -(1.0 - MixDist);

	//Original Perlin Noise
	vec3 FnMix = 3.0 * MixDist*MixDist - 2.0 * MixDist * MixDist * MixDist;

	//Improved Perlin Noise
//	vec2 MDSquared = MixDist*MixDist;
//	vec2 MDCubed = MDSquared*MixDist;
//	vec2 FnMix = 6.0 * MDCubed*MDSquared  - 15.0 * MDSquared * MDSquared + 10. * MDCubed;

	vec3 UL = gNoise3( PLoc ).xyz;
	vec3 UR = gNoise3( PLoc + vec3( 1.0/NoiseWidth, 0.0, 0.0 ) ).xyz;
	vec3 LL = gNoise3( PLoc + vec3( 0.0, 1.0/NoiseHeight, 0.0 ) ).xyz;
	vec3 LR = gNoise3( PLoc + vec3( 1.0/NoiseWidth, 1.0/NoiseHeight,0.0 ) ).xyz;
	vec3 ULS = (floor( UL * 2.0 ) - 0.5)*2.0;
	vec3 URS = (floor( UR * 2.0 ) - 0.5)*2.0;
	vec3 LLS = (floor( LL * 2.0 ) - 0.5)*2.0;
	vec3 LRS = (floor( LR * 2.0 ) - 0.5)*2.0;
	vec4 MixA = vec4(
		dot( ULS.xyz, vec3( MixDist.x, MixDist.y, MixDist.z) ),
		dot( URS.xyz, vec3( iMixDist.x, MixDist.y, MixDist.z) ),
		dot( LLS.xyz, vec3( MixDist.x, iMixDist.y, MixDist.z) ),
		dot( LRS.xyz, vec3( iMixDist.x, iMixDist.y, MixDist.z) ) );

	UL = gNoise3( PLoc + vec3( 0.0, 0.0, 1.0/NoiseHeight ) ).xyz;
	UR = gNoise3( PLoc + vec3( 1.0/NoiseWidth, 0.0, 1.0/NoiseHeight ) ).xyz;
	LL = gNoise3( PLoc + vec3( 0.0, 1.0/NoiseHeight, 1.0/NoiseHeight ) ).xyz;
	LR = gNoise3( PLoc + vec3( 1.0/NoiseWidth, 1.0/NoiseHeight, 1.0/NoiseHeight ) ).xyz;
	ULS = (floor( UL * 2.0 ) - 0.5)*2.0;
	URS = (floor( UR * 2.0 ) - 0.5)*2.0;
	LLS = (floor( LL * 2.0 ) - 0.5)*2.0;
	LRS = (floor( LR * 2.0 ) - 0.5)*2.0;
	vec4 MixB = vec4(
		dot( ULS.xyz, vec3( MixDist.x, MixDist.y, iMixDist.z) ),
		dot( URS.xyz, vec3( iMixDist.x, MixDist.y, iMixDist.z) ),
		dot( LLS.xyz, vec3( MixDist.x, iMixDist.y, iMixDist.z) ),
		dot( LRS.xyz, vec3( iMixDist.x, iMixDist.y, iMixDist.z) ) );




	return mix(
		mix( mix( MixA.x, MixA.y, FnMix.x ), mix( MixA.z, MixA.w, FnMix.x ), FnMix.y ),
		mix( mix( MixB.x, MixB.y, FnMix.x ), mix( MixB.z, MixB.w, FnMix.x ), FnMix.y ),
		FnMix.z );
}
