#include "GameMap.h"
#include <cstring>
#define v(x,y,z) x,y,z
extern GameMap gamemap;
void Room::reset() {
    gPosition = start;
    GameTimer = maxTime;
    begin();
}
void Room::begin() {
    for(auto& initfn:inits) initfn();
    if(initscript) (*initscript)();
}

void Room::update() {
    timeInRoom+=worldDeltaTime;
    run();
    if(runscript) (*runscript)(timeInRoom);
    for(auto& runfn:runs) runfn(timeInRoom);

    if (PlayerInRangeV(exitr1, exitr2)) {
        gamemap.curroom++;
    }
}