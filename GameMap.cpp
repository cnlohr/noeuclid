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
#define START_ROOM 9
#define NR_ROOMS 11 //RunRoom is 0 indexed, this should be one greater than the highest numbered room.
int firstrun = 0;
int lastroom = -1;
int room = START_ROOM;


class Room {
public:

    Room(double maxTime, Vec3f start, Vec3f exitr1, Vec3f exitr2) :
        maxTime(maxTime), start(start), exitr1(exitr1), exitr2(exitr2) {
    };
    double timeIn = 0, maxTime;
    Vec3f start, exitr1, exitr2;
    
    void begin() {
        gPosition = start;
        GameTimer = maxTime;
        init();
    }
    
    void update() {
        run();
        if (IsPlayerInRange(exitr1, exitr2)) {
            room++;
        }
    }

    virtual void run() {
        printf("ERROR: no room\n");
    };

    virtual void init() {
        printf("ERROR: no room to init\n");
    };
};

extern vector<Room*> rooms;

void Die() {
    rooms[room]->begin();
    GameAttempt++;
}

void GameMap::collision(struct CollisionProbe * ddat) {
    ///XXX TODO: pointers for where we're aimed should come in here.
    //printf( "CC %f %f %f  (%f)\n", ddat->TargetLocation.r, ddat->TargetLocation.g, ddat->TargetLocation.b, ddat->Normal.a ); 
}

class Room0 : public Room {
public:

    Room0() : Room(100,{3, 3, 64}, {4, 4, 61}, {1, 1, 1}) {
    }

    void init() {
        MakeEmptyBox({1, 1, 60}, {6, 6, 6}, WALL_BLOCK, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1); //White box,
        SetWarpSpaceArea({1, 1, 60}, {6, 6, 6}, {.1, .1, .1}); //very big box.
        ChangeCell(0, {4, 4, 60}, 1, DEFAULT_BRIGHT, 255, GOAL_BLOCK);

        ChangeCell(0, {2, 4, 62}, 1, DEFAULT_BRIGHT, 255, 15);
        ChangeCell(0, {4, 4, 62}, 1, DEFAULT_BRIGHT, 255, 18);
        ChangeCell(0, {3, 2, 61}, 1, DEFAULT_BRIGHT, 255, 35);
        ChangeCell(0, {5, 2, 63}, 1, DEFAULT_BRIGHT, 255, 50);
        ChangeCell(0, {3, 5, 64}, 1, DEFAULT_BRIGHT, 255, 81);
        ChangeCell(0, {5, 5, 63}, 1, DEFAULT_BRIGHT, 255, 22);


        //Util: Make reference swatches.
        for (int i = 0; i < 256; i++) {
            ChangeCell(0, {2 + (i % 16), 40 + (i / 16), 63}, 1, DEFAULT_BRIGHT, 255, i);
        }
    }

    void run() {
    }
};

class Room1 : public Room {
public:

    Room1() : Room(100, {4.28, 4.27, 59.13},{3, 9, 51},{3, 1, 2}) {
    }

    void init() {
        //Box off to the side
        MakeEmptyBox({2, 6, 50}, {4, 4, 3}, 7, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1);


        MakeEmptyBox({2, 2, 48}, {4, 4, 11}, 4, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1); //Chute
        ClearCell({4, 4, 60});
        ClearCell({4, 4, 59});
        int i;
        for (i = 0; i < 10; i++) {
            float den = (i + 1.0) / 12.0;
            if (den < .1) den = .1;
            SetWarpSpaceArea({2, 2, 59 - i}, {4, 4, 1},{ den, den, den}); //very big box
        }
        //Make hole in side.
        ClearRange({3, 6, 51}, {3, 1, 2});

        //ChangeCell( 0, 4, 10, 52, 1, DEFAULT_BRIGHT, 255, GOAL_BLOCK );
        PaintRange({4, 10, 50}, {1, 1, 3}, GOAL_BLOCK, 255);
    }

    void run() {
        //	printf( "%f %f\n", triangle(TimeInRoom1, .9) );
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

    Room2() : Room(100, {4.5, 7.92, 51.92},{3, 15.1, 48}, {3, 1, 2}) {
    }
    int stage;

    void init() {
        stage = 0;

        MakeEmptyBox({2, 11, 47}, {4, 4, 10}, SPAAACE_CELL, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1); //Open area
        SetWarpSpaceArea({2, 11, 46}, {4, 4, 3}, {.3, .3, .3}); //very big box

        //Clear the door in.
        PaintRange({3, 11, 47}, {3, 1, 8}, GOAL_BLOCK, 255);
        ClearRange({4, 11, 51}, {1, 1, 2});
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

    Room3() : Room(100, {4.5, 15.29, 48.44},{6, 32, 48}, {1, 1, 2}) {
    }
    int already_setup_jumpspace, already_removed_stretch = 0;

    void init() {
        already_removed_stretch = 0;
        already_setup_jumpspace = 0;
        SetWarpSpaceArea({1, 15, 40}, {12, 12, 12}, {.3, .3, .3});
        PaintRange({3, 15, 47}, {3, 2, 3}, WHITE_BLOCK, 255);

        MakeEmptyBox({1, 16, 40}, {12, 12, 12}, 2, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1); //Open area
        MakeEmptyBox({3, 28, 47}, {6, 5, 3}, 2, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1); //Tucked away (For goal)
        ClearRange({4, 28, 48}, {5, 2, 2}); //Open wall between open area and goal.
        PaintRange({12, 17, 40}, {1, 1, 1}, GOAL_BLOCK, 255);

        PaintRange({6, 33, 47}, {1, 1, 3}, GOAL_BLOCK, 255);

        ClearRange({4, 15, 48}, {1, 1, 1});
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
            MakeJumpSection({1, 16, 39}, {12, 12, 3}, 0, 0, 7, 0);
            MakeJumpSection({1, 16, 51}, {12, 12, 3}, 0, 0, -7, 0);
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

    Room4() : Room(100,{6.5, 31.5, 49},{5, 47, 43}, {1, 1, 2}) {
    }

    void init() {
        //Increasingly warped world.
        for (int x = 0; x < 14; x++) {
            SetWarpSpaceArea({2 + x, 34, 42}, {3, 15, 20}, {.8f / (14 - x), .8f / (14 - x), .4f});
        }
        //		SetWarpSpaceArea( 2, 34, 42, 12, 15, 20, .18, .18, .4 );    //very flat big box.
        //	SetWarpSpaceArea( 14, 34, 42, 8, 15, 20, .8, .8, .4 );    //very flat big box.

        MakeEmptyBox({3, 33, 43}, {15, 15, 16}, 2, 255, DEFAULT_BRIGHT, 1); //Main room.
        MakeEmptyBox({3, 28, 47}, {6, 5, 3}, 2, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1); //Tucked away (For start point)
        PaintRange({2, 32, 43 + 12 + 2}, {17, 17, 3}, SPAAACE_CELL, DEFAULT_DENSITY); //Space on ceiling.

        //Lava walls
        PaintRange({3, 35, 43}, {1, 10, 2}, 10, DEFAULT_DENSITY);
        PaintRange({18, 35, 43}, {1, 10, 2}, 10, DEFAULT_DENSITY);

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
        PaintRange({5, 47, 43}, {1, 1, 1}, GOAL_BLOCK, 255);
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

    Room5() : Room(15, {5.5, 47.5, 42},{5, 31, 31}, {1, 1, 2}) {
    }

    void init() {
        MakeEmptyBox({3, 45, 30}, {4, 4, 13}, 13, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1); //Start tube
        MakeEmptyBox({3, 30, 30}, {7, 15, 4}, 13, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1); //side tube
        ClearCell({5, 45, 31});
        ClearCell({5, 45, 32});

        //		SetWarpSpaceArea( 3, 32, 3, 12, 4, 3, .3, .3, .3 ); 
        SetWarpSpaceArea({3, 34, 30}, {3, 11, 6}, {1, .2, 1}); //very long box.
        SetWarpSpaceArea({7, 34, 30}, {3, 11, 6}, {1, 5, 1}); //very long box.

        //Target in floor.
        PaintRange({5, 30, 31}, {1, 1, 2}, GOAL_BLOCK, 255);
    }

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

    Room6() : Room(30, {5.5, 30.5, 32},{5, 15, 31}, {1, 2, 2}) {
    }

    void init() {
        MakeEmptyBox({3, 15, 30},{ 5, 15, 4}, 1, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1); //side tube

        PaintRange({3, 18, 30}, {5, 7, 1}, 10, 255); //lava.
        PaintRange({4, 27, 31}, {1, 3, 3}, 1, 120); //tube wall
        PaintRange({6, 27, 31}, {1, 3, 3}, 1, 120); //tube wall

        //Target on wall.
        PaintRange({5, 15, 31}, {1, 1, 2}, GOAL_BLOCK, 255);

        //Hide warp zone next to door.
        float farrot[] = {1, 0, 0, 0, 0, 1, 0, 1, 0};
        MakeJumpSection({7, 29, 31}, {2, 1, 3}, -2, 4, -15, farrot);
    }

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

    Room7() : Room(150, {5.5, 16.5, 32},{23, 7, 21}, {1, 1, 2}) {
    }

    void init() {
        //Mini waiting platform

        MakeEmptyBox({2, 2, 30}, {13, 13, 18}, 3, DEFAULT_DENSITY - 10, DEFAULT_BRIGHT, 1); //Main chamber
        PaintRange({3, 14, 30}, {5, 3, 1}, 1, 120);
        PaintRange({3, 15, 30}, {5, 1, 4}, 1, 120);

        //Warp from top of first to top of second.
        float farrot[] = {1, 0, 0, 0, -1, 0, 0, 0, -1};
        MakeJumpSection({2, 2, 41}, {13, 13, 1}, 0, -19, -65, farrot);

        //Adn vice versa
        MakeJumpSection({2, 2, 25}, {13, 13, 1}, 0, -19, -65, farrot);


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




        MakeEmptyBox({2, 2, 20}, {22, 13, 5}, 4, DEFAULT_DENSITY - 10, DEFAULT_BRIGHT, 1); //Goal Chamber
        ClearRange({14, 3, 19},{ 5, 12, 2});

        MakeEmptyBox({13, 2, 18}, {5, 13, 1}, 10, DEFAULT_DENSITY - 10, DEFAULT_BRIGHT, 1); //Lava pit


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

    Room8() : Room(200,{23, 7.5, 22},{16, 46, 54}, {2, 2, 2}) {
    };

    void init() {

        //Make sure room 7 is set up for us. (And room 4)
        rooms[7]->init();
        rooms[4]->init();
        PaintRange({3, 28, 48}, {6, 1, 2}, DEADGOAL_BLOCK, 255);
        PaintRange({5, 47, 43}, {1, 1, 1}, DEADGOAL_BLOCK, 255);
        PaintRange({6, 33, 48}, {1, 1, 2}, GOAL_BLOCK, 0);

        //But, block the exit.
        MakeJumpSection({25, 7, 21}, {3, 2, 3}, -19, 24, 27, 0);

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

        ClearCell({17, 48, 55});
        ClearCell({16, 48, 55});
        ClearCell({16, 48, 54});
        ClearCell({17, 48, 54});
        PaintRange({16, 48, 54},
        {
            2, 1, 2
        }, GOAL_BLOCK, 255);
        run();
    }

    void run() {
        int capden = 255 - timeIn * 200;

        if (capden < 0) {
            PaintRange({24, 7, 21},
            {
                1, 1, 2
            }, GOAL_BLOCK, 0);
        } else {
            PaintRange({24, 7, 21},
            {
                1, 1, 2
            }, GOAL_BLOCK, capden);
        }
    }
};

class Room9 : public Room {
public:

    Room9() : Room(200, {17.4, 49.8, 55.1},{4, 65, 48}, {2, 2, 2}) {
    }


    double TimeTransition = 0;
    char lifemap[16 * 16];
    char newlife[16 * 16];

    void init() {
        //Room 8's gotta be here.
        rooms[8]->init();
        PaintRange({15, 45, 53},
        {
            3, 3, 1
        }, 9, 130);


        //Little tunnel off end.
        //		gPositionX = 17.4;
        //		gPositionY = 46.8;
        //		gPositionZ = 55.1;

        int i = 1;
        for (int x = 0; x < 16; x++)
            for (int y = 0; y < 16; y++) {
                i = (i * 2089) % 491;
                if (((i % 7) > 2)) {
                    newlife[x + y * 16] = 1;
                }
            }


        //Draw whatever tubes and all.
        MakeEmptyBox({2, 49, 46},
        {
            17, 17, 10
        }, 3, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1);
        ClearRange({16, 49, 54},
        {
            2, 1, 2
        });
        PaintRange({2, 49, 46},
        {
            17, 16, 1
        }, 10, 60);

        PaintRange({4, 66, 47},
        {
            2, 1, 3
        },
        GOAL_BLOCK, 255);
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

    Room10() : Room(200, {5.1, 65.2, 48},{0,0,0},{-1,-1,-1}) {
    }

    void init() {
        rooms[9]->init();

        MakeEmptyBox({2, 67, 46}, {6, 10, 10}, 4, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1);
        ClearRange({4, 67, 47}, {2, 1, 3});
        SetWarpSpaceArea({2, 67, 46}, {10, 10, 10}, {.3, .3, 1}); //very big box.
        SetWarpSpaceArea({7, 67, 46 + 5}, {2, 10, 1}, {.3, 1.5, .5}); //very big box.
        PaintRange({3, 70, 46}, {10, 3, 1}, 10, DEFAULT_DENSITY);

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
    new Room0,
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

void GameMap::Update() {
    static int was_level_change_pressed;
    int i;

    if (gOverallUpdateNo == 0) {
        room = START_ROOM;
        rooms[room]->begin();
    }

    if (room != lastroom) {
        rooms[room]->init();
        lastroom = room;
    }

    gDaytime += worldDeltaTime;
    GameTimer -= worldDeltaTime;
    if (GameTimer < 0) {
        Die();
    }

    rooms[room]->timeIn += worldDeltaTime;
    rooms[room]->update();

    sprintf(gDialog, "Room: %d\n", room);

    if (gKeyMap['l'] || gKeyMap['L']) {
        for (i = 0; i < NR_ROOMS; i++) {
            rooms[i]->init();
        }
    }

    UpdatePickableBlocks(); //Draw the pickables.

    //Press 'r' to reset room.
    if (gKeyMap['r'] || gKeyMap['R']) {
        rooms[room]->begin();
    }

    if (gKeyMap['='] || gKeyMap['+']) {
        if (!was_level_change_pressed)
            room++;
        was_level_change_pressed = 1;
        rooms[room]->begin();
    } else if (gKeyMap['-'] || gKeyMap['_']) {
        if (!was_level_change_pressed)
            room--;
        rooms[room]->begin();
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