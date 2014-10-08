#ifndef _rthelper_h
#define _rthelper_h

#include "Common.h"
#include "Map.h"

//Executed after everything
void LoadProbes( bool isRerun );
void DoneProbes( bool isRerun );

void UpdatePositionAndRotation();

//This is actually a square. So this is the edge (8 -> 64 points)
#define PHYSICS_SIZE 16

class RTHelper
{
public:
	RTHelper( bool fakemode );
	~RTHelper();

	void DrawMap(double dTime, double Now);
	void LoadAttribMap();
	void SplitDrawSquare( float minx, float miny, float maxx, float maxy, float far = 10000 );

	Shader Pass1;
	Shader Pass1Physics;
	Shader Pass2;
	Shader Pass3;

	//Outputs:
	//1) 3D Position of texture intersection (relative to camera's int), a= [steps]
	//2) Normal of texture, a= [quality : 1 = good; 0 = sky; -1= too far.]
	Texture PassPhysicsOutputs[4];
	Texture Pass1Outputs[2];
	Texture Pass2Output;
	RFBuffer PassPhysicsRFB;
	RFBuffer Pass1RFB;
	RFBuffer Pass2RFB;
	Texture NoiseMap;

	Texture AdditionalInformationMap; //For jumping, etc.
	RGBAf * AdditionalInformationMapData;
	int     AdditionalInformationPointer;
	int     AllocAddInfo( int size ); //Request # of 4-float pairs.
	void	FreeAddInfo( int ptr ) { /*Stubbed, do not call AllocAddInfo() too much!*/ }
	void	ReloadAdditionalInformatioMapData();
	void	MarkAddInfoForReload() { bAddInfoForceReload = true; }

	Texture AttributeMap;
	void LoadAttributeMap();
	unsigned long LastAttributeTime;

	Map * TMap;

	//Attributes
	RGBAf * LTTex;
	float MapOffsetX;
	float MapOffsetY;
	float MapOffsetZ;

	float vX, vY, vZ; //velocities of player

	float LastPass1Time;
	float LastPass2Time;
	float LastPass3Time;

	int lastWidth;
	int lastHeight;

	//Probes (They live in an array, you don't have to worry about free'ing or deleting them)
	CollisionProbe * AddProbe() { if( ProbePlace < PHYSICS_SIZE*PHYSICS_SIZE ) return &Probes[ProbePlace++]; else return 0; }
	CollisionProbe Probes[PHYSICS_SIZE*PHYSICS_SIZE];
	unsigned ProbePlace;
	void ExecProbes( bool bRerun);
	void ForceProbeReRun() { bProbeReRun = true; }
	bool bProbeReRun;

	bool bAddInfoForceReload;
};

#endif

