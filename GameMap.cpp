/* 
 * File:   GameMap.cpp
 * Author: tehdog
 * 
 * Created on 19. Oktober 2014, 02:26
 */

#include "scripthelpers.h"
#include "GameMap.h"
#include <cstring>
#define START_ROOM 6
#define NR_ROOMS 11 //RunRoom is 0 indexed, this should be one greater than the highest numbered room.
int firstrun = 0;
int lastroom = -1;
int room = START_ROOM;

void StartAtRoom(int rid);
void RunRoom0();
void RunRoom1();
void RunRoom2();
void RunRoom3();
void RunRoom4();
void RunRoom5();
void RunRoom6();
void RunRoom7();
void RunRoom8();
void RunRoom9();
void RunRoom10();

void Die() {
    StartAtRoom(room);
    GameAttempt++;
}


//Only called once.

void initialize() {
    printf("Initialize.\n");

    StartAtRoom(START_ROOM);
}

void start(void * id) {
    printf("Start %d\n", gOverallUpdateNo);
}

void stop(void * id) {
    printf("Stop\n");
}

void RunRoom10() {
    int x, y, z;
    static double TimeInRoom;
    TimeInRoom += worldDeltaTime;
    int capden = 255 - TimeInRoom * 200;
    float warp = ((TimeInRoom * .1) > 1) ? 1 : ((TimeInRoom * .1) + .1);
    if (firstrun) {
        RunRoom9();

        TimeInRoom = 0;
        GameTimer = 200;
        firstrun = 0;

        MakeEmptyBox(2, 67, 46, 6, 10, 10, 4, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1);
        ClearRange(4, 67, 47, 2, 1, 3);
        SetWarpSpaceArea(2, 67, 46, 10, 10, 10, .3, .3, 1); //very big box.
        SetWarpSpaceArea(7, 67, 46 + 5, 2, 10, 1, .3, 1.5, .5); //very big box.
        PaintRange(3, 70, 46, 10, 3, 1, 10, DEFAULT_DENSITY);

    }

    for (z = 46; z < 57; z++)
        for (y = 67; y < 77; y++) {
            if ((y + z)&1) continue;
            QuickCell(0, 2, y, z, 1, DEFAULT_BRIGHT, sin(TimeInRoom * 10.0 + z + y)*50.5 + 160, 5);
        }
    UpdateZone(1, 66, 46, 1, 10, 10);

    for (z = 46; z < 57; z++)
        for (x = 2; x < 12; x++) {
            if ((x + z)&1) continue;
            QuickCell(0, x, 77, z, 1, DEFAULT_BRIGHT, sin(TimeInRoom * 10.0 + x + z)*50.5 + 160, 5);
        }
    UpdateZone(1, 76, 46, 10, 1, 10);

    for (y = 67; y < 77; y++)
        for (x = 2; x < 12; x++) {
            if ((y + x)&1) continue;
            QuickCell(0, x, y, 56, 1, DEFAULT_BRIGHT, sin(TimeInRoom * 10.0 + x + y)*50.5 + 160, 5);
        }
    UpdateZone(2, 67, 56, 10, 10, 1);


    if (capden < 0) {
        PaintRange(4, 66, 47, 2, 1, 3, GOAL_BLOCK, 0);
    } else {
        PaintRange(4, 66, 47, 2, 1, 3, GOAL_BLOCK, capden);
    }

    if (IsPlayerInRange(3, 70, 46, 6, 3, 2.1)) {
        Die();
    }

}

void RunRoom9() {
    static double TimeInRoom;
    TimeInRoom += worldDeltaTime;
    int capden = 255 - TimeInRoom * 200;
    float warp = ((TimeInRoom * .1) > 1) ? 1 : ((TimeInRoom * .1) + .1);

    static double TimeTransition = 0;
    static char lifemap[16 * 16];
    static char newlife[16 * 16];
    int x, y;

    if (firstrun) {

        TimeInRoom = 0;
        //Room 8's gotta be here.
        firstrun = 1;
        RunRoom8();
        PaintRange(15, 45, 53, 3, 3, 1, 9, 130);


        //Little tunnel off end.

        GameTimer = 200;
        firstrun = 0;

        //		gPositionX = 17.4;
        //		gPositionY = 46.8;
        //		gPositionZ = 55.1;

        int i = 1;
        for (x = 0; x < 16; x++)
            for (y = 0; y < 16; y++) {
                i = (i * 2089) % 491;
                if (((i % 7) > 2)) {
                    newlife[x + y * 16] = 1;
                }
            }


        //Draw whatever tubes and all.
        MakeEmptyBox(2, 49, 46, 17, 17, 10, 3, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1);
        ClearRange(16, 49, 54, 2, 1, 2);
        PaintRange(2, 49, 46, 17, 16, 1, 10, 60);

        PaintRange(4, 66, 47, 2, 1, 3, GOAL_BLOCK, 255);
    }

    //Fix life map up.
    if (TimeTransition >= 1) {
        memcpy(lifemap, newlife, sizeof ( lifemap));
        for (x = 0; x < 16; x++) {
            for (y = 0; y < 16; y++) {
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

    for (x = 0; x < 16; x++)
        for (y = 0; y < 16; y++) {
            double den = ((double) lifemap[x + y * 16]) * (1. - TimeTransition) + ((double) newlife[x + y * 16]) * TimeTransition;
            ChangeCell(0, 3 + x, 50 + y, 47, 1, DEFAULT_BRIGHT, den * 255, 5);
        }
    TimeTransition += worldDeltaTime;

    if (capden < 0) {
        PaintRange(16, 48, 54, 2, 1, 2, GOAL_BLOCK, 0);
    } else {
        PaintRange(16, 48, 54, 2, 1, 2, GOAL_BLOCK, capden);
    }

    if (IsPlayerInRange(4, 65, 48, 2, 2, 2)) {
        room = 10;
    }

    if (gPositionZ < 47.8) {
        Die();
    }
}

void GameMap::collision(struct CollisionProbe * ddat) {
    ///XXX TODO: pointers for where we're aimed should come in here.
    //printf( "CC %f %f %f  (%f)\n", ddat->TargetLocation.r, ddat->TargetLocation.g, ddat->TargetLocation.b, ddat->Normal.a ); 
}

void UpdateRoom(int rid) {
    switch (rid) {
        case 0: RunRoom0();
            break;
        case 1: RunRoom1();
            break;
        case 2: RunRoom2();
            break;
        case 3: RunRoom3();
            break;
        case 4: RunRoom4();
            break;
        case 5: RunRoom5();
            break;
        case 6: RunRoom6();
            break;
        case 7: RunRoom7();
            break;
        case 8: RunRoom8();
            break;
        case 9: RunRoom9();
            break;
        case 10: RunRoom10();
            break;
    }
}

void RunRoom0() {
    if (firstrun) {
        int i;
        GameTimer = 100;
        firstrun = 0;
        MakeEmptyBox(1, 1, 60, 6, 6, 6, WALL_BLOCK, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1); //White box,
        SetWarpSpaceArea(1, 1, 60, 6, 6, 6, .1, .1, .1); //very big box.
        ChangeCell(0, 4, 4, 60, 1, DEFAULT_BRIGHT, 255, GOAL_BLOCK);

        ChangeCell(0, 2, 4, 62, 1, DEFAULT_BRIGHT, 255, 15);
        ChangeCell(0, 4, 4, 62, 1, DEFAULT_BRIGHT, 255, 18);
        ChangeCell(0, 3, 2, 61, 1, DEFAULT_BRIGHT, 255, 35);
        ChangeCell(0, 5, 2, 63, 1, DEFAULT_BRIGHT, 255, 50);
        ChangeCell(0, 3, 5, 64, 1, DEFAULT_BRIGHT, 255, 81);
        ChangeCell(0, 5, 5, 63, 1, DEFAULT_BRIGHT, 255, 22);


        //Util: Make reference swatches.
        for (i = 0; i < 256; i++) {
            ChangeCell(0, 2 + (i % 16), 40 + (i / 16), 63, 1, DEFAULT_BRIGHT, 255, i);
        }
    }

    if (IsPlayerInRange(4, 4, 61, 1, 1, 1)) {
        room = 1;
    }
}

void RunRoom1() {
    static double TimeInRoom1;
    TimeInRoom1 += worldDeltaTime;

    if (firstrun) {
        GameTimer = 100;
        firstrun = 0;
        TimeInRoom1 = 0.0;

        //Box off to the side
        MakeEmptyBox(2, 6, 50, 4, 4, 3, 7, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1);


        MakeEmptyBox(2, 2, 48, 4, 4, 11, 4, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1); //Chute
        ClearCell(4, 4, 60);
        ClearCell(4, 4, 59);
        int i;
        for (i = 0; i < 10; i++) {
            float den = (i + 1.0) / 12.0;
            if (den < .1) den = .1;
            SetWarpSpaceArea(2, 2, 59 - i, 4, 4, 1, den, den, den); //very big box
        }
        //Make hole in side.
        ClearRange(3, 6, 51, 3, 1, 2);

        //ChangeCell( 0, 4, 10, 52, 1, DEFAULT_BRIGHT, 255, GOAL_BLOCK );
        PaintRange(4, 10, 50, 1, 1, 3, GOAL_BLOCK, 255);
    }

    //	printf( "%f %f\n", triangle(TimeInRoom1, .9) );
    //Make cute breathe
    MakeEmptyBox(2, 2, 48, 4, 4, 11, 4, swoovey(TimeInRoom1 * .1, 1)*70 + 185, DEFAULT_BRIGHT, 1); //Chute
    ClearCell(4, 4, 60);
    ClearCell(4, 4, 59);
    //Make hole in side.
    ClearRange(3, 6, 51, 3, 1, 2);


    int capden = 255 - TimeInRoom1 * 200;
    float warp = ((TimeInRoom1 * .1) > 1) ? 1 : ((TimeInRoom1 * .1) + .1);

    if (capden < 0) {
        ChangeCell(0, 4, 4, 60, 0, DEFAULT_BRIGHT, 0, GOAL_BLOCK);
    } else {
        ChangeCell(0, 4, 4, 60, 1, DEFAULT_BRIGHT, capden, GOAL_BLOCK);
    }

    if (IsPlayerInRange(3, 9, 51, 3, 1, 2)) {
        room = 2;
    }
}

void RunRoom2() {
    static int StageInRoom2;
    static double TimeInRoom2;
    TimeInRoom2 += worldDeltaTime;

    gDaytime = gDaytime * .99 + 400 * .01;
    if (firstrun) {
        GameTimer = 100;
        StageInRoom2 = 0;
        TimeInRoom2 = 0;
        firstrun = 0;

        MakeEmptyBox(2, 11, 47, 4, 4, 10, SPAAACE_CELL, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1); //Open area
        SetWarpSpaceArea(2, 11, 46, 4, 4, 3, .3, .3, .3); //very big box

        //Clear the door in.
        PaintRange(3, 11, 47, 3, 1, 8, GOAL_BLOCK, 255);
        ClearRange(4, 11, 51, 1, 1, 2);
    }


    int capden = 255 - TimeInRoom2 * 200;
    float warp = ((TimeInRoom2 * .1) > 1) ? 1 : ((TimeInRoom2 * .1) + .1);

    if (capden < 0) {
        PaintRange(4, 10, 51, 1, 1, 2, GOAL_BLOCK, 0);
    } else {
        PaintRange(4, 10, 51, 1, 1, 2, GOAL_BLOCK, capden);
    }

    //Force user to look around to get out.
    switch (StageInRoom2) {
        case 0:
            if (gDirectionY > .7 && IsPlayerInRange(3, 12, 48, 3, 3, 3)) StageInRoom2 = 1;
            break;
        case 1:
            if (gDirectionY < -.7) StageInRoom2 = 2;
            break;
        case 2:
            if (gDirectionY > .7) StageInRoom2 = 3;
            break;
        case 3:
            PaintRange(3, 11, 47, 3, 1, 8, SPAAACE_CELL, 255);
            if (gDirectionY < -.7) StageInRoom2 = 4;
            break;
        case 4:
            //Open hole in front of room.
            PaintRange(3, 15, 47, 3, 2, 3, WHITE_BLOCK, 255);
            ClearRange(4, 15, 48, 1, 1, 1);
            PaintRange(4, 16, 48, 1, 1, 1, GOAL_BLOCK, 255);
            SetWarpSpaceArea(3, 15, 47, 4, 4, 3, .3, .3, .3);
            if (gDirectionY < -.7) StageInRoom2 = 4;
            break;

    }
    //printf( "%f %f %f %d\n", gDirectionX, gDirectionY, gDirectionZ, StageInRoom2 );

    if (IsPlayerInRange(3, 15.1, 48, 3, 1, 2)) {
        room = 3;
    }
}

//Room with echo walls.

void RunRoom3() {
    static int already_setup_jumpspace;
    static int already_removed_stretch = 0;
    static double TimeInRoom3;
    TimeInRoom3 += worldDeltaTime;
    gDaytime = gDaytime * .999 + 472 * .001;

    if (IsPlayerInRange(2, 17, 41, 10, 10, 1.1) && !already_removed_stretch) {
        printf("Dewarpify.\n");
        //Remove the stretchyness that let us into this room.
        SetWarpSpaceArea(1, 15, 40, 12, 12, 12, 1, 1, 1);

        PaintRange(4, 16, 48, 1, 1, 1, DEADGOAL_BLOCK, 255);
        already_removed_stretch = 1;
    }

    if (firstrun) {
        GameTimer = 100;
        already_removed_stretch = 0;
        already_setup_jumpspace = 0;
        TimeInRoom3 = 0;
        firstrun = 0;
        SetWarpSpaceArea(1, 15, 40, 12, 12, 12, .3, .3, .3);
        PaintRange(3, 15, 47, 3, 2, 3, WHITE_BLOCK, 255);

        MakeEmptyBox(1, 16, 40, 12, 12, 12, 2, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1); //Open area
        MakeEmptyBox(3, 28, 47, 6, 5, 3, 2, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1); //Tucked away (For goal)
        ClearRange(4, 28, 48, 5, 2, 2); //Open wall between open area and goal.
        //		MakeJumpSection( 1, 16, 39, 12, 12, 3, 0, 0, 0 );
        //		MakeJumpSection( 1, 16, 51, 12, 12, 3, 0, 0, 0 );
        PaintRange(12, 17, 40, 1, 1, 1, GOAL_BLOCK, 255);

        PaintRange(6, 33, 47, 1, 1, 3, GOAL_BLOCK, 255);

        ClearRange(4, 15, 48, 1, 1, 1);
    }

    if (IsPlayerInRange(12, 17, 41, 1, 1, 2) && already_setup_jumpspace == 0) {
        ClearRange(1, 16, 37, 1, 1, 4);
        ClearRange(1, 16, 48, 1, 1, 4);
        MakeJumpSection(1, 16, 39, 12, 12, 3, 0, 0, 7, 0);
        MakeJumpSection(1, 16, 51, 12, 12, 3, 0, 0, -7, 0);
        printf("Jumpspace\n");
        already_setup_jumpspace = 1;
    }

    int capden = 255 - TimeInRoom3 * 200;
    float warp = ((TimeInRoom3 * .1) > 1) ? 1 : ((TimeInRoom3 * .1) + .1);

    int i;
    //Make shifting platform.
    for (i = 2; i < 13; i++) {
        float swoove = swoovey(TimeInRoom3 * .1, 3.0) + .5;
        swoove = (swoove * 13.0 - i)*.4;
        float swd = 1. - swoove*swoove;
        if (swd < 0) swd = 0;
        if (swd > 1) swd = 1;
        swd = pow(swd, .5);
        PaintRange(i, 21, 46, 1, 2, 1, 17, swd * 255); //Random little platform
    }

    if (capden < 0) {
        if (already_removed_stretch == 0)
            PaintRange(4, 16, 48, 1, 1, 1, GOAL_BLOCK, 0);
    } else {
        PaintRange(4, 16, 48, 1, 1, 1, GOAL_BLOCK, capden);
    }

    if (IsPlayerInRange(6, 32, 48, 1, 1, 2)) {
        room = 4;
    }
}

void RunRoom4() {
    int x, y;

    //Make evil deadly floor room.
    static double TimeInRoom4;
    TimeInRoom4 += worldDeltaTime;

    if (firstrun) {
        GameTimer = 100;

        //Increasingly warped world.
        for (x = 0; x < 14; x++) {
            SetWarpSpaceArea(2 + x, 34, 42, 3, 15, 20, .8 / (float) (14 - x), .8 / (float) (14 - x), .4);
        }
        //		SetWarpSpaceArea( 2, 34, 42, 12, 15, 20, .18, .18, .4 );    //very flat big box.
        //	SetWarpSpaceArea( 14, 34, 42, 8, 15, 20, .8, .8, .4 );    //very flat big box.

        MakeEmptyBox(3, 33, 43, 15, 15, 16, 2, 255, DEFAULT_BRIGHT, 1); //Main room.
        MakeEmptyBox(3, 28, 47, 6, 5, 3, 2, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1); //Tucked away (For start point)
        PaintRange(2, 32, 43 + 12 + 2, 17, 17, 3, SPAAACE_CELL, DEFAULT_DENSITY); //Space on ceiling.

        //Lava walls
        PaintRange(3, 35, 43, 1, 10, 2, 10, DEFAULT_DENSITY);
        PaintRange(18, 35, 43, 1, 10, 2, 10, DEFAULT_DENSITY);

        //Fill some lava in in a random pattern.
        unsigned long i = 1;
        for (x = 0; x < 15; x++)
            for (y = 0; y < 11; y++) {
                i = (i * 2089) % 491;
                if (((i % 7) > 1)) {
                    QuickCell(0, 3 + x, 35 + y, 43, 1, DEFAULT_BRIGHT, 60, 10); //Lava
                    AddDeathBlock(3 + x, 35 + y, 43);
                }
            }

        TimeInRoom4 = 0;
        firstrun = 0;


        //Target in floor.
        PaintRange(5, 47, 43, 1, 1, 1, GOAL_BLOCK, 255);
    }

    int capden = 255 - TimeInRoom4 * 200;
    float warp = ((TimeInRoom4 * .1) > 1) ? 1 : ((TimeInRoom4 * .1) + .1);

    //Open Room 3 goal to room 4.
    if (capden < 0) {
        PaintRange(6, 33, 48, 1, 1, 2, GOAL_BLOCK, 0);
        PaintRange(3, 28, 48, 6, 1, 2, DEADGOAL_BLOCK, 255);
    } else {
        PaintRange(6, 33, 48, 1, 1, 2, GOAL_BLOCK, capden);
        PaintRange(3, 28, 48, 6, 1, 2, DEADGOAL_BLOCK, 255 - capden);
    }

    for (x = 0; x < 14; x++)
        for (y = 0; y < 14; y++) {
            QuickCell(0, 4 + x, 34 + y, 43 + 10, 1, DEFAULT_BRIGHT, (sin(x * 2 + y + TimeInRoom4)*80 + 80), 9);
        }
    UpdateZone(3, 33, 43 + 10, 15, 15, 3);

    if (IsPlayerInRange(5, 47, 43, 1, 1, 2)) {
        room = 5;
    }

}

void RunRoom5() {
    static double TimeInRoom5;
    TimeInRoom5 += worldDeltaTime;
    int capden = 255 - TimeInRoom5 * 200;
    float warp = ((TimeInRoom5 * .1) > 1) ? 1 : ((TimeInRoom5 * .1) + .1);

    if (firstrun) {
        MakeEmptyBox(3, 45, 30, 4, 4, 13, 13, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1); //Start tube
        MakeEmptyBox(3, 30, 30, 7, 15, 4, 13, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1); //side tube
        ClearCell(5, 45, 31);
        ClearCell(5, 45, 32);

        //		SetWarpSpaceArea( 3, 32, 3, 12, 4, 3, .3, .3, .3 ); 
        SetWarpSpaceArea(3, 34, 30, 3, 11, 6, 1, .2, 1); //very long box.
        SetWarpSpaceArea(7, 34, 30, 3, 11, 6, 1, 5, 1); //very long box.

        //Target in floor.
        PaintRange(5, 30, 31, 1, 1, 2, GOAL_BLOCK, 255);

        GameTimer = 15;
        firstrun = 0;
    }

    //Open Room 3 goal to room 4.
    if (capden < 0) {
        PaintRange(5, 47, 43, 1, 1, 1, GOAL_BLOCK, 0);
    } else {
        PaintRange(5, 47, 43, 1, 1, 1, GOAL_BLOCK, capden);
    }

    if (IsPlayerInRange(5, 31, 31, 1, 1, 2)) {
        room = 6;
    }


}

void RunRoom6() {
    static double TimeInRoom;
    TimeInRoom += worldDeltaTime;
    int capden = 255 - TimeInRoom * 200;
    float warp = ((TimeInRoom * .1) > 1) ? 1 : ((TimeInRoom * .1) + .1);

    if (firstrun) {
        MakeEmptyBox(3, 15, 30, 5, 15, 4, 1, DEFAULT_DENSITY, DEFAULT_BRIGHT, 1); //side tube

        PaintRange(3, 18, 30, 5, 7, 1, 10, 255); //lava.
        PaintRange(4, 27, 31, 1, 3, 3, 1, 120); //tube wall
        PaintRange(6, 27, 31, 1, 3, 3, 1, 120); //tube wall

        //Target on wall.
        PaintRange(5, 15, 31, 1, 1, 2, GOAL_BLOCK, 255);

        //Hide warp zone next to door.
        float farrot[] = {1, 0, 0, 0, 0, 1, 0, 1, 0};
        MakeJumpSection(7, 29, 31, 2, 1, 3, -2, 4, -15, farrot);

        GameTimer = 30;
        firstrun = 0;
    }

    if (capden < 0) {
        PaintRange(5, 30, 31, 1, 1, 2, GOAL_BLOCK, 0);
    } else {
        PaintRange(5, 30, 31, 1, 1, 2, GOAL_BLOCK, capden);
    }

    if (IsPlayerInRange(3, 18, 30, 5, 7, 4)) {
        Die();
    }

    if (IsPlayerInRange(5, 15, 31, 1, 2, 2)) {
        room = 7;
    }
}

void RunRoom7() {
    static double TimeInRoom;
    TimeInRoom += worldDeltaTime;
    int capden = 255 - TimeInRoom * 200;
    float warp = ((TimeInRoom * .1) > 1) ? 1 : ((TimeInRoom * .1) + .1);

    if (firstrun) {
        //Mini waiting platform

        MakeEmptyBox(2, 2, 30, 13, 13, 18, 3, DEFAULT_DENSITY - 10, DEFAULT_BRIGHT, 1); //Main chamber
        PaintRange(3, 14, 30, 5, 3, 1, 1, 120);
        PaintRange(3, 15, 30, 5, 1, 4, 1, 120);

        //Warp from top of first to top of second.
        float farrot[] = {1, 0, 0, 0, -1, 0, 0, 0, -1};
        MakeJumpSection(2, 2, 41, 13, 13, 1, 0, -19, -65, farrot);

        //Adn vice versa
        float farrot2[] = {1, 0, 0, 0, -1, 0, 0, 0, -1};
        MakeJumpSection(2, 2, 25, 13, 13, 1, 0, -19, -65, farrot);


        //Put a little mound of pickables over in the corner.
        pickables_in_inventory = 0;
        ClearPicableBlocks();
        int x, y, z;
        for (x = 0; x < 3; x++) {
            for (y = 0; y < 3; y++) {
                for (z = 0; z < 2; z++) {
                    PlacePickableAt(x + 5, y + 5, z + 31, -(x + y) - z * 2.0 - 2);
                }
            }
        }

        ClickCellCB = PickableClick;




        MakeEmptyBox(2, 2, 20, 22, 13, 5, 4, DEFAULT_DENSITY - 10, DEFAULT_BRIGHT, 1); //Goal Chamber
        ClearRange(14, 3, 19, 5, 12, 2);

        MakeEmptyBox(13, 2, 18, 5, 13, 1, 10, DEFAULT_DENSITY - 10, DEFAULT_BRIGHT, 1); //Lava pit


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
                    QuickCell(0, 4 + x, 4 + y, 20, 1, DEFAULT_BRIGHT, 60, 10); //Lava
                    AddDeathBlock(4 + x, 4 + y, 20);
                }
            }


        GameTimer = 150;
        firstrun = 0;


        //Goal
        PaintRange(24, 7, 21, 1, 1, 2, GOAL_BLOCK, 255);

    }

    if (capden < 0) {
        PaintRange(5, 15, 31, 1, 1, 2, GOAL_BLOCK, 0);
    } else {
        PaintRange(5, 15, 31, 1, 1, 2, GOAL_BLOCK, capden);
    }

    if (IsPlayerInRange(13, 1, 18, 7, 15, 3)) {
        Die();
    }

    if (IsPlayerInRange(23, 7, 21, 1, 1, 2)) {
        room = 8;
    }
}

void RunRoom8() {
    static double TimeInRoom;
    TimeInRoom += worldDeltaTime;
    int capden = 255 - TimeInRoom * 200;
    float warp = ((TimeInRoom * .1) > 1) ? 1 : ((TimeInRoom * .1) + .1);

    if (firstrun) {
        TimeInRoom = 0;

        //Make sure room 7 is set up for us. (And room 4)
        firstrun = 1;
        RunRoom7();
        firstrun = 1;
        RunRoom4();
        PaintRange(3, 28, 48, 6, 1, 2, DEADGOAL_BLOCK, 255);
        PaintRange(5, 47, 43, 1, 1, 1, DEADGOAL_BLOCK, 255);
        PaintRange(6, 33, 48, 1, 1, 2, GOAL_BLOCK, 0);

        //But, block the exit.
        MakeJumpSection(25, 7, 21, 3, 2, 3, -19, 24, 27, 0);

        //Place some goodies in here.
        pickables_in_inventory = 0;
        ClearPicableBlocks();
        int x, y, z;
        for (x = 0; x < 2; x++) {
            for (y = 0; y < 4; y++) {
                for (z = 0; z < 2; z++) {
                    PlacePickableAt(x + 6, y + 29, z + 48, -(x + y)*.5 - z * .5);
                }
            }
        }

        GameTimer = 200;
        firstrun = 0;

        ClearCell(17, 48, 55);
        ClearCell(16, 48, 55);
        ClearCell(16, 48, 54);
        ClearCell(17, 48, 54);
        PaintRange(16, 48, 54, 2, 1, 2, GOAL_BLOCK, 255);
    }

    if (capden < 0) {
        PaintRange(24, 7, 21, 1, 1, 2, GOAL_BLOCK, 0);
    } else {
        PaintRange(24, 7, 21, 1, 1, 2, GOAL_BLOCK, capden);
    }

    if (IsPlayerInRange(16, 46, 54, 2, 2, 2)) {
        room = 9;
    }
}

void StartAtRoom(int rid) {
    firstrun = 1;
    room = rid;
    switch (rid) {
        case 0:
            GameAttempt = 1;
            gPositionX = 3;
            gPositionY = 3;
            gPositionZ = 64;
            break;
        case 1:
            gPositionX = 4.28;
            gPositionY = 4.27;
            gPositionZ = 59.13;
            break;
        case 2:
            gPositionX = 4.5;
            gPositionY = 7.92;
            gPositionZ = 51.92;
            break;
        case 3:
            gPositionX = 4.5;
            gPositionY = 15.29;
            gPositionZ = 48.44;
            break;
        case 4:
            gPositionX = 6.5;
            gPositionY = 31.5;
            gPositionZ = 49;
            break;
        case 5:
            gPositionX = 5.5;
            gPositionY = 47.5;
            gPositionZ = 42;
            break;
        case 6:
            gPositionX = 5.5;
            gPositionY = 30.5;
            gPositionZ = 32;
            break;
        case 7:
            gPositionX = 5.5;
            gPositionY = 16.5;
            gPositionZ = 32;
            break;
        case 8:
            gPositionX = 23;
            gPositionY = 7.5;
            gPositionZ = 22;
            break;
        case 9:
            gPositionX = 17.4;
            gPositionY = 49.8;
            gPositionZ = 55.1;
            break;
        case 10:
            gPositionX = 5.1;
            gPositionY = 65.2;
            gPositionZ = 48;
            break;
    }
}

GameMap::GameMap() {
}

GameMap::GameMap(const GameMap& orig) {
}

GameMap::~GameMap() {
}

void GameMap::Update() {
    static int was_level_change_pressed;
    int i;

    if (gOverallUpdateNo == 0) {
        initialize();
    }

    if (room != lastroom) {
        firstrun = 1;
        lastroom = room;
    }

    gDaytime += worldDeltaTime;
    GameTimer -= worldDeltaTime;
    if (GameTimer < 0) {
        Die();
    }

    UpdateRoom(room);

    sprintf(gDialog, "Room: %d\n", room);

    if (gKeyMap['l'] || gKeyMap['L']) {
        for (i = 0; i < NR_ROOMS; i++) {
            StartAtRoom(i);
            UpdateRoom(i);
        }
    }

    UpdatePickableBlocks(); //Draw the pickables.

    //Press 'r' to reset room.
    if (gKeyMap['r'] || gKeyMap['R']) {
        StartAtRoom(room);
    }

    if (gKeyMap['='] || gKeyMap['+']) {
        if (!was_level_change_pressed)
            room++;
        was_level_change_pressed = 1;
        StartAtRoom(room);
    } else if (gKeyMap['-'] || gKeyMap['_']) {
        if (!was_level_change_pressed)
            room--;
        was_level_change_pressed = 1;
        StartAtRoom(room);
    } else
        was_level_change_pressed = 0;

    //printf( "%f %f %f   %f %f %f\n", targetx, targety, targetz, gDirectionX, gDirectionY, gDirectionZ );
    if (gMouseLastClickButton != -1) {
        printf("Click on: %f %f %f\n", gTargetHitX, gTargetHitY, gTargetHitZ);
        if (gMouseLastClickButton == 0) {
            //Left-mouse
            float targetx = gTargetHitX + gTargetNormalX * .5;
            float targety = gTargetHitY + gTargetNormalY * .5;
            float targetz = gTargetHitZ + gTargetNormalZ * .5;
            float dist = sqrt((targetx - gPositionX)*(targetx - gPositionX) +
                    (targety - gPositionY)*(targety - gPositionY) +
                    (targetz - gPositionZ)*(targetz - gPositionZ));

            if (ClickCellCB)
                ClickCellCB(1, targetx, targety, targetz, dist);
        } else if (gMouseLastClickButton == 2) {
            //Right-mouse
            float targetx = gTargetHitX - gTargetNormalX * .5;
            float targety = gTargetHitY - gTargetNormalY * .5;
            float targetz = gTargetHitZ - gTargetNormalZ * .5;
            float dist = sqrt((targetx - gPositionX)*(targetx - gPositionX) +
                    (targety - gPositionY)*(targety - gPositionY) +
                    (targetz - gPositionZ)*(targetz - gPositionZ));

            if (ClickCellCB)
                ClickCellCB(0, targetx, targety, targetz, dist);
        }
        gMouseLastClickButton = -1;
    }

    //We're on the ground.  Make sure we don't die.
    if (gTimeSinceOnGround < .001) {
        int px = gPositionX;
        int py = gPositionY;
        int pz = gPositionZ;
        //printf( "%d %d %d\n", px, py, pz );
        if (IsOnDeathBlock(px, py, pz)) {
            printf("DIE\n");
            Die();
        }
    }


    if (IsPlayerInRange(2, 40, 63, 16, 16, 4)) {
        int lx = ((int) gPositionX) - 2;
        int ly = ((int) gPositionY) - 40;
        sprintf(gDialog, "OnTile %d\n", lx + ly * 16);
    }
    //Util: Make reference swatches.
    for (i = 0; i < 256; i++) {
        ChangeCell(0, 2 + (i % 16), 40 + (i / 16), 63, 1, DEFAULT_BRIGHT, 255, i);
    }

    /*
            totaltime += worldDeltaTime;
            sprintf( gDialog, "Update %f %f %f\n", gTargetHitX, gTargetHitY, gTargetHitZ );
     */

}




