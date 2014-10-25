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
#ifdef IS_TCC_RUNTIME
#define true 1
#define false 0
#define v(x,y,z) x,y,z
#define vf(x,y,z) x,y,z
typedef unsigned char byte;
// WARNING: tcc does not throw errors when the signature of a function is not known!
float sinf(float x);
float cosf(float x);
double sin(double x);
double cos(double x);
double pow(double a, double b);
#else 
#include "Common.h"
#include <algorithm>

#define CLAMP( x, mi, ma ) (x < mi ? mi:( x>ma ? ma:x ) )

extern Vec3f gPosition;
extern Vec3f gDirection;
extern Vec3f gTargetNormal;
extern Vec3f gTargetCompression;
extern Vec3f gTargetHit;

extern int AddSizeStride;

void ClearCellV(Vec3i p);
void PaintRangeV(Vec3i p, Vec3i s, RGBA val);
void ClearRangeV(Vec3i p, Vec3i s);
void EmptyBoxV(Vec3i p, Vec3i s, bool force_empty, RGBA val);
void JumpSpaceV(Vec3i p, Vec3i s, Vec3f ofs, Vec3f f1 = {1,0,0}, Vec3f f2={0,1,0}, Vec3f f3 = {0,0,1});
void WarpSpaceV(Vec3i p, Vec3i s, Vec3f comp);
int PlayerInRangeV(Vec3f p, Vec3f s) ;
void ChangeCellV(int t, Vec3i p, RGBA c);
void QuickCellV(int t, Vec3i p, RGBA c);
void QuickCell1GBAOnlyV(Vec3i p, byte g, byte b, byte a);
void UpdateZoneV(Vec3i p, Vec3i s);
void MarkAddDataForReload();
bool fileChanged(string fname);

struct PickableBlock {
    Vec3i p;
    float phasing; //If +1 or 0, block is truly here.  If -1, phasing out.  Note: 0 = do not re-update.
    float density; //between 0 and 1.
    char in_use;
};

struct DeathBlock {
    Vec3i p;
    int in_use;
};


#endif

#define PICKABLE_CELL 114
#define DEFAULT_BRIGHT 190
#define DEFAULT_DENSITY 125
#define WALL_BLOCK 12
#define WHITE_BLOCK 19
#define GOAL_BLOCK 24
#define DEADGOAL_BLOCK 7
#define DEFAULT_EMPTY_BLOCK 0
#define SPAAACE_CELL 255

extern double worldDeltaTime;

extern float gDaytime;

extern float gRenderMixval;
extern float gRenderDensityLimit;
extern float gRenderDensityMux;
extern float gTargetActualDistance;
extern float gTargetProjDistance;
extern float gTargetPerceivedDistance;
extern char gDialog[1024];
extern byte gKeyMap[256];
extern byte gFocused;
extern int gMouseLastClickButton;
extern float gTimeSinceOnGround;
extern int pickables_in_inventory;
extern int gOverallUpdateNo;
extern double GameTimer;
extern double GameAttempt;

float swoovey(float f, float siny);

char loopingarrayaccess(char * map, int w, int h, int x, int y);

int PlayerInRange(float x, float y, float z, float x2, float y2, float z2);
void PaintRange(int x, int y, int z, int x2, int y2, int z2, byte block, byte density);
void JumpSpace(int x, int y, int z, int x2, int y2, int z2, float xofs, float yofs, float zofs);
void JumpSpaceExtended(int x, int y, int z, int x2, int y2, int z2, float xofs, float yofs, float zofs, float xm1, float ym1, float zm1, float xm2, float ym2, float zm2, float xm3, float ym3, float zm3);

void ClearPickableBlocks();
void PlacePickableAt(int x, int y, int z, float initDensity);
void ChangeCell(int x, int y, int z, byte block, byte density);
void ClearCell(int x, int y, int z);
void ClearRange(int x, int y, int z, int x2, int y2, int z2);
void EmptyBox(int x, int y, int z, int x2, int y2, int z2, byte block, byte density);
void WarpSpace(int x, int y, int z, int x2, int y2, int z2, float f1, float f2, float f3);
void QuickCell(int t, int x, int y, int z, byte block, byte density);
void AddDeathBlock(int x, int y, int z);
void UpdateZone(int x, int y, int z, int x2, int y2, int z2);
void die();
#endif