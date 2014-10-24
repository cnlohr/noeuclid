#define true 1
#define false 0
#define v(x,y,z) x,y,z
#define vf(x,y,z) x,y,z
#define PICKABLE_CELL 114
#define DEFAULT_BRIGHT 190
#define DEFAULT_DENSITY 125
#define WALL_BLOCK 12
#define WHITE_BLOCK 19
#define GOAL_BLOCK 24
#define DEADGOAL_BLOCK 7
#define DEFAULT_EMPTY_BLOCK 0
#define SPAAACE_CELL 255
typedef unsigned char byte;

//TODO remerge this with scripthelpers.h
// WARNING: tcc does not throw errors when the signature of a function is not known!
// tcc funs
void Cell(int x, int y, int z, byte block, byte density);
void ClearCell(int x, int y, int z);
void ClearRange(int x, int y, int z, int x2, int y2, int z2);
void EmptyBox(int x, int y, int z, int x2, int y2, int z2, byte block, byte density);
void WarpSpace(int x, int y, int z, int x2, int y2, int z2, float f1, float f2, float f3);
int PlayerInRange(float x, float y, float z, float x2, float y2, float z2);
void PaintRange(int x, int y, int z, int x2, int y2, int z2, byte block, byte density);
//Make a triangle wave.
float swoovey(float f, float siny);