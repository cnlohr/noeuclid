#ifndef _NONEUCLID_H
#define _NONEUCLID_H

#ifdef __cplusplus
extern "C" {
#endif

extern double worldDeltaTime;

extern float gDaytime;

extern float gRenderMixval;
extern float gRenderDensityLimit;
extern float gRenderDensityMux;

extern float gPositionX;
extern float gPositionY;
extern float gPositionZ;
extern float gDirectionX;
extern float gDirectionY;
extern float gDirectionZ;
extern float gTargetNormalX;
extern float gTargetNormalY;
extern float gTargetNormalZ;
extern float gTargetCompressionX;
extern float gTargetCompressionY;
extern float gTargetCompressionZ;
extern float gTargetHitX;
extern float gTargetHitY;
extern float gTargetHitZ;
extern float gTargetActualDistance;
extern float gTargetProjDistance;
extern float gTargetPerceivedDistance;
extern char gDialog[1024];
extern unsigned char gKeyMap[256];
extern unsigned char gFocused;
extern int gMouseLastClickButton;
extern float gTimeSinceOnGround;

extern int gOverallUpdateNo;

extern double GameTimer;
extern double GameAttempt;

extern int AddSizeStride;

void ChangeCell( int t, int x, int y, int z, unsigned char r, unsigned char g, unsigned char b, unsigned char a );
void QuickCell( int t, int x, int y, int z, unsigned char r, unsigned char g, unsigned char b, unsigned char a );
void QuickCell1GBAOnly( int x, int y, int z, unsigned char g, unsigned char b, unsigned char a );
void UpdateZone( int x, int y, int z, int sx, int sy, int sz );
int AllocAddInfo( int nradds );
void AlterAddInfo( int pos, float x, float y, float z, float a );
void MarkAddDataForReload();

#ifndef _TCCEXPORTS_H
double sin( double f );
double cos( double f );
double pow( double base, double exp );
#endif

#ifdef __cplusplus
};
#endif

#endif

