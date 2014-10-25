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
    if(initscript) (*initscript)();
}

void Room::update() {
    timeInRoom+=worldDeltaTime;
    if(runscript) (*runscript)(timeInRoom);
    for(auto& runfn:runs) runfn(timeInRoom);

    if (PlayerInRangeV(exitr1, exitr2)) {
        gamemap.setRoom(gamemap.curroom+1);
    }
}