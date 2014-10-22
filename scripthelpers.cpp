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
    ChangeCell(0, p, 0, DEFAULT_BRIGHT, 0, DEFAULT_EMPTY_BLOCK);
}

void PaintRange(Vec3i p, Vec3i s, int cell, int density, int unclear) {
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

void MakeJumpSection(Vec3i p, Vec3i s, Vec3f ofs, Vec3f f1, Vec3f f2, Vec3f f3) {
    int newalloc = gh->AllocAddInfo(4);
    gh->AdditionalInformationMapData[newalloc + 0] = f1;
    gh->AdditionalInformationMapData[newalloc + 1] = f2;
    gh->AdditionalInformationMapData[newalloc + 2] = f3;
    gh->AdditionalInformationMapData[newalloc + 3] = ofs;

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

bool fileChanged(string fname) {
    static unordered_map<string, time_t> modTime;
    struct stat s; stat(fname.c_str(), &s);
    time_t mtime = s.st_mtime;
    if(modTime[fname] != mtime) {
        modTime[fname] = mtime; return true;
    } else return false;
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


