#include "GameMap.h"
#include <cstring>

extern GameMap gamemap;

void Room::reset() {
    gPosition = start;
    GameTimer = maxTime;
    begin();
}
void Room::begin() {
    for(auto& initfn:inits) initfn();
}

void Room::update() {
    timeIn+=worldDeltaTime;
    run();
    for(auto& runfn:runs) runfn(timeIn);

    if (IsPlayerInRange(exitr1, exitr2)) {
        gamemap.curroom++;
    }
}

void Room1::init() {
    for (int i = 0; i < 10; i++) {
        float den = (i + 1.0) / 12.0;
        if (den < .1) den = .1;
        SetWarpSpaceArea({2, 2, 59 - i}, {4, 4, 1},{ den, den, den}); //very big box
    }
}

void Room1::run() {
    //Make cute breathe
    MakeEmptyBox({2, 2, 48}, {4, 4, 11}, 4, swoovey(timeIn * .1, 1)*70 + 185, DEFAULT_BRIGHT, 1); //Chute
    ClearCell({4, 4, 60});
    ClearCell({4, 4, 59});
    //Make hole in side.
    ClearRange({3, 6, 51}, {3, 1, 2});
}

void Room2::init() {
    stage = 0;
}

void Room2::run() {
    gDaytime = gDaytime * .99 + 400 * .01;

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

void Room3::init() {
    already_removed_stretch = 0;
    already_setup_jumpspace = 0;
}

void Room3::run() {
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
}

void Room4::init() {
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
                gamemap.AddDeathBlock({3 + x, 35 + y, 43});
            }
        }
    //Target in floor.
}

void Room4::run() {
    for (int x = 0; x < 14; x++)
        for (int y = 0; y < 14; y++) {
            QuickCell(0, {4 + x, 34 + y, 43 + 10}, 1, DEFAULT_BRIGHT, (sin(x * 2 + y + timeIn)*80 + 80), 9);
        }
    UpdateZone({3, 33, 43 + 10},{ 15, 15, 3});
}

void Room6::run() {
    if (IsPlayerInRange({3, 18, 30}, {5, 7, 4})) {
        gamemap.die();
    }
}

void Room7::init() {
    //Put a little mound of pickables over in the corner.
    pickables_in_inventory = 0;
    gamemap.ClearPickableBlocks();
    int x, y, z;
    for (x = 0; x < 3; x++) {
        for (y = 0; y < 3; y++) {
            for (z = 0; z < 2; z++) {
                gamemap.PlacePickableAt({x + 5, y + 5, z + 31}, -(x + y) - z * 2.0 - 2);
            }
        }
    }

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
                gamemap.AddDeathBlock({4 + x, 4 + y, 20});
            }
        }
    //Goal
    PaintRange({24, 7, 21}, {1, 1, 2}, GOAL_BLOCK, 255);

}

void Room7::run() {
    if (IsPlayerInRange({13, 1, 18}, {7, 15, 3})) {
        gamemap.die();
    }
}

void Room8::init() {
    //Place some goodies in here.
    pickables_in_inventory = 0;
    gamemap.ClearPickableBlocks();
    for (int x = 0; x < 2; x++) {
        for (int y = 0; y < 4; y++) {
            for (int z = 0; z < 2; z++) {
                gamemap.PlacePickableAt({x + 6, y + 29, z + 48}, -(x + y)*.5 - z * .5);
            }
        }
    }
}

void Room9::init() {
    //Room 8's gotta be here.
    int i = 10;
    for (int x = 0; x < 16; x++)
        for (int y = 0; y < 16; y++) {
            i = (i * 2089) % 491;
            if (((i % 7) > 2)) {
                newlife[x + y * 16] = 1;
            }
        }
}

void Room9::run() {
    //Fix life map up.
    if (TimeTransition >= 1) {
        memcpy(lifemap, newlife, sizeof (lifemap));
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
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {

            double den = ((double) lifemap[x + y * 16]) * (1. - TimeTransition) + ((double) newlife[x + y * 16]) * TimeTransition;
            QuickCell(0, {3 + x, 50 + y, 47}, 1, DEFAULT_BRIGHT, den*255, 5);
        }
    }
    UpdateZone({3,50,47}, {19, 66, 47});
    TimeTransition += worldDeltaTime;

    if (gPosition.z < 47.8) {
        gamemap.die();
    }
}

void Room10::run() {
    for (int z = 46; z < 57; z++)
        for (int y = 67; y < 77; y++) {
            if ((y + z)&1) continue;
            QuickCell(0,{2, y, z}, 1, DEFAULT_BRIGHT, sin(timeIn * 10.0 + z + y)*50.5 + 160, 5);
        }
    UpdateZone({1, 66, 46}, {1, 10, 10});

    for (int z = 46; z < 57; z++)
        for (int x = 2; x < 12; x++) {
            if ((x + z)&1) continue;
            QuickCell(0,{x, 77, z}, 1, DEFAULT_BRIGHT, sin(timeIn * 10.0 + x + z)*50.5 + 160, 5);
        }
    UpdateZone({1, 76, 46},{10, 1, 10});

    for (int y = 67; y < 77; y++)
        for (int x = 2; x < 12; x++) {
            if ((y + x)&1) continue;
            QuickCell(0,{x, y, 56}, 1, DEFAULT_BRIGHT, sin(timeIn * 10.0 + x + y)*50.5 + 160, 5);
        }
    UpdateZone({2, 67, 56}, {10, 10, 1});

    if (IsPlayerInRange({3, 70, 46}, {6, 3, 2.1})) {
        gamemap.die();
    }

}

void Room11::run() {
    //float ang = timeIn * 0.1;
    //MakeJumpSection({3, 67, 37}, {3, 1, 3}, {0, 5, 0},{1, 0, 0},{0, cos(ang), -sin(ang)},{0.1, sin(ang), cos(ang)});
}