#include "Common.h"

const Vec3it& Vec3it::operator++() {
    ++i.z;
    if(i.z==c.e.z) {
        i.z = c.s.z;
        ++i.y;
        if(i.y == c.e.y) {
            i.y = c.s.y;
            ++i.x;
        }
    }
    return *this;
}