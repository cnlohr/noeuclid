//Copyright 2011 <>< Charles Lohr - File may be licensed under the MIT/x11 license or the new BSD license.  You choose.

#ifndef _MAP_H
#define _MAP_H

#include <stdio.h>
#include <math.h>
#include <list>
#include "Common.h"
#include "OGLParts.h"


//MAP Contains "GLTextureData"


using namespace std;
class RTHelper;

struct CellUpdate {
    Vec3i p; Vec3i s;
};

class Map {
public:
    Map(string filename, RTHelper * p, bool fakemode);
    ~Map();
    void FakeIt();
    void DefaultIt();
    void RecalculateAccelerationStructure(int ix, int iy, int iz, int sx, int sy, int sz);
    void Draw();

    list< CellUpdate > ListUpdates;

    void TackChange(Vec3i p) {
        ListUpdates.push_back({p, {0,0,0}});
    }

    void TackMultiChange(Vec3i p, Vec3i s) {
        ListUpdates.push_back({p, s});
    }
    
    inline RGBA & TexCell(unsigned i, int x, int y, int z) {
        x = ((unsigned long) x) % GLH_SIZEX;
        y = ((unsigned long) y) % GLH_SIZEY;
        z = ((unsigned long) z) % GLH_SIZEZ;
        return GLTextureData[i][x + y * GLH_SIZEX + z * GLH_SIZEX * GLH_SIZEY];
    }

    inline RGBA & TexCell(unsigned i, Vec3iu p) {
        p.x = ((unsigned long) p.x) % GLH_SIZEX;
        p.y = ((unsigned long) p.y) % GLH_SIZEY;
        p.z = ((unsigned long) p.z) % GLH_SIZEZ;
        return GLTextureData[i][p.x + p.y * GLH_SIZEX + p.z * GLH_SIZEX * GLH_SIZEY];
    }
    
    inline RGBA & TexCell(unsigned i, Vec3i p) {
        p.x = ((unsigned long) p.x) % GLH_SIZEX;
        p.y = ((unsigned long) p.y) % GLH_SIZEY;
        p.z = ((unsigned long) p.z) % GLH_SIZEZ;
        return GLTextureData[i][p.x + p.y * GLH_SIZEX + p.z * GLH_SIZEX * GLH_SIZEY];
    }


    bool m_bTriggerFullRecalculate;
    bool m_bReloadFullTexture;

private:
    void UpdateCellSpecific(Vec3i p, Vec3i s);

    void SetCellInternal(unsigned x, unsigned y, unsigned z, unsigned cell, unsigned color);
    
    void SetCellInternal(Vec3iu p, unsigned cell, unsigned color);

    //Comp 0("GeoTex"): 
    //Red Channel:   Block Type < not ????
    //Green Channel: Metadata / Cell Type? Or something like that?
    //Blue Channel: "Density" of block according to addtex (use this to render)
    //Alpha Channel: Actual Cell to Draw 
    //Comp 1("AddTex"): [possible hit?, jumpx, jumpy, jumpz]
    //Comp 2("MovTex"): [x] [y] [z] [w] (Universe-warping)
    RGBA * GLTextureData[3];
    unsigned int i3DTex[3];

    bool doSubtrace;
    RTHelper * parent;

public:

    void ChangeSubtrace(bool bSubtrace) {
        doSubtrace = bSubtrace;
        m_bTriggerFullRecalculate = true;
    }

    bool GetSubtrace() {
        return doSubtrace;
    }

    void UpdateSphereTexture();
};

#endif
