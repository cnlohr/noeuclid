/* 
 * File:   Room.h
 * Author: tehdog
 *
 * Created on 20. Oktober 2014, 22:12
 */

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
    virtual ~Room() {};
    void reset();
    void begin();
    void update();
    void (*initscript)(void);
    void (*runscript)(double);

    virtual void run() {
    };

    virtual void init() {
    };
};

#endif	/* ROOM_H */

