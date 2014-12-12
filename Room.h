#ifndef ROOM_H
#define	ROOM_H

#include "scripthelpers.h"

extern int curroom;

class Room {
public:
    vector<initfn> inits;
    vector<runfn> runs;
    double timeInRoom = 0, maxTime = 1000;
    Vec3f start, exitr1, exitr2;
    void reset();
    void begin();
    void update();
    void (*initscript)(void);
    void (*runscript)(double);
};

#endif	/* ROOM_H */

