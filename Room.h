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
    double timeIn = 0, maxTime = 1000;
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

class Room1 : public Room {
public:
    void init();
    void run();
};

class Room2 : public Room {
public:
    int stage;
    void init();
    void run();
};

class Room3 : public Room {
public:
    int already_setup_jumpspace, already_removed_stretch = 0;
    void init();
    void run();
};

class Room4 : public Room {
public:
    void init();

    void run();
};

class Room5 : public Room {
};

class Room6 : public Room {
public:
    void run();
};

class Room7 : public Room {
public:
    void init();
    void run();
};

class Room8 : public Room {
public:
    void init();
};

class Room9 : public Room {
public:
    double TimeTransition = 0;
    char lifemap[16 * 16];
    char newlife[16 * 16];

    void init();
    void run();
};

class Room10 : public Room {
public:
    void run();
};

class Room11 : public Room {
public:
    void init();
    void run();
};


#endif	/* ROOM_H */

