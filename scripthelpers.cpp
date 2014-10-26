#include "scripthelpers.h"
#include "RTHelper.h"
#include "sys/stat.h"
#include "GameMap.h"
#include <fstream>
extern RTHelper gh;
//File for useful scripts.
extern GameMap gamemap;
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

void ClearCellV(Vec3i p) {
    ChangeCellV(0, p, {0, DEFAULT_BRIGHT, 0, DEFAULT_EMPTY_BLOCK});
}

void PaintRangeV(Vec3i p, Vec3i s, RGBA val) {
    int i, j, k;
    for (i = p.x; i < p.x + s.x; i++)
        for (j = p.y; j < p.y + s.y; j++)
            for (k = p.z; k < p.z + s.z; k++) {
                gh.TMap->TexCell(0, {i, j, k}) = val;
            }
    UpdateZoneV(p, s + Vec3i{1,1,1});
}

void ClearRangeV(Vec3i p, Vec3i s) {
    PaintRangeV(p, s, {0, DEFAULT_BRIGHT, DEFAULT_EMPTY_BLOCK, 0});
}

void EmptyBoxV(Vec3i p, Vec3i s, bool force_empty, RGBA v) {
    //Will cause an update over our whole area.
    if (force_empty)
        ClearRangeV(p, s);
    else
        UpdateZoneV(p, s + Vec3i{1,1,1});


    for (int j = p.x; j <= p.x + s.x; j++)
        for (int k = p.y; k <= p.y + s.y; k++) {
            gh.TMap->TexCell(0, {j, k, p.z}) = v;
            gh.TMap->TexCell(0, {j, k, p.z + s.z}) = v;
        }

    for (int j = p.z; j <= p.z + s.z; j++)
        for (int k = p.x; k <= p.x + s.x; k++) {
            gh.TMap->TexCell(0, {k, p.y, j}) = v;
            gh.TMap->TexCell(0, {k, p.y + s.y, j}) = v;
        }

    for (int j = p.z; j <= p.z + s.z; j++)
        for (int k = p.y; k <= p.y + s.y; k++) {
            gh.TMap->TexCell(0, {p.x, k, j}) = v;
            gh.TMap->TexCell(0, {p.x + s.x, k, j}) = v;
        }

}

void JumpSpaceV(Vec3i p, Vec3i s, Vec3f offset, Vec3f f1, Vec3f f2, Vec3f f3) {
    int newalloc = gh.AllocAddInfo(4);
    gh.AdditionalInformationMapData[newalloc + 0] = f1;
    gh.AdditionalInformationMapData[newalloc + 1] = f2;
    gh.AdditionalInformationMapData[newalloc + 2] = f3;
    gh.AdditionalInformationMapData[newalloc + 3] = offset;

    short i, j, k;
    for (i = p.x; i <= p.x + s.x; i++)
        for (j = p.y; j <= p.y + s.y; j++)
            for (k = p.z; k <= p.z + s.z; k++) {
                    gh.TMap->TexCell(1, {i, j, k}).g = 0;
                    gh.TMap->TexCell(1, {i, j, k}).b = newalloc % AddSizeStride;
                    gh.TMap->TexCell(1, {i, j, k}).a = newalloc / AddSizeStride;
            }
    UpdateZoneV(p, s + Vec3i{1,1,1});
    gh.MarkAddInfoForReload();
}

void WarpSpaceV(Vec3i p, Vec3i s, Vec3f comp) {
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
    if(xt == 0 || yt==0 || zt==0) throw runtime_error("tried to warp space to 0");
    //	float xyzpres = sqrt( xcomp*xcomp + ycomp*ycomp + zcomp*zcomp );

    short i, j, k;
    for (i = p.x; i <= p.x + s.x; i++)
        for (j = p.y; j <= p.y + s.y; j++)
            for (k = p.z; k <= p.z + s.z; k++) {
                gh.TMap->TexCell(2, {i, j, k}) = {xt, yt, zt, wt};
            }
    UpdateZoneV(p, s + Vec3i{1,1,1});
}

int PlayerInRangeV(Vec3f p, Vec3f s) {
    if (gPosition.x >= p.x && gPosition.x <= p.x + s.x &&
            gPosition.y >= p.y && gPosition.y<= p.y + s.y &&
            gPosition.z >= p.z && gPosition.z <= p.z + s.z) return 1;
    return 0;
}

// true if file has changed since last call
bool fileChanged(string fname) {
    static unordered_map<string, time_t> modTime;
    struct stat s; stat(fname.c_str(), &s);
    time_t mtime = s.st_mtime;
    if(modTime[fname] != mtime) {
        modTime[fname] = mtime; return true;
    } else return false;
}

// read complete file into string
string readFile(string fname) {
    ifstream i(fname);
    if(!i.is_open()) throw runtime_error("Can't open file "+fname);
    return string((istreambuf_iterator<char>(i)), istreambuf_iterator<char>());
}

void ChangeCellV(int t, Vec3i p, RGBA c) {
    gh.TMap->TexCell(t, p) = c;
    gh.TMap->TackChange(p);
}

void QuickCellV(int t, Vec3i p, RGBA c) {
    gh.TMap->TexCell(t, p) = c;
}

void QuickCell1GBAOnlyV(Vec3i p, byte g, byte b, byte a) {

}

void UpdateZoneV(Vec3i p, Vec3i s) {
    gh.TMap->TackMultiChange(p, s);
}

void ChangeCell(int x, int y, int z, byte block, byte density) {
    ChangeCellV(0,{x,y,z},{1,190,density,block});
}

void ClearCell(int x, int y, int z) {
    ClearCellV({x,y,z});
}

void QuickCell(int t, int x, int y, int z, byte block, byte density) {
    gh.TMap->TexCell(t, {x,y,z}) = {1,190,density,block};
}

void ClearRange(int x, int y, int z, int x2, int y2, int z2) {
    ClearRangeV({x,y,z}, {x2,y2,z2});
}

void EmptyBox(int x, int y, int z, int x2, int y2, int z2, byte block, byte density) {
    EmptyBoxV({x,y,z}, {x2,y2,z2}, true, RGBA{1, DEFAULT_BRIGHT, density, block});
}

void WarpSpace(int x, int y, int z, int x2, int y2, int z2, float f1, float f2, float f3) {
    WarpSpaceV({x,y,z},{x2,y2,z2},{f1,f2,f3});
}

int PlayerInRange(float x, float y, float z, float x2, float y2, float z2) {
    return PlayerInRangeV({x,y,z},{x2,y2,z2});
}

void PaintRange(int x, int y, int z, int x2, int y2, int z2, byte block, byte density) {
    PaintRangeV({x,y,z},{x2,y2,z2}, {1,190,density,block});
}

void JumpSpace(int x, int y, int z, int x2, int y2, int z2, float xofs, float yofs, float zofs) {
    JumpSpaceV({x,y,z},{x2,y2,z2},{xofs,yofs,zofs});
}

void JumpSpaceExtended(int x, int y, int z, int x2, int y2, int z2, float xofs, float yofs, float zofs, float xm1, float ym1, float zm1, float xm2, float ym2, float zm2, float xm3, float ym3, float zm3) {
    JumpSpaceV({x,y,z},{x2,y2,z2},{xofs,yofs,zofs},{xm1,  ym1,  zm1},  {xm2,  ym2,  zm2},  {xm3,  ym3,  zm3});
}

void ClearPickableBlocks() {
    gamemap.ClearPickableBlocks();
}

void PlacePickableAt(int x, int y, int z, float initDensity) {
    gamemap.PlacePickableAt({x,y,z}, initDensity);
}

void AddDeathBlock(int x, int y, int z) {
    gamemap.AddDeathBlock({x,y,z});
}

void UpdateZone(int x, int y, int z, int x2, int y2, int z2) {
    UpdateZoneV({x,y,z},{x2,y2,z2});
}
void die() {
    gamemap.die();
}