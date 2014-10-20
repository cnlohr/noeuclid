/* 
 * File:   GameMap.cpp
 * Author: tehdog
 * 
 * Created on 19. Oktober 2014, 02:26
 */

#include "scripthelpers.h"
#include "GameMap.h"
#include <cstring>
#include <functional>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
using namespace std;
#define START_ROOM 0
int lastroom = -1, curroom = START_ROOM;


class Room {
public:
    Room() : maxTime(1000), start({0,0,0}),exitr1({0,0,0}),exitr2({0,0,0}) {}
    Room(double maxTime, Vec3f start, Vec3f exitr1, Vec3f exitr2) :
        maxTime(maxTime), start(start), exitr1(exitr1), exitr2(exitr2) {
        inits.push_back([this](){init();});
    };
    vector<function<void()>> inits;
    double timeIn = 0, maxTime;
    Vec3f start, exitr1, exitr2;
    
    void reset() {
        gPosition = start;
        GameTimer = maxTime;
        begin();
    }
    void begin() {
        cout<<"beginning room"<<endl;
        for(auto& initfn:inits) initfn();
    }
    
    void update() {
        run();
        if (IsPlayerInRange(exitr1, exitr2)) {
            curroom++;
        }
    }

    virtual void run() {
    };

    virtual void init() {
    };
};

extern vector<Room*> rooms;

void Die() {
    rooms[curroom]->reset();
    GameAttempt++;
}

void GameMap::collision(struct CollisionProbe * ddat) {
    ///XXX TODO: pointers for where we're aimed should come in here.
    //printf( "CC %f %f %f  (%f)\n", ddat->TargetLocation.r, ddat->TargetLocation.g, ddat->TargetLocation.b, ddat->Normal.a ); 
}

class Room1 : public Room {
public:
    void init() {
        for (int i = 0; i < 10; i++) {
            float den = (i + 1.0) / 12.0;
            if (den < .1) den = .1;
            SetWarpSpaceArea({2, 2, 59 - i}, {4, 4, 1},{ den, den, den}); //very big box
        }
    }

    void run() {
        //Make cute breathe
        MakeEmptyBox({2, 2, 48}, {4, 4, 11}, 4, swoovey(timeIn * .1, 1)*70 + 185, DEFAULT_BRIGHT, 1); //Chute
        ClearCell({4, 4, 60});
        ClearCell({4, 4, 59});
        //Make hole in side.
        ClearRange({3, 6, 51}, {3, 1, 2});


        int capden = 255 - timeIn * 200;

        if (capden < 0) {
            ChangeCell(0, {4, 4, 60}, 0, DEFAULT_BRIGHT, 0, GOAL_BLOCK);
        } else {
            ChangeCell(0, {4, 4, 60}, 1, DEFAULT_BRIGHT, capden, GOAL_BLOCK);
        }
    }
};

class Room2 : public Room {
public:
    int stage;

    void init() {
        stage = 0;
    }

    void run() {
        gDaytime = gDaytime * .99 + 400 * .01;

        int capden = 255 - timeIn * 200;

        if (capden < 0) {
            PaintRange({4, 10, 51}, {1, 1, 2}, GOAL_BLOCK, 0);
        } else {
            PaintRange({4, 10, 51}, {1, 1, 2}, GOAL_BLOCK, capden);
        }

        //Force user to look around to get out.
        switch (stage) {
            case 0:
                if (gDirection.y > .7 && IsPlayerInRange({3, 12, 48}, {3, 3, 3})) stage = 1;
                break;
            case 1:
                if (gDirection.y < -.7) stage = 2;
                break;
            case 2:
                if (gDirection.y > .7) stage = 3;
                break;
            case 3:
                PaintRange({3, 11, 47}, {3, 1, 8}, SPAAACE_CELL, 255);
                if (gDirection.y < -.7) stage = 4;
                break;
            case 4:
                //Open hole in front of room.
                PaintRange({3, 15, 47}, {3, 2, 3}, WHITE_BLOCK, 255);
                ClearRange({4, 15, 48}, {1, 1, 1});
                PaintRange({4, 16, 48}, {1, 1, 1}, GOAL_BLOCK, 255);
                SetWarpSpaceArea({3, 15, 47}, {4, 4, 3}, {.3, .3, .3});
                break;

        }
    }
};

class Room3 : public Room {
public:
    int already_setup_jumpspace, already_removed_stretch = 0;

    void init() {
        already_removed_stretch = 0;
        already_setup_jumpspace = 0;
    }

    void run() {
        gDaytime = gDaytime * .999 + 472 * .001;

        if (IsPlayerInRange({2, 17, 41}, {10, 10, 1.1}) && !already_removed_stretch) {
            printf("Dewarpify.\n");
            //Remove the stretchyness that let us into this room.
            SetWarpSpaceArea({1, 15, 40}, {12, 12, 12}, {1, 1, 1});

            PaintRange({4, 16, 48}, {1, 1, 1}, DEADGOAL_BLOCK, 255);
            already_removed_stretch = 1;
        }

        if (IsPlayerInRange({12, 17, 41}, {1, 1, 2}) && already_setup_jumpspace == 0) {
            ClearRange({1, 16, 37}, {1, 1, 4});
            ClearRange({1, 16, 48}, {1, 1, 4});
            MakeJumpSection({1, 16, 39}, {12, 12, 3}, {0, 0, 7});
            MakeJumpSection({1, 16, 51}, {12, 12, 3}, {0, 0, -7});
            printf("Jumpspace\n");
            already_setup_jumpspace = 1;
        }

        int capden = 255 - timeIn * 200;

        int i;
        //Make shifting platform.
        for (i = 2; i < 13; i++) {
            float swoove = swoovey(timeIn * .1, 3.0) + .5;
            swoove = (swoove * 13.0 - i)*.4;
            float swd = 1. - swoove*swoove;
            if (swd < 0) swd = 0;
            if (swd > 1) swd = 1;
            swd = pow(swd, .5);
            PaintRange({i, 21, 46}, {1, 2, 1}, 17, swd * 255); //Random little platform
        }

        if (capden < 0) {
            if (already_removed_stretch == 0)
                PaintRange({4, 16, 48}, {1, 1, 1}, GOAL_BLOCK, 0);
        } else {
            PaintRange({4, 16, 48}, {1, 1, 1}, GOAL_BLOCK, capden);
        }
    }
};

class Room4 : public Room {
public:
    void init() {
        //Increasingly warped world.
        for (int x = 0; x < 14; x++) {
            SetWarpSpaceArea({2 + x, 34, 42}, {3, 15, 20}, {.8f / (14 - x), .8f / (14 - x), .4f});
        }
        
        //Fill some lava in in a random pattern.
        unsigned long i = 1;
        for (int x = 0; x < 15; x++)
            for (int y = 0; y < 11; y++) {
                i = (i * 2089) % 491;
                if (((i % 7) > 1)) {
                    QuickCell(0, {3 + x, 35 + y, 43}, 1, DEFAULT_BRIGHT, 60, 10); //Lava
                    AddDeathBlock({3 + x, 35 + y, 43});
                }
            }
        //Target in floor.
    }

    void run() {
        int capden = 255 - timeIn * 200;

        //Open Room 3 goal to room 4.
        if (capden < 0) {
            PaintRange({6, 33, 48}, {1, 1, 2}, GOAL_BLOCK, 0);
            PaintRange({3, 28, 48}, {6, 1, 2}, DEADGOAL_BLOCK, 255);
        } else {
            PaintRange({6, 33, 48}, {1, 1, 2}, GOAL_BLOCK, capden);
            PaintRange({3, 28, 48}, {6, 1, 2}, DEADGOAL_BLOCK, 255 - capden);
        }

        for (int x = 0; x < 14; x++)
            for (int y = 0; y < 14; y++) {
                QuickCell(0, {4 + x, 34 + y, 43 + 10}, 1, DEFAULT_BRIGHT, (sin(x * 2 + y + timeIn)*80 + 80), 9);
            }
        UpdateZone({3, 33, 43 + 10},{ 15, 15, 3});
    }
};

class Room5 : public Room {
public:    
    void run() {
        int capden = 255 - timeIn * 200;
        //Open Room 3 goal to room 4.
        if (capden < 0) {
            PaintRange({5, 47, 43}, {1, 1, 1}, GOAL_BLOCK, 0);
        } else {
            PaintRange({5, 47, 43}, {1, 1, 1}, GOAL_BLOCK, capden);
        }
    }
};

class Room6 : public Room {
public:
    void run() {
        int capden = 255 - timeIn * 200;

        if (capden < 0) {
            PaintRange({5, 30, 31}, {1, 1, 2}, GOAL_BLOCK, 0);
        } else {
            PaintRange({5, 30, 31}, {1, 1, 2}, GOAL_BLOCK, capden);
        }

        if (IsPlayerInRange({3, 18, 30}, {5, 7, 4})) {
            Die();
        }
    }
};

class Room7 : public Room {
public:
    void init() {
        //Put a little mound of pickables over in the corner.
        pickables_in_inventory = 0;
        ClearPicableBlocks();
        int x, y, z;
        for (x = 0; x < 3; x++) {
            for (y = 0; y < 3; y++) {
                for (z = 0; z < 2; z++) {
                    PlacePickableAt({x + 5, y + 5, z + 31}, -(x + y) - z * 2.0 - 2);
                }
            }
        }

        ClickCellCB = PickableClick;
        char lavamap[8 * 8] = {
            0, 1, 1, 1, 1, 1, 1, 0,
            1, 1, 0, 0, 0, 0, 1, 1,
            1, 0, 1, 0, 0, 1, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 1, 1, 1, 1, 0, 1,
            1, 0, 0, 1, 1, 0, 0, 1,
            1, 1, 0, 0, 0, 0, 1, 1,
            0, 1, 1, 1, 1, 1, 1, 0,
        };

        //Now, in the goal chamber add some lava.
        for (x = 0; x < 8; x++)
            for (y = 0; y < 8; y++) {
                if (lavamap[x + y * 8]) {
                    QuickCell(0, {4 + x, 4 + y, 20}, 1, DEFAULT_BRIGHT, 60, 10); //Lava
                    AddDeathBlock({4 + x, 4 + y, 20});
                }
            }
        //Goal
        PaintRange({24, 7, 21}, {1, 1, 2}, GOAL_BLOCK, 255);

    }

    void run() {
        int capden = 255 - timeIn * 200;

        if (capden < 0) {
            PaintRange({5, 15, 31}, {1, 1, 2}, GOAL_BLOCK, 0);
        } else {
            PaintRange({5, 15, 31}, {1, 1, 2}, GOAL_BLOCK, capden);
        }

        if (IsPlayerInRange({13, 1, 18}, {7, 15, 3})) {
            Die();
        }
    }
};

class Room8 : public Room {
public:
    void init() {

        //Make sure room 7 is set up for us. (And room 4)
        rooms[7]->begin();
        rooms[4]->begin();

        //Place some goodies in here.
        pickables_in_inventory = 0;
        ClearPicableBlocks();
        int x, y, z;
        for (x = 0; x < 2; x++) {
            for (y = 0; y < 4; y++) {
                for (z = 0; z < 2; z++) {
                    PlacePickableAt({x + 6, y + 29, z + 48}, -(x + y)*.5 - z * .5);
                }
            }
        }
        
    }

    void run() {
        int capden = 255 - timeIn * 200;

        if (capden < 0) {
            PaintRange({24, 7, 21}, {1, 1, 2}, GOAL_BLOCK, 0);
        } else {
            PaintRange({24, 7, 21},{1, 1, 2}, GOAL_BLOCK, capden);
        }
    }
};

class Room9 : public Room {
public:
    double TimeTransition = 0;
    char lifemap[16 * 16];
    char newlife[16 * 16];

    void init() {
        //Room 8's gotta be here.
        rooms[8]->begin();

        int i = 1;
        for (int x = 0; x < 16; x++)
            for (int y = 0; y < 16; y++) {
                i = (i * 2089) % 491;
                if (((i % 7) > 2)) {
                    newlife[x + y * 16] = 1;
                }
            }
    }

    void run() {
        int capden = 255 - timeIn * 200;
        //Fix life map up.
        if (TimeTransition >= 1) {
            memcpy(lifemap, newlife, sizeof ( lifemap));
            for (int x = 0; x < 16; x++) {
                for (int y = 0; y < 16; y++) {
                    int alive = lifemap[x + y * 16];
                    int qty =
                            loopingarrayaccess(lifemap, 16, 16, x - 1, y - 1) +
                            loopingarrayaccess(lifemap, 16, 16, x - 1, y) +
                            loopingarrayaccess(lifemap, 16, 16, x - 1, y + 1) +
                            loopingarrayaccess(lifemap, 16, 16, x, y - 1) +
                            loopingarrayaccess(lifemap, 16, 16, x, y) +
                            loopingarrayaccess(lifemap, 16, 16, x, y + 1) +
                            loopingarrayaccess(lifemap, 16, 16, x + 1, y - 1) +
                            loopingarrayaccess(lifemap, 16, 16, x + 1, y) +
                            loopingarrayaccess(lifemap, 16, 16, x + 1, y + 1);

                    newlife[x + y * 16] = alive;

                    if (qty == 3 || qty == 4)
                        newlife[x + y * 16] = 1;
                    else if (qty > 4)
                        newlife[x + y * 16] = 0;
                    else if (qty < 2)
                        newlife[x + y * 16] = 0;
                }
            }
            TimeTransition -= 1;
        }

        for (int x = 0; x < 16; x++)
            for (int y = 0; y < 16; y++) {
                double den = ((double) lifemap[x + y * 16]) * (1. - TimeTransition) + ((double) newlife[x + y * 16]) * TimeTransition;
                ChangeCell(0, {3 + x, 50 + y, 47}, 1, DEFAULT_BRIGHT, den * 255, 5);
            }
        TimeTransition += worldDeltaTime;

        if (capden < 0) {
            PaintRange({16, 48, 54}, {2, 1, 2}, GOAL_BLOCK, 0);
        } else {
            PaintRange({16, 48, 54}, {2, 1, 2}, GOAL_BLOCK, capden);
        }
        if (gPosition.z < 47.8) {
            Die();
        }
    }
};

class Room10 : public Room {
public:
    void init() {
        rooms[9]->begin();

        

    }

    void run() {
        int capden = 255 - timeIn * 200;
        for (int z = 46; z < 57; z++)
            for (int y = 67; y < 77; y++) {
                if ((y + z)&1) continue;
                QuickCell(0,{2, y, z}, 1, DEFAULT_BRIGHT, sin(timeIn * 10.0 + z + y)*50.5 + 160, 5);
            }
        UpdateZone({1, 66, 46},
        {
            1, 10, 10
        });

        for (int z = 46; z < 57; z++)
            for (int x = 2; x < 12; x++) {
                if ((x + z)&1) continue;
                QuickCell(0,{x, 77, z}, 1, DEFAULT_BRIGHT, sin(timeIn * 10.0 + x + z)*50.5 + 160, 5);
            }
        UpdateZone({1, 76, 46},
        {
            10, 1, 10
        });

        for (int y = 67; y < 77; y++)
            for (int x = 2; x < 12; x++) {
                if ((y + x)&1) continue;
                QuickCell(0,{x, y, 56}, 1, DEFAULT_BRIGHT, sin(timeIn * 10.0 + x + y)*50.5 + 160, 5);
            }
        UpdateZone({2, 67, 56},
        {
            10, 10, 1
        });


        if (capden < 0) {
            PaintRange({4, 66, 47},
            {
                2, 1, 3
            }, GOAL_BLOCK, 0);
        } else {
            PaintRange({4, 66, 47},
            {
                2, 1, 3
            }, GOAL_BLOCK, capden);
        }

        if (IsPlayerInRange({3, 70, 46}, {6, 3, 2.1})) {
            Die();
        }

    }
};

vector<Room*> rooms{
    new Room(),
    new Room1(),
    new Room2(),
    new Room3(),
    new Room4(),
    new Room5(),
    new Room6(),
    new Room7(),
    new Room8(),
    new Room9(),
    new Room10()
};

GameMap::GameMap() {
}

GameMap::~GameMap() {
}

void GameMap::init() {
    initfuncs();
    
    ifstream file("games/rooms.txt");
    string line;
    int room = 0;
    int lineNum = 0;
    while(getline(file, line)) {
        lineNum++;
        if(line.back()=='\r') line.pop_back();
        if(line.length()==0) continue;
        
        if(line[0]=='#') continue;
        for(char c: "()") line.erase(remove(line.begin(),line.end(),c),line.end()); // remove parens
        istringstream l(line);
        string cmd; l >> cmd;
        if(cmd == "Room") l >> room;
        else if(cmd == "Start") l >> rooms[room]->start;
        else if(cmd == "Exit") l >> rooms[room]->exitr1 >> rooms[room]->exitr2;
        else if(cmd == "Time") l >> rooms[room]->maxTime;
        else if(cmd == "Init") rooms[room]->inits.push_back([room](){rooms[room]->init();});
        else if(funcs.count(cmd)) {
            rooms[room]->inits.push_back(funcs[cmd](l));
            if(l.fail()) cout<<lineNum<<": Error: failbit"<<endl;
        } else {
            cout <<lineNum<<": Error: Invalid command " << cmd << endl;
        }
    }
    curroom = START_ROOM;
    rooms[curroom]->reset();
}

void GameMap::update() {
    static int was_level_change_pressed;
    int i;

    if (gOverallUpdateNo == 0) {
        init();
    }

    if (curroom != lastroom) {
        printf("Switching to room %d", curroom);
        rooms[curroom]->begin();
        lastroom = curroom;
    }

    gDaytime += worldDeltaTime;
    GameTimer -= worldDeltaTime;
    if (GameTimer < 0) {
        Die();
    }

    rooms[curroom]->timeIn += worldDeltaTime;
    rooms[curroom]->update();

    sprintf(gDialog, "Room: %d\n", curroom);

    if (gKeyMap['l'] || gKeyMap['L']) {
        for (Room* room:rooms) {
            room->begin();
        }
    }

    UpdatePickableBlocks(); //Draw the pickables.

    //Press 'r' to reset room.
    if (gKeyMap['r'] || gKeyMap['R']) {
        rooms[curroom]->reset();
    }

    if (gKeyMap['='] || gKeyMap['+']) {
        if (!was_level_change_pressed)
            curroom++;
        was_level_change_pressed = 1;
        rooms[curroom]->reset();
    } else if (gKeyMap['-'] || gKeyMap['_']) {
        if (!was_level_change_pressed)
            curroom--;
        rooms[curroom]->reset();
    } else
        was_level_change_pressed = 0;

    //printf( "%f %f %f   %f %f %f\n", targetx, targety, targetz, gDirectionX, gDirectionY, gDirectionZ );
    if (gMouseLastClickButton != -1) {
        printf("Click on: %f %f %f\n", gTargetHit.x, gTargetHit.y, gTargetHit.z);
        if (gMouseLastClickButton == 0) {
            //Left-mouse
            Vec3f target = gTargetHit + gTargetNormal * .5;
            Vec3f dist = (target-gPosition);
            
            if (ClickCellCB)
                ClickCellCB(true, target, dist.len());
        } else if (gMouseLastClickButton == 2) {
            //Right-mouse
            Vec3f target = gTargetHit - gTargetNormal * .5;
            Vec3f dist = (target-gPosition);

            if (ClickCellCB)
                ClickCellCB(false, target, dist.len());
        }
        gMouseLastClickButton = -1;
    }

    //We're on the ground.  Make sure we don't die.
    if (gTimeSinceOnGround < .001) {
        Vec3i p = {(int)gPosition.x, (int)gPosition.y, (int)gPosition.z};
        //printf( "%d %d %d\n", px, py, pz );
        if (IsOnDeathBlock(p)) {
            printf("DIE\n");
            Die();
        }
    }


    if (IsPlayerInRange({2, 40, 63}, {16, 16, 4})) {
        int lx = ((int) gPosition.x) - 2;
        int ly = ((int) gPosition.y) - 40;
        sprintf(gDialog, "OnTile %d\n", lx + ly * 16);
    }
    //Util: Make reference swatches.
    for (i = 0; i < 256; i++) {
        ChangeCell(0,{2 + (i % 16), 40 + (i / 16), 63}, 1, DEFAULT_BRIGHT, 255, i);
    }

}
