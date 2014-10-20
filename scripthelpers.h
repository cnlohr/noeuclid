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

//Read an array with looping ends

void ChangeCell(int t, Vec3i p, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void QuickCell(int t, Vec3i p, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void QuickCell1GBAOnly(Vec3i p, unsigned char g, unsigned char b, unsigned char a);
void UpdateZone(Vec3i p, Vec3i s);
int AllocAddInfo(int nradds);
void AlterAddInfo(int pos, Vec3f f, float a);
void AlterAddInfo(int pos, float x, float y, float z, float a);
void MarkAddDataForReload();
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
extern unsigned char gKeyMap[256];
extern unsigned char gFocused;
extern int gMouseLastClickButton;
extern float gTimeSinceOnGround;

extern int gOverallUpdateNo;

extern double GameTimer;
extern double GameAttempt;

extern int AddSizeStride;

char loopingarrayaccess(char * map, int w, int h, int x, int y) {
    x = ((x % w) + w) % w;
    y = ((y % h) + h) % h;
    return map[x + y * w];
}

//Make a triangle wave.

float swoovey(float f, float siny) {
    if (f < 0) f *= -1;
    f = fmod(f, 1.0);
    float fs = sin(f * 3.14159 * 2);
    //	if( f > .5 ) f = 1 - f;
    //	f = f * 4.0 - 1.0;
    //	if( f < 0 ) f = -pow( -f, siny );
    //	else f = pow( f, siny );
    //	return f;

    if (fs < 0) fs = -pow(-fs, siny);
    else fs = pow(fs, siny);
    return fs;
}

typedef void (*ClickCellCBType)(bool left, Vec3f pos, float dist);
ClickCellCBType ClickCellCB;

//File for useful scripts.

//Defines outside extents.

void ClearCell(Vec3i p) {
    ChangeCell(0, p, 0, DEFAULT_BRIGHT, 0, DEFAULT_EMPTY_BLOCK);
}

void PaintRange(Vec3i p, Vec3i s, int cell, int density, int unclear = 1) {
    int i, j, k;
    for (i = p.x; i < p.x + s.x; i++)
        for (j = p.y; j < p.y + s.y; j++)
            for (k = p.z; k < p.z + s.z; k++) {
                QuickCell(0, {i, j, k}, unclear, DEFAULT_BRIGHT, density, cell);
            }
    UpdateZone(p, s + Vec3i{1,1,1});
}

void ClearRange(Vec3i p, Vec3i s) {
    PaintRange(p, s, DEFAULT_EMPTY_BLOCK, 0, 0);
}

void MakeEmptyBox(Vec3i p, Vec3i s, short cell, short defden, short bright, int force_empty) {
    //Will cause an update over our whole area.
    if (force_empty)
        ClearRange(p, s);
    else
        UpdateZone(p, s + Vec3i{1,1,1});


    short j, k;
    for (j = p.x; j <= p.x + s.x; j++)
        for (k = p.y; k <= p.y + s.y; k++) {
            QuickCell(0, {j, k, p.z}, 1, bright, defden, cell);
            QuickCell(0, {j, k, p.z + s.z}, 1, bright, defden, cell);
        }

    for (j = p.z; j <= p.z + s.z; j++)
        for (k = p.x; k <= p.x + s.x; k++) {
            QuickCell(0, {k, p.y, j}, 1, bright, defden, cell);
            QuickCell(0, {k, p.y + s.y, j}, 1, bright, defden, cell);
        }

    for (j = p.z; j <= p.z + s.z; j++)
        for (k = p.y; k <= p.y + s.y; k++) {
            QuickCell(0, {p.x, k, j}, 1, bright, defden, cell);
            QuickCell(0, {p.x + s.x, k, j}, 1, bright, defden, cell);
        }

}

void MakeJumpSection(Vec3i p, Vec3i s, Vec3f ofs, Vec3f f1 = {1,0,0}, Vec3f f2={0,1,0}, Vec3f f3 = {0,0,1}) {
    int newalloc = AllocAddInfo(4);
    
    AlterAddInfo(newalloc + 0, f1, 0);
    AlterAddInfo(newalloc + 1, f2, 0);
    AlterAddInfo(newalloc + 2, f3, 0);
    AlterAddInfo(newalloc + 3, ofs, 0);

    short i, j, k;
    for (i = p.x; i <= p.x + s.x; i++)
        for (j = p.y; j <= p.y + s.y; j++)
            for (k = p.z; k <= p.z + s.z; k++) {
                QuickCell1GBAOnly({i, j, k}, 0, newalloc % AddSizeStride, newalloc / AddSizeStride);
            }
    UpdateZone(p, s + Vec3i{1,1,1});
    MarkAddDataForReload();
}

void SetWarpSpaceArea(Vec3i p, Vec3i s, Vec3f comp) {
    //0..255, 0..255, 0..255, 0..255
    //Bit compression = xyz/w

    float maxcomp = std::max(comp.x,std::max(comp.y,comp.z));
    if (maxcomp < 1.0) maxcomp = 1.0;
    Vec3f cc = comp; cc/=maxcomp;
    float wterm = 1. / maxcomp;
    cc.x = CLAMP(cc.x, 0, 1);
    cc.y = CLAMP(cc.y, 0, 1);
    cc.z = CLAMP(cc.z, 0, 1);
    wterm = CLAMP(wterm, 0, 1);

    unsigned char xt = cc.x * 255;
    unsigned char yt = cc.y * 255;
    unsigned char zt = cc.z * 255;
    unsigned char wt = wterm * 255;

    //	float xyzpres = sqrt( xcomp*xcomp + ycomp*ycomp + zcomp*zcomp );

    short i, j, k;
    for (i = p.x; i <= p.x + s.x; i++)
        for (j = p.y; j <= p.y + s.y; j++)
            for (k = p.z; k <= p.z + s.z; k++) {
                QuickCell(2, {i, j, k}, xt, yt, zt, wt);
            }
    UpdateZone(p, s + Vec3i{1,1,1});
}

int IsPlayerInRange(Vec3f p, Vec3f s) {
    if (gPosition.x >= p.x && gPosition.x <= p.x + s.x &&
            gPosition.y >= p.y && gPosition.y<= p.y + s.y &&
            gPosition.z >= p.z && gPosition.z <= p.z + s.z) return 1;
    return 0;
}


#define MAX_PICKABLES 256
extern int pickables_in_inventory;
//pickable block helper.

struct PickableBlock {
    Vec3i p;
    float phasing; //If +1 or 0, block is truly here.  If -1, phasing out.  Note: 0 = do not re-update.
    float density; //between 0 and 1.
    char in_use;
} PBlocks[MAX_PICKABLES];

//Returns id if pickable is there.
//Returns -1 if no block.
//Returns -2 if block tween incomplete.

int GetPickableAt(Vec3i p) {
    int i;
    for (i = 0; i < MAX_PICKABLES; i++) {
        struct PickableBlock * pb = &PBlocks[i];
        if(pb->in_use) printf("%d %d %d\n",pb->p.x,pb->p.y,pb->p.z);
        if (pb->in_use && pb->p == p) {
            return (pb->density > .99) ? i : -2;
        }
    }
    return -1;
}

void DissolvePickable(int pid) {
    struct PickableBlock * pb = &PBlocks[pid];
    pb->phasing = -1;
}

//If -1, no pickables left.
//If -2, Pickable already there.
//Otherwise returns which pickable!
//initial_density should be 0 unless you want to shorten (+) or lengthen (-) tween.

int PlacePickableAt(Vec3i p, float initial_density) {
    int i;

    for (i = 0; i < MAX_PICKABLES; i++) {
        struct PickableBlock * pb = &PBlocks[i];
        if (!pb->in_use) {
            break;
        }
    }
    if (i == MAX_PICKABLES)
        return -2;

    struct PickableBlock * pb = &PBlocks[i];
    pb->p = p;
    pb->phasing = 1;
    pb->in_use = 1;
    pb->density = initial_density;
    return 0;
}

void ClearPicableBlocks() {
    int i;
    for (i = 0; i < MAX_PICKABLES; i++) {
        struct PickableBlock * pb = &PBlocks[i];
        pb->p = Vec3i{0,0,0};
        pb->phasing = 0;
        pb->density = 0;
        pb->in_use = 0;
    }
    pickables_in_inventory = 0;
}

//Redraw 

void UpdatePickableBlocks() {
    int i;
    for (i = 0; i < MAX_PICKABLES; i++) {
        struct PickableBlock * pb = &PBlocks[i];

        if (!pb->in_use) continue;

        pb->density += worldDeltaTime * pb->phasing;

        if (pb->density >= 1.0 && pb->phasing > 0) {
            pb->density = 1.0;
            pb->phasing = 0;
        }
        if (pb->density <= 0 && pb->phasing < 0) {
            pb->density = 0.0;
            pb->phasing = 0.0;
            pb->in_use = 0;
        }
        float drawden = (pb->density < 0) ? 0.0 : ((pb->density > 1.0) ? 1.0 : pb->density);

        if (drawden > .01) {
            PaintRange(pb->p, {1, 1, 1}, PICKABLE_CELL, drawden * 200);
        } else {
            ClearCell(pb->p);
        }

    }
}

void PickableClick(bool left, Vec3f p, float dist) {
    //TODO calculate dist some other way (through portals)
    //if (dist > 4) return;
    if (left) {
        if (pickables_in_inventory > 0) {
            PlacePickableAt({(int)p.x,(int)p.y,(int)p.z}, 0.0);
            pickables_in_inventory--;
        } else {
            printf("No blocks.\n");
        }
    } else {
        int r = GetPickableAt({(int)p.x,(int)p.y,(int)p.z});

        if (r >= 0) {
            pickables_in_inventory++;
            PBlocks[r].phasing = -1;
        } else {
            printf("No pickable at (%d,%d,%d)",(int)p.x,(int)p.y,(int)p.z);
        }

    }
}


//Define all lava/death blocks.
//If the user touches one, he dies.
#define MAX_DEATH_BLOCKS 8192

struct DeathBlock {
    Vec3i p;
    int in_use;
} DeathBlocks[MAX_DEATH_BLOCKS];

void AddDeathBlock(Vec3i p) {
    int i;

    //Don't add multiple.
    for (i = 0; i < MAX_DEATH_BLOCKS; i++) {
        struct DeathBlock * db = &DeathBlocks[i];
        if (db->p == p) {
            return;
        }
    }

    for (i = 0; i < MAX_DEATH_BLOCKS; i++) {
        struct DeathBlock * db = &DeathBlocks[i];
        if (db->in_use == 0) {
            db->in_use = 1;
            db->p = p;
            break;
        }
    }
}
#include "RTHelper.h"
extern RTHelper * gh;

int IsOnDeathBlock(Vec3i p) {
    int i;
    for (i = 0; i < MAX_DEATH_BLOCKS; i++) {
        struct DeathBlock * db = &DeathBlocks[i];
        if (db->p.x == p.x && db->p.y == p.y && (db->p.z == p.z || db->p.z == p.z - 1)) {
            return 1;
        }
    }
    return 0;
}

void ChangeCell(int t, Vec3i p, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    QuickCell(t,p,r,g,b,a);
    gh->TMap->TackChange(p);
}

void QuickCell(int t, Vec3i p, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    gh->TMap->TexCell(t, p).r = r;
    gh->TMap->TexCell(t, p).g = g;
    gh->TMap->TexCell(t, p).b = b;
    gh->TMap->TexCell(t, p).a = a;
}

void QuickCell1GBAOnly(Vec3i p, unsigned char g, unsigned char b, unsigned char a) {
    gh->TMap->TexCell(1, p).g = g;
    gh->TMap->TexCell(1, p).b = b;
    gh->TMap->TexCell(1, p).a = a;
}

void UpdateZone(Vec3i p, Vec3i s) {
    gh->TMap->TackMultiChange(p, s);
}

int AllocAddInfo(int nr) {
    return gh->AllocAddInfo(nr);
}

void AlterAddInfo(int pos, Vec3f p, float a) {
    gh->AdditionalInformationMapData[pos] = RGBAf(p, a);
}

void AlterAddInfo(int pos, float x, float y, float z, float a) {
    gh->AdditionalInformationMapData[pos] = RGBAf(x,y,z, a);
}

void MarkAddDataForReload() {
    gh->MarkAddInfoForReload();
}

#include <functional>
#include <unordered_map>
#include <iostream>
using namespace std;
unordered_map<string, function<function<void()>(istream&)>> funcs;
unordered_map<string, int> aliases;
using fn = function<void()>;
#define DONE (i>>ws).eof()
//TODO nobody can read this
void initfuncs() {
    
    funcs["EmptyBox"] = [](istream& i) -> function<void()> {
        Vec3i p,s; BlockType b; i>>p>>s>>b;
        int density = DEFAULT_DENSITY; if(!DONE) i>>density;
        return bind(MakeEmptyBox, p, s, b, density, DEFAULT_BRIGHT, 1);
    };
    funcs["Cell"] = [](istream& i) -> function<void()> { 
        Vec3i p; BlockType b; i>>p>>b;
        return bind(ChangeCell, 0, p, 1, DEFAULT_BRIGHT, 255, b);
    };
    funcs["ClearCell"] = [](istream& i) -> function<void()> {
        Vec3i p; i>>p; //TODO merge with "Block"
        return bind(ChangeCell, 0, p, 0, DEFAULT_BRIGHT, 0, DEFAULT_EMPTY_BLOCK);
    };
    funcs["ClearRange"] = [](istream& i) -> function<void()> {
        Vec3i p,s; i>>p>>s;
        return bind(ClearRange, p, s);
    };
    funcs["PaintRange"] = [](istream& i) -> function<void()> {
        Vec3i p,s; BlockType b; i>>p>>s>>b;
        int density = DEFAULT_DENSITY; if(!DONE) i>>density;
        return bind(PaintRange, p, s, b, density, 1);
    };
    funcs["Warp"] = [](istream& i) -> function<void()> { 
        Vec3i p,s; Vec3f x;
        i>>p>>s>>x;
        return bind(SetWarpSpaceArea, p, s, x);
    };
    funcs["JumpSection"] = [](istream& i) -> function<void()> {
        Vec3i p,s; Vec3f ofs, f1 = {1,0,0},f2={0,1,0},f3={0,0,1};
        i>>p>>s>>ofs;
        if(!DONE) i>>f1>>f2>>f3;
        return bind(MakeJumpSection, p, s, ofs, f1, f2, f3);
    };

}
#endif

