/* 
 * File:   GameMap.h
 * Author: tehdog
 *
 * Created on 19. Oktober 2014, 02:26
 */

#ifndef GAMEMAP_H
#define	GAMEMAP_H
#define START_ROOM 1
//Define all lava/death blocks.
//If the user touches one, he dies.
#define MAX_DEATH_BLOCKS 8192
#define MAX_PICKABLES 256

#include <functional>
#include "scripthelpers.h"
using initfn = std::function<void()>;
using runfn = std::function<void(double timeIn)>;
#include "Room.h"
#include <unordered_map>
class GameMap {
public:
    int lastroom = -1, curroom = START_ROOM;
    DeathBlock DeathBlocks[MAX_DEATH_BLOCKS];
    PickableBlock PBlocks[MAX_PICKABLES];
    vector<Room*> rooms {
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
    GameMap();
    GameMap(const GameMap& orig);
    void update();
    void die();
    void collision(struct CollisionProbe * ddat);
    virtual ~GameMap();
    
    #define DONE (i>>ws).eof()
//TODO nobody can read this
    unordered_map<string, std::function<initfn(istream&)>> initfuncs {
        {"EmptyBox", [](istream& i) -> initfn {
            Vec3i p,s; BlockType b; i>>p>>s>>b;
            int density = DEFAULT_DENSITY; if(!DONE) i>>density;
            return bind(MakeEmptyBox, p, s, b, density, DEFAULT_BRIGHT, 1);
        }}, {"Cell", [](istream& i) -> initfn { 
            Vec3i p; BlockType b; i>>p>>b;
            return bind(ChangeCell, 0, p, 1, DEFAULT_BRIGHT, 255, b);
        }}, {"ClearCell", [](istream& i) -> initfn {
            Vec3i p; i>>p; //TODO merge with "Block"
            return bind(ChangeCell, 0, p, 0, DEFAULT_BRIGHT, 0, DEFAULT_EMPTY_BLOCK);
        }}, {"ClearRange", [](istream& i) -> initfn {
            Vec3i p,s; i>>p>>s;
            return bind(ClearRange, p, s);
        }}, {"PaintRange", [](istream& i) -> initfn {
            Vec3i p,s; BlockType b; i>>p>>s>>b;
            int density = DEFAULT_DENSITY; if(!DONE) i>>density;
            return bind(PaintRange, p, s, b, density, 1);
        }}, {"Warp", [](istream& i) -> initfn { 
            Vec3i p,s; Vec3f x;
            i>>p>>s>>x;
            return bind(SetWarpSpaceArea, p, s, x);
        }}, {"JumpSection", [](istream& i) -> initfn {
            Vec3i p,s; Vec3f ofs, f1 = {1,0,0},f2={0,1,0},f3={0,0,1};
            i>>p>>s>>ofs;
            if(!DONE) i>>f1>>f2>>f3;
            return bind(MakeJumpSection, p, s, ofs, f1, f2, f3);
        }}, {"RequireRoom", [this](istream& i) -> initfn {
            int room; i>>room;
            return [this,room]() { rooms[room]->begin(); };
        }}
    };
    unordered_map<string, function<runfn(istream&)>> runfuncs {
        {"AnimateOpen", [](istream& i) -> runfn {
            Vec3i p,s; i>>p>>s;
            return [p,s](double timeIn) {
                int capden = 255 - timeIn * 200;
                PaintRange(p, s, GOAL_BLOCK, capden<0?0:capden);
            };
        }}, {"AnimateClose", [](istream& i) -> runfn {
            Vec3i p,s; i>>p>>s;
            return [p,s](double timeIn) {
                int capden = 255 - timeIn * 200;
                PaintRange(p, s, DEADGOAL_BLOCK, capden<0?255:255-capden);
            };
        }}
    };
    unordered_map<string, int> aliases;
    
    void AddDeathBlock(Vec3i p);
    bool IsOnDeathBlock(Vec3i p);

    //Returns id if pickable is there.
    //Returns -1 if no block.
    //Returns -2 if block tween incomplete.

    int GetPickableAt(Vec3i p);

    void DissolvePickable(int pid);
    //If -1, no pickables left.
    //If -2, Pickable already there.
    //Otherwise returns which pickable!
    //initial_density should be 0 unless you want to shorten (+) or lengthen (-) tween.

    int PlacePickableAt(Vec3i p, float initial_density);

    void ClearPickableBlocks();

    //Redraw 

    void UpdatePickableBlocks();

    void PickableClick(bool left, Vec3f p, float dist);
private:
    void init();
};



#endif	/* GAMEMAP_H */

