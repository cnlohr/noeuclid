/* 
 * File:   GameMap.h
 * Author: tehdog
 *
 * Created on 19. Oktober 2014, 02:26
 */

#ifndef GAMEMAP_H
#define	GAMEMAP_H
//Define all lava/death blocks.
//If the user touches one, he dies.
#define MAX_DEATH_BLOCKS 8192
#define MAX_PICKABLES 256

#include <functional>
#include "scripthelpers.h"
using initfn = std::function<void()>;
using runfn = std::function<void(double timeIn)>;
#include "Room.h"
#include "TCC.h"
#include <unordered_map>
extern TCC tcc;

class GameMap {
public:
    int lastroom = -1, curroom = 0, startroom = 0;
    DeathBlock DeathBlocks[MAX_DEATH_BLOCKS];
    PickableBlock PBlocks[MAX_PICKABLES];
    vector<Room> rooms;
    GameMap();
    GameMap(const GameMap& orig);
    void update();
    void die();
    void setRoom(int newroom, bool reset = false);
    void collision(struct CollisionProbe * ddat);
    virtual ~GameMap();
    
    #define DONE (i>>ws).eof()
//TODO nobody can read this
    unordered_map<string, std::function<initfn(istream&)>> initfuncs {
        {"EmptyBox", [](istream& i) -> initfn {
            Vec3i p,s; BlockType b; i>>p>>s>>b;
            int density = DEFAULT_DENSITY; if(!DONE) i>>density;
            return bind(EmptyBoxV, p, s, true, RGBA{1, DEFAULT_BRIGHT, byte(density), b});
        }}, {"Cell", [](istream& i) -> initfn { 
            Vec3i p; BlockType b; i>>p>>b;
            return bind(ChangeCellV, 0, p, RGBA{1, DEFAULT_BRIGHT, 255, b});
        }}, {"ClearCell", [](istream& i) -> initfn {
            Vec3i p; i>>p; //TODO merge with "Block"
            return bind(ChangeCellV, 0, p, RGBA{0, DEFAULT_BRIGHT, 0, DEFAULT_EMPTY_BLOCK});
        }}, {"ClearRange", [](istream& i) -> initfn {
            Vec3i p,s; i>>p>>s;
            return bind(ClearRangeV, p, s);
        }}, {"PaintRange", [](istream& i) -> initfn {
            Vec3i p,s; BlockType b; i>>p>>s>>b;
            int density = DEFAULT_DENSITY; if(!DONE) i>>density;
            return bind(PaintRangeV, p, s, RGBA{1,190,byte(density),b});
        }}, {"Warp", [](istream& i) -> initfn { 
            Vec3i p,s; Vec3f x;
            i>>p>>s>>x;
            return bind(WarpSpaceV, p, s, x);
        }}, {"JumpSection", [](istream& i) -> initfn {
            Vec3i p,s; Vec3f ofs, f1 = {1,0,0},f2={0,1,0},f3={0,0,1};
            i>>p>>s>>ofs;
            if(!DONE) i>>f1>>f2>>f3;
            return bind(JumpSpaceV, p, s, ofs, f1, f2, f3);
        }}, {"RequireRoom", [this](istream& i) -> initfn {
            int room; i>>room;
            return [this,room]() { rooms[room].begin(); };
        }}
    };
    unordered_map<string, function<runfn(istream&)>> runfuncs {
        {"AnimateOpen", [](istream& i) -> runfn {
            Vec3i p,s; i>>p>>s;
            return [p,s](double timeIn) {
                int capden = 255 - timeIn * 200;
                PaintRangeV(p, s, {1, 190, byte(capden<0?0:capden), GOAL_BLOCK});
            };
        }}, {"AnimateClose", [](istream& i) -> runfn {
            Vec3i p,s; i>>p>>s;
            return [p,s](double timeIn) {
                int capden = 255 - timeIn * 200;
                PaintRangeV(p, s, {1,190,byte(capden<0?255:255-capden),DEADGOAL_BLOCK});
            };
        }}, {"DieInRange", [](istream& i) -> runfn {
            Vec3f p,s; i>>p>>s;
            return [p,s](double timeIn) {
                if (PlayerInRangeV(p,s)) ::die();
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
    void loadRooms(string fname);
};



#endif	/* GAMEMAP_H */

