/*
	Copyright (c) 2008-2014 <>< Charles Lohr
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

#include "OGLParts.h"
#include "GLUTCore.h"
#include "RTHelper.h"
#include <sys/stat.h>
#include <sys/time.h>
#include <math.h>

float gDaytime;
float gRenderMixval = .9;
float gRenderDensityLimit = .2;
float gRenderDensityMux = 1.0;

#define NOISESIZE 16
#define PHYSICS_PIXEL( x )  float((x%PHYSICS_SIZE)+0.1)/PHYSICS_SIZE, float((PHYSICS_SIZE-(x/PHYSICS_SIZE)) - 0.1)/PHYSICS_SIZE

void RGBAf::FromStringArray( string * arr )
{
	r = atof( arr[0].c_str() );
	g = atof( arr[1].c_str() );
	b = atof( arr[2].c_str() );
	a = atof( arr[3].c_str() );
}


RTHelper::RTHelper( bool fakemode ) : vX( 0 ), vY( 0 ), vZ( 0 ), ProbePlace(0)
{
	printf( "Populate Complete.\n" );

	lastWidth = 0;
	lastHeight = 0;

	float rNoise[NOISESIZE*NOISESIZE*4];
	for( unsigned i = 0; i < NOISESIZE*NOISESIZE*4; i++ )
		rNoise[i] = (rand()%16384)/16384.0f;
	NoiseMap.LoadTexture( (char*)rNoise, NOISESIZE, NOISESIZE, TTRGBA16 );

	printf( "Textures Loaded.\n" );

	AdditionalInformationPointer = 0;
	AdditionalInformationMapData = new RGBAf[ADDSIZEX * ADDSIZEY];
	ReloadAdditionalInformatioMapData();

	printf( "Loaded addinfo pointers.\n");
	AllocAddInfo( ADDSIZEX-1 ); //First one is a no-change.

	printf( "Allocated Addinfo.\n" );
	Pass1RFB.Setup();
	PassPhysicsRFB.Setup();
	Pass2RFB.Setup();

	printf( "RB Setup. Loading Shaders.\n" );

	Pass1Physics.LoadShader( "Shaders/Pass1Physics" );
	Pass1.LoadShader( "Shaders/Pass1" );
	Pass2.LoadShader( "Shaders/Pass2" );
	Pass3.LoadShader( "Shaders/Pass3" );

	printf( "Shaders loaded.\n" );
	PassPhysicsOutputs[0].MakeDynamicTexture( PHYSICS_SIZE, PHYSICS_SIZE, TTRGBA32 );
	PassPhysicsOutputs[1].MakeDynamicTexture( PHYSICS_SIZE, PHYSICS_SIZE, TTRGBA32 );
	PassPhysicsOutputs[2].MakeDynamicTexture( PHYSICS_SIZE, PHYSICS_SIZE, TTRGBA32 );
	PassPhysicsOutputs[3].MakeDynamicTexture( PHYSICS_SIZE, PHYSICS_SIZE, TTRGBA32 );
	printf( "Loading attribute map.\n" );
	LTTex = new RGBAf[256*8*16];
	LoadAttributeMap();

	printf( "Done loading attributes. Making map.\n ");
	TMap = new Map( "test.dat", this, fakemode );
	printf( "Done loading map.\n" );
}

RTHelper::~RTHelper()
{
	delete TMap;
	delete LTTex;
}

int RTHelper::AllocAddInfo( int size )
{
	int ret = 0;
	int x = AdditionalInformationPointer%ADDSIZEX;
	int y = AdditionalInformationPointer/ADDSIZEX;
	if( x + size >= ADDSIZEX )
	{
		x = 0;
		y++;
	}
	ret = x + y * ADDSIZEX;
	x += size;
	AdditionalInformationPointer = x + y * ADDSIZEX;

	if( size > 0 )
		AdditionalInformationMapData[ret+0] = RGBAf( 1, 0, 0, 0 );
	if( size > 1 )
		AdditionalInformationMapData[ret+1] = RGBAf( 0, 1, 0, 0 );
	if( size > 2 )
		AdditionalInformationMapData[ret+2] = RGBAf( 0, 0, 1, 0 );
	if( size > 3 )
		AdditionalInformationMapData[ret+3] = RGBAf( 0, 0, 0, 0 );
	if( size > 4 )
	{
		memset( &AdditionalInformationMapData[ret+4], 0, sizeof( RGBAf ) * (size - 4) );
	}
	return ret;
}



void RTHelper::ReloadAdditionalInformatioMapData()
{
	AdditionalInformationMap.LoadTexture( (char*)AdditionalInformationMapData, ADDSIZEX, ADDSIZEY, TTRGBA16, false ); //For jumping, etc.
	glFinish();
	glFlush();
}

void RTHelper::LoadAttributeMap()
{
	struct stat sb;
	stat( "tileattributes.txt", &sb );
	LastAttributeTime = sb.st_mtime;


	FILE * f = fopen( "tileattributes.txt", "r" );
	if( !f )
	{
		fprintf( stderr, "Error opening tileattributes.txt\n" );
		return;
	}

	printf( "Loading tileattributes.txt\n" );
	for( unsigned iLine = 0; iLine < 256; iLine++ ) //actually cell type
	{
		//Default (set everything up)

		for( unsigned iTile = 0; iTile < 16; iTile++ ) //actually meta's.
		{
			LTTex[iLine * 128 + iTile * 8 + 0].r = 1.0;//float(iLine%16)/16.;
			LTTex[iLine * 128 + iTile * 8 + 0].g = 1.0;//float(iLine/16)/16.;
			LTTex[iLine * 128 + iTile * 8 + 0].b = 1.0;//float(iTile)/16.;
			LTTex[iLine * 128 + iTile * 8 + 0].a = 1.0;

			LTTex[iLine * 128 + iTile * 8 + 1].r = 1.0;
			LTTex[iLine * 128 + iTile * 8 + 1].g = 1.0;
			LTTex[iLine * 128 + iTile * 8 + 1].b = 1.0;
			LTTex[iLine * 128 + iTile * 8 + 1].a = 1.0;

			LTTex[iLine * 128 + iTile * 8 + 2].r = 0.0;
			LTTex[iLine * 128 + iTile * 8 + 2].g = 0.0;
			LTTex[iLine * 128 + iTile * 8 + 2].b = 0.0;
			LTTex[iLine * 128 + iTile * 8 + 2].a = 0.0;

			LTTex[iLine * 128 + iTile * 8 + 3].r = 0.0;
			LTTex[iLine * 128 + iTile * 8 + 3].g = 0.0;
			LTTex[iLine * 128 + iTile * 8 + 3].b = 0.0;
			LTTex[iLine * 128 + iTile * 8 + 3].a = 0.0;

			LTTex[iLine * 128 + iTile * 8 + 4].r = 1.0;
			LTTex[iLine * 128 + iTile * 8 + 4].g = 1.0;
			LTTex[iLine * 128 + iTile * 8 + 4].b = 0.1;
			LTTex[iLine * 128 + iTile * 8 + 4].a = 1.0;

			LTTex[iLine * 128 + iTile * 8 + 7].r = 1.0;
			LTTex[iLine * 128 + iTile * 8 + 7].g = 1.0;
			LTTex[iLine * 128 + iTile * 8 + 7].b = 0.0;
			LTTex[iLine * 128 + iTile * 8 + 7].a = 1.0;
		}
	}


	char tline[1024];
	int lineNo = 0;
	while( !feof( f ) && fgets( tline, 1023, f ) )
	{
		lineNo++;
		string Description;
		int iTileID = -1;
		int iMetaID = -1;
		if( tline[0] == '#' ) continue;

		RGBAf BaseColor;
		RGBAf NoiseColor;
		RGBAf NoiseSet;
		RGBAf NoiseMux;
		RGBAf CoreData;
		RGBAf TimeSettings;
		RGBAf Speckles;
		RGBAf ShaderEndingTerms( 1, 1, 0, 0);
		float fdensity; 

		vector< string > dats;
		string stmp;
		char c = 0;
		for( int ipl = 0; (c = tline[ipl]); ipl++ )
		{
			if( c == ' ' || c == '\t' )
			{
				if( stmp.size() )
					dats.push_back( stmp );
				stmp = "";
			}
			else if( c == 10 );
			else
			{
				stmp += c;
			}
		}
		if (stmp != "")
			dats.push_back( stmp );
		if (dats.size() == 0) // Empty line.
			continue;
		if( dats.size() < 28 )
		{
			printf( "Malformatted line in TileAttributes.txt, line %d: (%lu) args.\n", lineNo, dats.size() );
			continue;
		}

		Description = dats[0];
		iTileID = atoi( dats[1].c_str() );
		iMetaID = atoi( dats[2].c_str() );
		fdensity = atof( dats[3].c_str() );

		BaseColor.FromStringArray( &dats[4] );
		NoiseColor.FromStringArray( &dats[8] );
		NoiseSet.FromStringArray( &dats[12] );
		NoiseMux.FromStringArray( &dats[16] );
		CoreData.FromStringArray( &dats[20] );
		TimeSettings.FromStringArray( &dats[24] );
		if( dats.size() >= 32 )
			Speckles.FromStringArray( &dats[28] );
		if( dats.size() >= 36 )
			ShaderEndingTerms.FromStringArray( &dats[32] );
	
		ShaderEndingTerms.a = fdensity; //Overwrite the last term.

//		printf( "Loading: %d / %d (%s) ... %f %f %f %f / SPEC: %f %f %f %f\n", iTileID, iMetaID, Description.c_str(),
//			BaseColor.r, BaseColor.g, BaseColor.b, BaseColor.a,
//			Speckles.r, Speckles.g, Speckles.b, Speckles.a );

		if( iTileID == -1 ) continue;

		if( iMetaID == -1 )
		{
			for( iMetaID = 0; iMetaID < 16; iMetaID ++ )
			{
				LTTex[iTileID * 128 + iMetaID * 8 + 0] = BaseColor;
				LTTex[iTileID * 128 + iMetaID * 8 + 1] = NoiseColor;
				LTTex[iTileID * 128 + iMetaID * 8 + 2] = NoiseSet;
				LTTex[iTileID * 128 + iMetaID * 8 + 3] = NoiseMux;
				LTTex[iTileID * 128 + iMetaID * 8 + 4] = CoreData;
				LTTex[iTileID * 128 + iMetaID * 8 + 5] = TimeSettings;
				LTTex[iTileID * 128 + iMetaID * 8 + 6] = Speckles;
				LTTex[iTileID * 128 + iMetaID * 8 + 7] = ShaderEndingTerms;
			}
		}
		else
		{
			LTTex[iTileID * 128 + iMetaID * 8 + 0] = BaseColor;
			LTTex[iTileID * 128 + iMetaID * 8 + 1] = NoiseColor;
			LTTex[iTileID * 128 + iMetaID * 8 + 2] = NoiseSet;
			LTTex[iTileID * 128 + iMetaID * 8 + 3] = NoiseMux;
			LTTex[iTileID * 128 + iMetaID * 8 + 4] = CoreData;
			LTTex[iTileID * 128 + iMetaID * 8 + 5] = TimeSettings;
			LTTex[iTileID * 128 + iMetaID * 8 + 6] = Speckles;
			LTTex[iTileID * 128 + iMetaID * 8 + 7] = ShaderEndingTerms;
		}
//		LTTex[iTile * 256 + iMetaID * 16 + 3] = ;
	}

	printf( "Tile attributes loaded.\n" );

//	for( unsigned i = 0; i < 512; i++ )
//	{
//		printf( "%f %f %f %f\n", LTTex[i].r, LTTex[i].g, LTTex[i].b, LTTex[i].a );
//	}
	AttributeMap.LoadTexture( (char*)LTTex, 128, 256, TTRGBA32, false );
	glFinish();
	glFlush();
	fclose( f );
	printf( "Done with tile attributes.\n" );

}

void RTHelper::ExecProbes( bool isRerun )
{

	LoadProbes( isRerun );

	glBegin(GL_POINTS);

	for( unsigned i = 0; i < ProbePlace; i++ )
	{
		CollisionProbe & p = Probes[i];
		glMultiTexCoord4f( 1, p.AuxRotation.r, p.AuxRotation.g, p.AuxRotation.b, 0 );
		glNormal3f( p.Direction.r, p.Direction.g, p.Direction.b );				//Direction
		glColor4f( p.Position.r, p.Position.g, p.Position.b, -1 );	//Position (With override)
		glTexCoord2f( PHYSICS_PIXEL(i) );
		glVertex3f( 0, 0, p.Direction.a);				//Maxlen specified (vertex.z)
	}

	glEnd();

	float PhysicsData[PHYSICS_SIZE*PHYSICS_SIZE*4];
	float PhysicsData2[PHYSICS_SIZE*PHYSICS_SIZE*4];
	float PhysicsData3[PHYSICS_SIZE*PHYSICS_SIZE*4];
	float PhysicsData4[PHYSICS_SIZE*PHYSICS_SIZE*4];

	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
	StripDataFromBuffer( 0, 0, PHYSICS_SIZE, PHYSICS_SIZE, TTRGBA32, (char*)PhysicsData );
	glReadBuffer(GL_COLOR_ATTACHMENT1_EXT);
	StripDataFromBuffer( 0, 0, PHYSICS_SIZE, PHYSICS_SIZE, TTRGBA32, (char*)PhysicsData2 );
	glReadBuffer(GL_COLOR_ATTACHMENT2_EXT);
	StripDataFromBuffer( 0, 0, PHYSICS_SIZE, PHYSICS_SIZE, TTRGBA32, (char*)PhysicsData3 );
	glReadBuffer(GL_COLOR_ATTACHMENT3_EXT);
	StripDataFromBuffer( 0, 0, PHYSICS_SIZE, PHYSICS_SIZE, TTRGBA32, (char*)PhysicsData4 );
	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);

	for( unsigned i = 0; i < ProbePlace; i++ )
	{
		CollisionProbe & p = Probes[i];
		p.NewDirection = RGBAf( PhysicsData4[i*4+0], PhysicsData4[i*4+1], PhysicsData4[i*4+2], PhysicsData4[i*4+3] );
		p.Normal = 		RGBAf( PhysicsData[i*4+0], PhysicsData[i*4+1], PhysicsData[i*4+2], PhysicsData[i*4+3] );
		p.InAreaWarp = 	RGBAf( PhysicsData2[i*4+0], PhysicsData2[i*4+1], PhysicsData2[i*4+2], PhysicsData2[i*4+3] );
		p.TargetLocation = 	RGBAf( PhysicsData3[i*4+0], PhysicsData3[i*4+1], PhysicsData3[i*4+2], PhysicsData3[i*4+3] );
	}

	DoneProbes( isRerun );
	ProbePlace = 0;
}

void RTHelper::DrawMap(double dTime, double fTotalTime)
{
	float aspect = (float)GLUT.miWidth / (float)GLUT.miHeight;

	Pass1.CheckForNewer( "Shaders/Pass1" );
	Pass1Physics.CheckForNewer( "Shaders/Pass1Physics" );
	Pass2.CheckForNewer( "Shaders/Pass2" );
	Pass3.CheckForNewer( "Shaders/Pass3" );

	struct stat sb;
	::stat( "tileattributes.txt", &sb );
	if( LastAttributeTime != sb.st_mtime )
		LoadAttributeMap();


	if( bAddInfoForceReload )
	{
		bAddInfoForceReload = false;
		ReloadAdditionalInformatioMapData();
	}

	if( GLUT.miWidth != lastWidth || GLUT.miHeight != lastHeight )
	{
		Pass1Outputs[0].MakeDynamicTexture( GLUT.miWidth, GLUT.miHeight, TTRGBA32 );
		Pass1Outputs[1].MakeDynamicTexture( GLUT.miWidth, GLUT.miHeight, TTRGBA32 );
		Pass2Output.MakeDynamicTexture( GLUT.miWidth, GLUT.miHeight, TTRGBA32 );
		lastWidth = GLUT.miWidth;
		lastHeight = GLUT.miHeight;
	}
	glEnable( GL_TEXTURE_3D );

	TMap->Draw();
	
	glPushMatrix();
	glScalef( 1., 1., -1. );
	glTranslatef( -MapOffsetX, -MapOffsetY, -MapOffsetZ );

	timeval tva, tvb;
	gettimeofday( &tva, 0 );

	AttributeMap.ActivateTexture( 4 );
	AdditionalInformationMap.ActivateTexture( 3 );
	vector< string > vsAllSamplerLocs;
	vsAllSamplerLocs.push_back( "GeoTex" );
	vsAllSamplerLocs.push_back( "AddTex" );
	vsAllSamplerLocs.push_back( "MovTex" );
	vsAllSamplerLocs.push_back( "AdditionalInformationMap" );
	vsAllSamplerLocs.push_back( "AttribMap" );
	vsAllSamplerLocs.push_back( "Pass1A" );
	vsAllSamplerLocs.push_back( "Pass1B" );
	vsAllSamplerLocs.push_back( "Pass2" );


	vector< string > vsAllFloats;
	vector< float  > vfAllFloats;
	vsAllFloats.push_back( "msX" ); vfAllFloats.push_back( GLH_SIZEX );
	vsAllFloats.push_back( "msY" ); vfAllFloats.push_back( GLH_SIZEY );
	vsAllFloats.push_back( "msZ" ); vfAllFloats.push_back( GLH_SIZEZ );
	vsAllFloats.push_back( "msX" ); vfAllFloats.push_back( GLH_SIZEX );
	vsAllFloats.push_back( "ScreenX" ); vfAllFloats.push_back( GLUT.miWidth );
	vsAllFloats.push_back( "ScreenY" ); vfAllFloats.push_back( GLUT.miHeight );
	vsAllFloats.push_back( "time" ); vfAllFloats.push_back( gDaytime );
	vsAllFloats.push_back( "do_subtrace" ); vfAllFloats.push_back( 1 );
	vsAllFloats.push_back( "mixval" ); vfAllFloats.push_back( gRenderMixval );
	vsAllFloats.push_back( "densitylimit" ); vfAllFloats.push_back( gRenderDensityLimit );
	vsAllFloats.push_back( "densitymux" ); vfAllFloats.push_back( gRenderDensityMux );

	//Pre-pass: Physics
	PassPhysicsRFB.ConfigureAndStart( PHYSICS_SIZE, PHYSICS_SIZE, 4, &PassPhysicsOutputs[0], true );
	Pass1Physics.ActivateShader( vsAllSamplerLocs, vsAllFloats, vfAllFloats );
	bProbeReRun = false;
	do
	{
		bool bIsRerun = bProbeReRun;
		bProbeReRun = false;
		ExecProbes( bIsRerun );
	} while( bProbeReRun );
	Pass1Physics.DeactivateShader();
	PassPhysicsRFB.End( lastWidth, lastHeight );

	//Sometimes the dX/dY are changed after this... reload them.
	glPopMatrix();
	glPushMatrix();

	//Re-load the matrix
	UpdatePositionAndRotation();

	glScalef( 1., 1., -1. );
	glTranslatef( -MapOffsetX, -MapOffsetY, -MapOffsetZ );
	glColor4f( 1., 1., 1., 1. );

	//Pass 1: Core ray tracer
	Pass1RFB.ConfigureAndStart( lastWidth, lastHeight, 2, &Pass1Outputs[0], true );
	Pass1.ActivateShader( vsAllSamplerLocs, vsAllFloats, vfAllFloats );
	SplitDrawSquare( -aspect, -1, aspect, 1 );
	Pass1.DeactivateShader();
	Pass1RFB.End( lastWidth, lastHeight );

	Pass1Outputs[0].ActivateTexture( 5 );
	Pass1Outputs[1].ActivateTexture( 6 );

	glFinish();
	glFlush();
	gettimeofday( &tvb, 0 );
	LastPass1Time = ( tvb.tv_sec - tva.tv_sec ) * 1000000 + (tvb.tv_usec - tva.tv_usec);

	//Pass 2: Map Shading
	//Don't need the spheres anymore.
	vsAllSamplerLocs[3] = "NoiseMap";
	NoiseMap.ActivateTexture( 3 );

	gettimeofday( &tva, 0 );

	glClearColor( 0., 0., 0., 0. );
	Pass2RFB.ConfigureAndStart( lastWidth, lastHeight, 1, &Pass2Output, true );
	Pass2.ActivateShader( vsAllSamplerLocs, vsAllFloats, vfAllFloats );
	SplitDrawSquare( -aspect, -1, aspect, 1 );
	Pass2.DeactivateShader();
	Pass2RFB.End( lastWidth, lastHeight );

	glFinish();
	glFlush();
	gettimeofday( &tvb, 0 );
	LastPass2Time = ( tvb.tv_sec - tva.tv_sec ) * 1000000 + (tvb.tv_usec - tva.tv_usec);

	gettimeofday( &tva, 0 );

	//Pass 3: Final pass
	Pass2Output.ActivateTexture( 7 );

	Pass3.ActivateShader( vsAllSamplerLocs, vsAllFloats, vfAllFloats );
	SplitDrawSquare( -aspect, -1, aspect, 1 );
	Pass3.DeactivateShader();

	Pass2Output.DeactivateTexture( 7 );
	Pass1Outputs[0].DeactivateTexture( 5 );
	Pass1Outputs[1].DeactivateTexture( 6 );
	NoiseMap.DeactivateTexture( 3 );
	AttributeMap.DeactivateTexture( 4 );

	glActiveTextureARB( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_3D, 0 );
	glActiveTextureARB( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_3D, 0 );
	glDisable( GL_TEXTURE_3D );

	glPopMatrix();

	glFinish();
	glFlush();
	gettimeofday( &tvb, 0 );
	LastPass3Time = ( tvb.tv_sec - tva.tv_sec ) * 1000000 + (tvb.tv_usec - tva.tv_usec);


}


//The original intent of this function was to allow for splitting apart of workloads
//turns out NVIDIA drivers won't let us do that, so back to the original we go....
void RTHelper::SplitDrawSquare( float minx, float miny, float maxx, float maxy, float fark )
{
	glBegin( GL_QUADS );
	glTexCoord2f( 0.0f, 0.0f );
	glVertex3f( minx, miny, fark );
	glTexCoord2f( 0.0f, 1.0f );
	glVertex3f( minx, maxy, fark );
	glTexCoord2f( 1.0f, 1.0f );
	glVertex3f( maxx, maxy, fark );
	glTexCoord2f( 1.0f, 0.0f );
	glVertex3f( maxx, miny, fark );
	glEnd();

	return;

#define mix( x, y, a )   ((x)*(a)+(y)*(1.0-a))
#define NUM_SLICESX 8
#define NUM_SLICESY 8

	for( int y = 0; y < NUM_SLICESY; y++ )
	for( int x = 0; x < NUM_SLICESX; x++ )
	{
		float flpercentx = ((float)x)/((float)NUM_SLICESX);
		float fhpercentx = ((float)x+1)/((float)NUM_SLICESX);
		float flpercenty = ((float)y)/((float)NUM_SLICESY);
		float fhpercenty = ((float)y+1)/((float)NUM_SLICESY);
		glBegin( GL_QUADS );
		glTexCoord2f( mix( 0, 1, flpercentx ), mix( 0, 1, flpercenty ) );
		glVertex3f( mix( minx, maxx, flpercentx), mix( miny, maxy, flpercenty ), fark );
		glTexCoord2f( mix( 0, 1, flpercentx ), mix( 0, 1, fhpercenty ) );
		glVertex3f( mix( minx, maxx, flpercentx), mix( miny, maxy, fhpercenty ), fark );
		glTexCoord2f( mix( 0, 1, fhpercentx ), mix( 0, 1, fhpercenty ) );
		glVertex3f( mix( minx, maxx, fhpercentx), mix( miny, maxy, fhpercenty ), fark );
		glTexCoord2f( mix( 0, 1, fhpercentx ), mix( 0, 1, flpercenty ) );
		glVertex3f( mix( minx, maxx, fhpercentx), mix( miny, maxy, flpercenty ), fark );
		glEnd();
		glFinish();
		glFlush();
		glutSwapBuffers();
	}

}

