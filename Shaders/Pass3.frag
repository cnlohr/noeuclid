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
uniform sampler2D Pass2;

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
vec3 CalcSky();

float skybright;
vec3 SkyBrightPos;

varying mat4 toss;

void main()
{
	CurrentCamera = mod( InitialCamera, 1.0 );
	FloorCameraPos = floor( InitialCamera );
	CameraOffset = mod( (FloorCameraPos * msize), 1.0 ) + lshw;
	dir = normalize(RayDirection);

	vec4 Pass1Position = texture2D( Pass1A, PosInTex );
	vec4 Pass1Normal = texture2D( Pass1B, PosInTex );
	vec4 Pass2Data = texture2D( Pass2, vec2( PosInTex.x, 1.-PosInTex.y ) );

	daytime = mod( time*.01, 6.28318 );
	SunPos = vec3( -cos( daytime ), 0., sin( daytime ) );

	//Night time
	NightAmount = -SunPos.z + .1;
	NightAmount = min( NightAmount, .5 );
	MoonColor = vec3( .5, .6, .7 ) * NightAmount;
	MoonPos = -SunPos;

	DayAmount = SunPos.z + .1;
	DayAmount = clamp( DayAmount*2., 0., 1. );
	SunColor = (vec3( .9, .9, .5 ) + DayAmount*0.)*DayAmount;

	vec3 SkyColor = CalcSky();
	vec4 basecolor;
	if( Pass1Normal.a < .5 || Pass2Data.a <= 0. )
		basecolor = vec4( SkyColor, 1. );
	else
		basecolor = vec4( Pass2Data );


/* Crepscular rays (it sort of works) */

	//Find sky bright position in screen space. (then do crepscular rays)
	float aspect = ScreenX/ScreenY;
	vec4 ssposSB = toss*vec4( SkyBrightPos.rgb, 0. )*2.;
	ssposSB/=-ssposSB.z/2.; //normalize the p
	ssposSB = vec4( (-ssposSB.x)/aspect, ssposSB.yzw );
	vec2 rPosInTex = (vec2( 1.-PosInTex.x, 1.-PosInTex.y )*2.-1.);

	vec2 delp = ( ssposSB.rg - rPosInTex.rg );
	vec2 delts =  delp.rg;
	delts = vec2( -delts.x,delts.y );
	float addative = 0.0;
	vec2 tptr = vec2( PosInTex.x, 1.-PosInTex.y );
	const int c = 25; //Can be adjusted based on resolution
	const float a = .25/float(c);
	const float b = .15/float(c);
	for( int i = 0; i < c; i++ )
	{
		tptr += delts*a;
		vec4 Pass2d = texture2D( Pass2, tptr );
		if( Pass2d.a < 0.1 ) addative += b / length( delp );
	}


//	gl_FragColor = vec4(addative);
	gl_FragColor = basecolor;

//	gl_FragColor = vec4( delts,0., 1. );
//	gl_FragColor = basecolor + vec4(addative);

	return;

}



vec3 CalcSky()
{
	float dz = dir.z+.1;//(dir.z>0.1)?dir.z:0.1;
	vec3 clouds = vec3( 0.1, 0.12, .8 )*2.;
	if( dz > 0.02)
	{
		vec2 skypos = vec2( dir.x/dz, dir.y/dz )/20. + dir.xy*.1;
		skypos/=2.;
		skypos += vec2(time*.001);

		float noiser = pNoise3( vec3( skypos, time*.001 ) ) * 1. + pNoise2( skypos * 2. ) * .5 + pNoise2( skypos * 4. ) * .25;
		noiser = noiser*noiser;
		if( noiser < 1.0 )
			clouds = mix( vec3( 0.1,0.12,8. ), vec3( 1. ), noiser );
		else
			clouds = mix( vec3(1.) , vec3(0.5), noiser-1. );
	}

	//Sunsize controlled by middle two terms, last term controls sun glaze
	float sunqty = max( pow( (1.0 - length((dir - SunPos)))*1.1, 20.5 ), 0. );
	sunqty = (sunqty<1.)?(sunqty*.8):1.;
	vec3 suncolor = vec3( 1., .9, .2 ) * sunqty;


	clouds *= (1.05-sunqty );
	clouds *= DayAmount;

	clouds = clamp( clouds, vec3( 0. ), vec3( 1. ) );

	//a little yucky...  need to rotate view direction about day time for stars
//-cos( daytime ), 0., sin( daytime )
	vec3 StarOrientation = vec3( dir.z * sin(daytime) - dir.x * cos(daytime), dir.y, dir.z*cos(daytime) + dir.x*sin(daytime) );
	float StarNoise = pNoise3( vec3( StarOrientation )/10. )*1.0 + pNoise3( vec3( StarOrientation ) * ScreenY/32. )*5. -2.;
	StarNoise -= 1.2;

	StarNoise = max(0.,StarNoise);
	StarNoise *= NightAmount;

	StarNoise = clamp( StarNoise, 0., 2. );
	//Sunsize controlled by middle two terms, last term controls sun glaze
	float moonqty = max( pow( (1.00 - length((dir - MoonPos)))*1.15, 9.5 ), 0. );
	if(moonqty<1.)
		moonqty *= .5;
	else
	{
		vec2 moont = (dir.bg-MoonPos.bg)*5.;

		moont = tan( moont * 3.14159/2. )/6.;
		moonqty = pNoise2( moont )+pNoise2( moont*2. )*.5+pNoise2( moont*4. )*.25;
		moonqty = moonqty/4. + .9;
		StarNoise = 0.;
	}

	vec3 mooncolor = vec3( .8, .8, .8 ) * moonqty;

	vec3 NightSky = mooncolor + StarNoise;

	if( DayAmount > NightAmount )
	{
		SkyBrightPos = SunPos;
	}
	else
	{
		SkyBrightPos = MoonPos;
	}

	return clouds+suncolor+StarNoise + mooncolor;
//	return  noise;
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

