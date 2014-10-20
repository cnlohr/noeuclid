/* 
 * File:   GameMap.h
 * Author: tehdog
 *
 * Created on 19. Oktober 2014, 02:26
 */

#ifndef GAMEMAP_H
#define	GAMEMAP_H

class GameMap {
public:
    GameMap();
    GameMap(const GameMap& orig);
    void update();
    void collision(struct CollisionProbe * ddat);
    virtual ~GameMap();
private:
    void init();
};


#endif	/* GAMEMAP_H */

