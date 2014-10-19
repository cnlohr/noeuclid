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
    void Update();
    void collision(struct CollisionProbe * ddat);
    virtual ~GameMap();
private:

};


#endif	/* GAMEMAP_H */

