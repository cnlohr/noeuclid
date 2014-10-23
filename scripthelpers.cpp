#include "scripthelpers.h"
#include "RTHelper.h"
#include "sys/stat.h"
extern RTHelper * gh;
//File for useful scripts.

//Defines outside extents.

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

void ClearCell(Vec3i p) {
    ChangeCell(0, p, {0, DEFAULT_BRIGHT, 0, DEFAULT_EMPTY_BLOCK});
}


void PaintRange(Vec3i p, Vec3i s, byte block, byte density) {
    PaintRange(p, s, {1,190,density,block});
}
void PaintRange(Vec3i p, Vec3i s, RGBA val) {
    int i, j, k;
    for (i = p.x; i < p.x + s.x; i++)
        for (j = p.y; j < p.y + s.y; j++)
            for (k = p.z; k < p.z + s.z; k++) {
                gh->TMap->TexCell(0, {i, j, k}) = val;
            }
    UpdateZone(p, s + Vec3i{1,1,1});
}

void ClearRange(Vec3i p, Vec3i s) {
    PaintRange(p, s, {0, DEFAULT_BRIGHT, DEFAULT_EMPTY_BLOCK, 0});
}

void MakeEmptyBox(Vec3i p, Vec3i s, bool force_empty, RGBA v) {
    //Will cause an update over our whole area.
    if (force_empty)
        ClearRange(p, s);
    else
        UpdateZone(p, s + Vec3i{1,1,1});


    for (int j = p.x; j <= p.x + s.x; j++)
        for (int k = p.y; k <= p.y + s.y; k++) {
            gh->TMap->TexCell(0, {j, k, p.z}) = v;
            gh->TMap->TexCell(0, {j, k, p.z + s.z}) = v;
        }

    for (int j = p.z; j <= p.z + s.z; j++)
        for (int k = p.x; k <= p.x + s.x; k++) {
            gh->TMap->TexCell(0, {k, p.y, j}) = v;
            gh->TMap->TexCell(0, {k, p.y + s.y, j}) = v;
        }

    for (int j = p.z; j <= p.z + s.z; j++)
        for (int k = p.y; k <= p.y + s.y; k++) {
            gh->TMap->TexCell(0, {p.x, k, j}) = v;
            gh->TMap->TexCell(0, {p.x + s.x, k, j}) = v;
        }

}

void MakeJumpSection(Vec3i p, Vec3i s, Vec3f offset, Vec3f f1, Vec3f f2, Vec3f f3) {
    int newalloc = gh->AllocAddInfo(4);
    gh->AdditionalInformationMapData[newalloc + 0] = f1;
    gh->AdditionalInformationMapData[newalloc + 1] = f2;
    gh->AdditionalInformationMapData[newalloc + 2] = f3;
    gh->AdditionalInformationMapData[newalloc + 3] = offset;

    short i, j, k;
    for (i = p.x; i <= p.x + s.x; i++)
        for (j = p.y; j <= p.y + s.y; j++)
            for (k = p.z; k <= p.z + s.z; k++) {
                QuickCell1GBAOnly({i, j, k}, 0, newalloc % AddSizeStride, newalloc / AddSizeStride);
            }
    UpdateZone(p, s + Vec3i{1,1,1});
    gh->MarkAddInfoForReload();
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

    byte xt = cc.x * 255;
    byte yt = cc.y * 255;
    byte zt = cc.z * 255;
    byte wt = wterm * 255;

    //	float xyzpres = sqrt( xcomp*xcomp + ycomp*ycomp + zcomp*zcomp );

    short i, j, k;
    for (i = p.x; i <= p.x + s.x; i++)
        for (j = p.y; j <= p.y + s.y; j++)
            for (k = p.z; k <= p.z + s.z; k++) {
                gh->TMap->TexCell(2, {i, j, k}) = {xt, yt, zt, wt};
            }
    UpdateZone(p, s + Vec3i{1,1,1});
}

int IsPlayerInRange(Vec3f p, Vec3f s) {
    if (gPosition.x >= p.x && gPosition.x <= p.x + s.x &&
            gPosition.y >= p.y && gPosition.y<= p.y + s.y &&
            gPosition.z >= p.z && gPosition.z <= p.z + s.z) return 1;
    return 0;
}

bool fileChanged(string fname) {
    static unordered_map<string, time_t> modTime;
    struct stat s; stat(fname.c_str(), &s);
    time_t mtime = s.st_mtime;
    if(modTime[fname] != mtime) {
        modTime[fname] = mtime; return true;
    } else return false;
}

void ChangeCell(int t, Vec3i p, RGBA c) {
    gh->TMap->TexCell(t, p) = c;
    gh->TMap->TackChange(p);
}

void QuickCell(int t, Vec3i p, RGBA c) {
    gh->TMap->TexCell(t, p) = c;
}

void QuickCell1GBAOnly(Vec3i p, byte g, byte b, byte a) {
    gh->TMap->TexCell(1, p).g = g;
    gh->TMap->TexCell(1, p).b = b;
    gh->TMap->TexCell(1, p).a = a;
}

void UpdateZone(Vec3i p, Vec3i s) {
    gh->TMap->TackMultiChange(p, s);
}

void Cell(int x, int y, int z, byte block, byte density) { 
    ChangeCell(0,{x,y,z},{1,190,density,block});
}

void EmptyBox(Vec3i p, Vec3i s, byte block, byte density) {
    MakeEmptyBox(p, s, true, RGBA{1, DEFAULT_BRIGHT, density, block});
}