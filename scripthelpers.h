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

#include "Common.h"
#include <algorithm>
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

#define CLAMP( x, mi, ma ) (x < mi ? mi:( x>ma ? ma:x ) )

extern double worldDeltaTime;

extern float gDaytime;

extern float gRenderMixval;
extern float gRenderDensityLimit;
extern float gRenderDensityMux;

extern Vec3f gPosition;
extern Vec3f gDirection;
extern Vec3f gTargetNormal;
extern Vec3f gTargetCompression;
extern Vec3f gTargetHit;
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

extern int AddSizeStride;

char loopingarrayaccess(char * map, int w, int h, int x, int y);

//Make a triangle wave.
float swoovey(float f, float siny);

typedef void (*ClickCellCBType)(bool left, Vec3f pos, float dist);


void ClearCell(Vec3i p);
void PaintRange(Vec3i p, Vec3i s, RGBA val);
void ClearRange(Vec3i p, Vec3i s);
void MakeEmptyBox(Vec3i p, Vec3i s, bool force_empty, RGBA val);
void MakeJumpSection(Vec3i p, Vec3i s, Vec3f ofs, Vec3f f1 = {1,0,0}, Vec3f f2={0,1,0}, Vec3f f3 = {0,0,1});
void SetWarpSpaceArea(Vec3i p, Vec3i s, Vec3f comp);
int IsPlayerInRange(Vec3f p, Vec3f s) ;
void ChangeCell(int t, Vec3i p, RGBA c);
void QuickCell(int t, Vec3i p, RGBA c);
void QuickCell1GBAOnly(Vec3i p, byte g, byte b, byte a);
void UpdateZone(Vec3i p, Vec3i s);
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

// tcc funs
void tccCell(int x, int y, int z, byte block, byte density);
void tccClearCell(int x, int y, int z);
void tccClearRange(int x, int y, int z, int x2, int y2, int z2);
void tccEmptyBox(int x, int y, int z, int x2, int y2, int z2, byte block, byte density);
void tccWarpSpace(int x, int y, int z, int x2, int y2, int z2, float f1, float f2, float f3);
#endif