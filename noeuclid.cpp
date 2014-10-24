/*
        Copyright (c) 2005-2014 <>< Charles Lohr
        All rights reserved.

        Redistribution and use in source and binary forms, with or without
        modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
                  notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
                  notice, this list of conditions and the following disclaimer in the
                  documentation and/or other materials provided with the distribution.
 * Neither the name of the <organization> nor the
                  names of its contributors may be used to endorse or promote products
                  derived from this software without specific prior written permission.

        THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
        ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
        WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
        DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
        DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
        (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
        LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
        ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
        (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
        SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifdef _WIN32
#include <windows.h>
#include <GL/glew.h>
#include <GL/glut.h>
#endif
#include "OGLParts.h"
#include "RTHelper.h"
#include <unistd.h>
#include "Common.h"
#include <sys/time.h>
#include "GameMap.h"
#include "TCC.h"
#include <fstream>
void mousePress(int b, int state, int x, int y);

void mouseDrag(int x, int y);

void reshape(int Width, int Height);

void draw();

extern GLuint imTypes[6];

extern GLuint imXTypes[6];
extern GLuint byTypes[6];

class GLUTCore {
public:

    GLUTCore() {
    };

    ///Initialization function, call this before doing anything else

    /** This must be called AFTER you use GLUTCore's callbacks and before
        pretty much anything else with GLUT you can simply call this
        immediately after main with the parameters from your main() function
        as well as the initial width, height and title for the window */
    void Init(int argc, char**argv, int iWidth, int iHeight, const char * sName) {
        fGLUTCamDistance = 5.0;
        iNumFrames = 0;
        iLastSecond = 0;
        miWidth = iWidth;
        miHeight = iHeight;
        /* ask for a window at 0,0 with dimensions winWidth, winHeight */
        /* need color, depth (for 3D drawing) and double buffer (smooth display) */
        glutInit(&argc, argv);
        glutInitWindowPosition(0, 0);
        glutInitWindowSize(iWidth, iHeight);
        glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
        glutCreateWindow(sName);

        /* set callback functions to be called by GLUT for drawing, window
        resize, keypress, mouse button press, and mouse movement */
        glutDisplayFunc(draw);
        glutReshapeFunc(reshape);
        glutMouseFunc(mousePress);
        glutMotionFunc(mouseDrag);
        glutPassiveMotionFunc(mouseDrag);
    };

    ///Main GLUT Loop call

    /** Call this after you've done everything you want to have execute once.
        This is what causes the different callbacks to get called over and
        over. */
    void DrawAndDoMainLoop() {
        //The first time you run TackFPS it will return a trash value, so get it out of the way now
        TackFPS(true);

        //Pre-emptively call the resize and draw functions to get the cycle going
        reshape(miWidth, miHeight);
        draw();
        glutMainLoop();
    };
    ///FPS Tack and Get Delta Time

    /** This function updates the internal (GLUTCore) timers and if a
        second has been past, it will display FPS.  It will always return
        the total time between last frame and this frame */
    float TackFPS(bool bSilent = false) {
        iNumFrames++;

        unsigned iCurSecond, iCurMillisecond;
        struct timeval T;
        gettimeofday(&T, 0);
        iCurSecond = T.tv_sec;
        iCurMillisecond = T.tv_usec;
        if (iCurSecond != iLastSecond) {
            iLastSecond = iCurSecond;
            if (!bSilent)
                printf("FPS: %d\n", iNumFrames);
            iNumFrames = 0;
        }
        iDeltaMS = (iCurSecond - iTmrLastSecond)*1000000 + (iCurMillisecond - iTmrLastMillisecond);
        iTmrLastSecond = iCurSecond;
        iTmrLastMillisecond = iCurMillisecond;
        return ((float) iDeltaMS) / 1000000.0f;
    };

    ///Set camera distance for default distance on reshape

    /** Whenever the viewport is resized, the default change viewport
        function gets called and that looks at the camera distance to
        generate the apropriate camera transformations */
    void SetCamDist(const float fDist) {
        fGLUTCamDistance = fDist;
    }

    ///The current Width and Height of the window
    int miWidth, miHeight;

    ///The last location of the mouse
    int miLastMouseX, miLastMouseY;

    ///The current location of the mouse
    int miSetMouseX = 0, miSetMouseY = 0;

    int miZoomY;

    ///This value gets set by SetCamDistance (See that function)
    static float fGLUTCamDistance;

private:
    ///Delta time between last frame and this frame
    int iDeltaMS;

    ///Total number of frames ever seen
    unsigned iNumFrames;
    ///The last seen second.  If this is different, FPS will be printed upon tack.
    unsigned iLastSecond;

    ///The last second seen by the frametimer.
    unsigned iTmrLastSecond;
    ///The last millisecond seen by the frametimer.
    unsigned iTmrLastMillisecond;
};
float GLUTCore::fGLUTCamDistance;

GLUTCore glut;

//Default usage parameters for the mouse motion functs.
int oldX, oldY, buttonPressed = 0, rbuttonPressed = 0;



int frame = 0;

RTHelper * gh;

int cix = 0;
int ciy = 0;
int gGodMode = 0;

float mouseSensitivity = 0.5;
double GameTimer = 1000;
double GameAttempt = 1;
extern int pickables_in_inventory;
float gTimeSinceOnGround;
byte gKeyMap[256];
bool bPause = false;
byte gFocused;
double worldDeltaTime;
int show_debugging = 0;
int pickables_in_inventory = 0;
int AddSizeStride = ADDSIZEX;

void DrawSquare(float minx, float miny, float maxx, float maxy) {
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(minx, miny, 0.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(minx, maxy, 0.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(maxx, maxy, 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(maxx, miny, 0.0f);
    glEnd();
}

void SetupFor2D() {
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glScalef(1.0, -1.0, 0.0);
    glTranslatef(-1.0, -1.0, 0.0);
    glScalef(2.0 / float(glut.miWidth), 2.0 / float(glut.miHeight), 0.0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void PopFrom2D() {
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void StripDataFromBuffer(int ix, int iy, int iwidth, int iheight, TextureType tt, char * buffer) {
    glReadPixels(ix, iy, iwidth, iheight, imXTypes[tt], byTypes[tt], buffer);
}

void pKeyDown(byte key, int x, int y) {
    if (key == 27) {
        printf("Esc was pushed\n");
        exit(0);
    } //esc as a quck exit.
    if (key == 'p') bPause = !bPause;
    if (key == 'g') gGodMode = !gGodMode;
    if (key == '0') show_debugging = !show_debugging;
    if (key == '8') {
        mouseSensitivity *= .75;
        if (mouseSensitivity < .1) mouseSensitivity = .1;
    }
    if (key == '9') {
        mouseSensitivity *= 1.5;
        if (mouseSensitivity > 2.0) mouseSensitivity = 2.0;
    }
    gKeyMap[key] = 1;
}

void pKeyUp(byte key, int x, int y) {
    gKeyMap[key] = 0;
}

int lButtonDown = 0, rButtonDown = 0;

/*
double Pitch = 90.;
double Yaw = 0.;
double Roll = 0.0;
 */
float LookQuaternion[4] = {0, 0, 0, 1.0f};

Vec3f gPosition;
Vec3f gDirection;
Vec3f gTargetNormal;
Vec3f gTargetCompression;
Vec3f gTargetHit;
float gTargetActualDistance;
float gTargetProjDistance;
float gTargetPerceivedDistance;
char gDialog[1024];
int gMouseLastClickButton = -1;
int gOverallUpdateNo = 0; //Gets reset if we "re-load" everything
GameMap gamemap;
unordered_map<string, int> aliases;

void mousePress(int b, int state, int x, int y) {
    glut.miLastMouseX = x;
    glut.miLastMouseY = y;
    if (b == GLUT_LEFT_BUTTON)
        lButtonDown = !lButtonDown;
    if (b == GLUT_RIGHT_BUTTON)
        rButtonDown = !rButtonDown;

    if (state)
        gMouseLastClickButton = b;
}

void mouseDrag(int x, int y) {
    if (!gFocused) return;
    //Find the amount moved from last frame to this frame.
    float dx = x - glut.miWidth / 2;
    float dy = y - glut.miHeight / 2;
    dx *= mouseSensitivity * worldDeltaTime * 40;
    dy *= mouseSensitivity * worldDeltaTime * 40;
    glut.miLastMouseX = x;
    glut.miLastMouseY = y;

    if (gFocused) {
        if (dx != 0 || dy != 0) {
            glutWarpPointer(glut.miWidth / 2, glut.miHeight / 2);
            glutSetCursor(GLUT_CURSOR_NONE);
        }
    }

    float qinitrotFwd[4];
    float qinitrotBack[4];
    float initialrotaxis[] = {1, 0, 0};

    quatfromaxisangle(qinitrotFwd, initialrotaxis, 3.14159 / 2.);
    quatfromaxisangle(qinitrotBack, initialrotaxis, -3.14159 / 2.);

    quatrotateabout(LookQuaternion, LookQuaternion, qinitrotFwd);


    float invrot[4];
    quatcopy(invrot, LookQuaternion);
    invrot[0] *= -1;
    float rot[4];
    quatcopy(rot, LookQuaternion);

    //We rotate around +Z because...


    //This whole mess is rotating aroudn the wrong axis.


    float qtmp2[4];
    float euler2[] = {-dy * 0.005f, 0.0, 0.0};
    quatfromeuler(qtmp2, euler2);
    quatrotateabout(LookQuaternion, LookQuaternion, qtmp2);

    quatrotateabout(LookQuaternion, LookQuaternion, invrot);
    float qtmp1[4];
    float euler1[] = {0.0, 0.0, dx * 0.005f};
    quatfromeuler(qtmp1, euler1);
    quatrotateabout(LookQuaternion, LookQuaternion, qtmp1);

    quatrotateabout(LookQuaternion, LookQuaternion, rot);


    quatrotateabout(LookQuaternion, LookQuaternion, qinitrotBack);
}

void reshape(int Width, int Height) {
    //set up a projection, rotation and general camera stuff with respect to mouse input
    glut.miWidth = Width;
    glut.miHeight = Height;
    glViewport(0, 0, (GLint) Width, (GLint) Height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float) Width / (float) Height, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -GLUTCore::fGLUTCamDistance);
    glRotatef(glut.miSetMouseY * 0.5f, 1.0f, 0.0f, 0.0f);
    glRotatef(glut.miSetMouseX * 0.5f, 0.0f, 0.0f, 1.0f);
}

vector<CollisionProbe *> probes;

CollisionProbe * gpTest;
CollisionProbe * gpTestVelocity;
CollisionProbe * gpForward;
CollisionProbe * gpRotFwd;
CollisionProbe * gpRotUp;
RGBAf LastSquish;

float gPhyDTime;
//static

void LoadProbes(bool isRerun) {
    Vec3f gv; //Goal direction.
    Vec3f d = {0, 0, 0};

    if (!isRerun) {
        //		if( gKeyMap['r'] ) gOverallUpdateNo = 0;
        //		'r' is handled by the script, itself.

        if (gKeyMap['t']) {
            //reset full map.
            gh->TMap->DefaultIt();
            gh->TMap->RecalculateAccelerationStructure(0, 0, 0, GLH_SIZEX, GLH_SIZEY, GLH_SIZEZ);
            gh->TMap->m_bReloadFullTexture = true;

        }
        if (gKeyMap['a']) d.x -= 1.;
        if (gKeyMap['d']) d.x += 1.;
        if (gKeyMap['s']) d.y += 1.;
        if (gKeyMap['w']) d.y -= 1.;
        if (gKeyMap[']']) d.z += 1.;
        if (gKeyMap['[']) d.z -= 1.;
        if (gKeyMap[' '] && gTimeSinceOnGround < 0.1) gh->v.z = 10; // && gh->gTimeSinceOnGround < 0.1
        gTimeSinceOnGround += worldDeltaTime;

        /*
                float ny = dx * sin( -Yaw/180.*3.14159 ) + dy * cos( -Yaw/180.*3.14159 );
                float nx = dx * cos( -Yaw/180.*3.14159 ) - dy * sin( -Yaw/180.*3.14159 );
                Roll = Roll * .95; //auto-righting.
         */
    }

    Vec3f ForwardVec = {0, 0, -1};
    ForwardVec = quatrotatevector(LookQuaternion, ForwardVec);
    ForwardVec.z = -ForwardVec.z; //??? WHY? WHY WHY??? Is LookQuaternion busted???

    Vec3f MoveVec = {d.x, 0, d.y};
    MoveVec = quatrotatevector(LookQuaternion, MoveVec);
    float nx = MoveVec.x;
    float ny = MoveVec.y;

    //	printf( "%f %f %f\n", fwdx, fwdy, fwdz );

    if (gGodMode) {
        gh->v = {nx * 4.f, ny * 4.f, d.z * 4.f};
        gh->MapOffset += gh->v*worldDeltaTime;
    } else {
        if (!isRerun) {
            float xymag = sqrt(nx * nx + ny * ny);
            if (xymag > .001) {
                nx /= xymag;
                ny /= xymag;
            }

            gh->v.x = nx * 4.;
            gh->v.y = ny * 4.;
            gh->v.z -= worldDeltaTime * 16.; //gravity
            gh->v.z *= .995; //terminal velocity
        }
        /*			else
                        {
                                gh->vX = 0;
                                gh->vY = 0;
        //				gh->vZ = 0;
                        }
                        gh->vZ = gh->vZ * .994;
         */

    }


    if (!isRerun) {
        gv = gh->v*worldDeltaTime;
    } else {
        printf("GVRerun\n");
    }
    //Spew out a boatload of rays, trying to intersect things.

    int i, j;
    const int stacks = 6;
    for (i = 0; i < stacks * 2; i++) {
        //Stack goes: 1 2 3 4 5 4 3 2 1, i goes 012345678
        int stack = i;
        if (i >= stacks) stack = stacks * 2 - stack - 1;

        //Stack is the number of radial rays.
        float sigma = (i / ((float) stacks * 2 - 1)* 3.14159);
        d.z = cos(sigma);
        float mz = sin(sigma);
        stack++;
        for (j = 0; j < stack; j++) {
            float theta = (j / (float) stack) * 3.14159 * 2.0;
            d.x = mz * cos(theta);
            d.y = mz * sin(theta);
            CollisionProbe * p;
            probes.push_back(p = gh->AddProbe());
            p->Position = RGBAf(gh->MapOffset);
            p->Direction = RGBAf(d + gv, 10000);
        }
    }


    {
        gpTest = gh->AddProbe();
        gpTest->Position = RGBAf(gh->MapOffset);
        gpTest->Direction = RGBAf(gv, gv.len());

        gpTestVelocity = gh->AddProbe();
        gpTestVelocity->Position = RGBAf(gh->MapOffset, 0);
        gpTestVelocity->Direction = RGBAf(gv, gv.len());
        gpTestVelocity->AuxRotation = RGBAf(gh->v, 0);

    }


    gpForward = gh->AddProbe();
    gpForward->Position = RGBAf(gh->MapOffset, 0);
    gpForward->Direction = RGBAf(ForwardVec, 10000);


    //	Yaw
    //	Pitch
    //	Roll
    //???


    float Front[3] = {1, 0, 0}, FrontRot[3]; //1,0,0 = Right; 0,0,1 = Fwd
    float Up[3] = {0, 1, 0}, UpRot[3]; //0,1,0 = Up
    quatrotatevector(FrontRot, LookQuaternion, Front);
    quatrotatevector(UpRot, LookQuaternion, Up);

    //	FrontRot[1] *= -1;
    //	FrontRot[0] *= -1;
    //	FrontRot[2] *= -1;
    //	UpRot[2] *= -1;

    gpRotFwd = gh->AddProbe();
    gpRotFwd->Position = RGBAf(gh->MapOffset, 0);
    gpRotFwd->Direction = RGBAf(gv, gv.len());
    gpRotFwd->AuxRotation = RGBAf(FrontRot[0], FrontRot[1], FrontRot[2], 0);

    gpRotUp = gh->AddProbe();
    gpRotUp->Position = RGBAf(gh->MapOffset, 0);
    gpRotUp->Direction = RGBAf(gv, gv.len());
    gpRotUp->AuxRotation = RGBAf(UpRot[0], UpRot[1], UpRot[2], 0);


}

void DoneProbes(bool bReRun) {
    //#define CFM  .4
    //#define CFMXY .5
    double CFM = .2;
#define VCFM 1.0
#define COLLIDESIZE 0.5
    //Ratio of XY to Z
#define COLLIDESIZERATIO 3
#define MINSIDE .0
#define ITERATIONS 10


    //Here's what we do: gpTest->TargetLocation.r, .g, .b contains the new position we should be at, bar whatever forces we're receiving from the other rays.

    int i;

    Vec3f newp = gh->MapOffset;

    int iterations = 0;


    CollisionProbe * tp = gpTest;


    if (tp->Direction.r * tp->Direction.r + tp->Direction.g * tp->Direction.g + tp->Direction.b * tp->Direction.b > 0.0 && !bReRun) {
        Vec3f df = tp->TargetLocation.vec() - newp;

        double dtdiffx = df.len();

        newp = tp->TargetLocation.vec();

        double dirdiffx = sqrt(tp->Direction.r * tp->Direction.r + tp->Direction.g * tp->Direction.g + tp->Direction.b * tp->Direction.b);

        //Check to see if it's a jump, if so, consider re-running probes.
        if (dtdiffx > dirdiffx * 1.5 + 1.0) {
            printf("Jump %f %f %f -> %f %f %f\n", gh->MapOffset.x, gh->MapOffset.y, gh->MapOffset.z, tp->TargetLocation.r, tp->TargetLocation.g, tp->TargetLocation.b);
            newp = gh->MapOffset = tp->TargetLocation.vec(); // - tp->Direction.r; //??? WHY not these but Z?
            //XXX WHY WHY WHY??? WHY?? (Read why below)
            //XXX TODO TODO TODO!! There is a glitch.  We have to rotate the tp->Direction before adding it, otherwise really weird things will happen.
            //I haven't gotten around to this yet.

            //Attempt to correct direction of speed.
            gh->v = gpTestVelocity->NewDirection.vec();

            gh->ForceProbeReRun();
            goto clend;
        }
    }


    for (iterations = 0; iterations < ITERATIONS; iterations++)
        for (i = probes.size() - 1; i >= 0; i--) {
            CollisionProbe * tp = probes[i];
            //		if( tp->Normal.a > 1.0 ) continue;

            //		if( tp->TargetLocation.a > COLLIDESIZE ) continue;



            //We have a collision, need to "push" back.

            Vec3f sfn = tp->Normal.vec();

            //Actual hit xyz
            Vec3f t = tp->TargetLocation.vec();
            Vec3f nd = t - newp; //tp->Direction.r;		//Direction of ray.

            double nddiff = nd.len();
            if (nddiff < 0.00001 || std::isnan(nddiff)) {
                //fprintf( stderr, "Error: fault with tp direction.\n" );
                continue; //Don't know why this could happen.
            }

            //Make sure this was a resolved surface.  Unresolved services point to
            //some oopsies that happened inside the ray tracer.
            if (tp->Normal.r > 1) {
                continue;
            }



            nd /= nddiff;
            Vec3f iaw = tp->InAreaWarp.vec(); //Space compression


            double newcalcd = 0.0;
            //newcalcd = tp->TargetLocation.a;
            Vec3f newcollision = t - newp;
            newcollision.x /= iaw.x;
            newcollision.y /= iaw.y;
            newcollision.z /= iaw.z;

            //Tricky: newcollision can actually go inverted if the thing would be fast enough to get embedded.  Flip it back around.
            newcollision.x = fabs(newcollision.x) * ((nd.x > 0) ? 1 : -1);
            newcollision.y = fabs(newcollision.y) * ((nd.y > 0) ? 1 : -1);
            newcollision.z = fabs(newcollision.z) * ((nd.z > 0) ? 1 : -1);

            newcalcd = sqrt(newcollision.x * newcollision.x * COLLIDESIZERATIO + newcollision.y * newcollision.y * COLLIDESIZERATIO + newcollision.z * newcollision.z / COLLIDESIZERATIO);
            //		printf( "%f-%f %f-%f %f-%f ---\n", newcollisionx,newcollisiony,newcollisionz );

            if (newcalcd > COLLIDESIZE) continue;
            float press = (COLLIDESIZE - newcalcd) / COLLIDESIZE;

            tp->id = i;
            gamemap.collision(tp);

            /*
                            printf( " +%f (%f %f %f)  ->\n", press, newcollisionx, newcollisiony, newcollisionz );
                            printf( "  %f %f %f %f\n", tp->Position.r, tp->Position.g, tp->Position.b, tp->Position.a );   //x,y,z,unused
                            printf( "  %f %f %f %f\n", tp->Direction.r, tp->Direction.g, tp->Direction.b, tp->Direction.a );   //x,y,z,unused
                            printf( "  %f %f %f %f\n", tp->AuxRotation.r, tp->AuxRotation.g, tp->AuxRotation.b, tp->AuxRotation.a );   //x,y,z,unused
                            printf( "  %f %f %f %f\n", tp->NewDirection.r, tp->NewDirection.g, tp->NewDirection.b, tp->NewDirection.a );   //x,y,z,unused
                            printf( "  %f %f %f %f\n", tp->Normal.r, tp->Normal.g, tp->Normal.b, tp->Normal.a );   //x,y,z,unused
                            printf( "  %f %f %f %f\n", tp->InAreaWarp.r, tp->InAreaWarp.g, tp->InAreaWarp.b, tp->InAreaWarp.a );   //x,y,z,unused
                            printf( "  %f %f %f %f\n", tp->TargetLocation.r, tp->TargetLocation.g, tp->TargetLocation.b, tp->TargetLocation.a );   //x,y,z,unused
             */

            //If it is a bottom probe, we are on the ground.
            if (i > probes.size() - 3) {
                gTimeSinceOnGround = 0;
                //gh->vZ = 0;
            }


            //First of all, nerf any motion toward the collision.
            {
                Vec3f ns = gh->v;
                ns /= ns.len();


                Vec3f dot = nd.dot(ns);
                if (dot.x > 0) {
                    gh->v.x *= (1. - dot.x * press) * VCFM;
                }
                if (dot.y > 0) {
                    gh->v.y *= (1. - dot.y * press) * VCFM;
                }
                if (dot.z > 0) {
                    gh->v.z *= (1. - dot.z * press * VCFM);
                }
            }
            //Next, push the MapOffset back
            //(Change newx, newy, newz)

            //(ndx,ndy,ndz) represents ray.
            //(tx, ty, tz)  represents target ray hit.
            //press = distance of compression.

            Vec3f nid = newcollision * press;
            nid = nid * CFM;
            nid = nid.dot(iaw);

            if (sqrt(nid.x * nid.x + nid.y * nid.y) < MINSIDE) {
                nid.x = nid.y = 0;
            }

            newp.x -= nid.x * fabs(sfn.x);
            newp.y -= nid.y * fabs(sfn.y);
            newp.z -= nid.z * fabs(sfn.z);
        }

    if (!gGodMode) {
        gh->MapOffset = newp;
    }


    //Extract Yaw, Pitch, Roll.

    //	printf( "%f %f %f\n", gpRotFwd->NewDirection.r, gpRotFwd->NewDirection.g, gpRotFwd->NewDirection.b );
    //	printf( "%f %f %f\n", gpRotUp->NewDirection.r, gpRotUp->NewDirection.g, gpRotUp->NewDirection.b );

    /*
            float PlusZ[3];
            float Up[3];
            float Fwd[3];
     */

    //Re-rotate the camera based on the jump.
    float orotmat[12];

    //	printf( "%f %f %f  %f %f %f\n", gpRotFwd->NewDirection.r, gpRotFwd->NewDirection.g, gpRotFwd->NewDirection.b,
    //		gpRotUp->NewDirection.r, gpRotUp->NewDirection.g, gpRotUp->NewDirection.b );
    memset(orotmat, 0, sizeof ( orotmat));
    copy3d(&orotmat[0], &gpRotFwd->NewDirection.r); //X
    copy3d(&orotmat[4], &gpRotUp->NewDirection.r); //Y
    cross3d(&orotmat[8], &orotmat[0], &orotmat[4]); //Z

    float newquat[4];
    quatfrommatrix(newquat, orotmat);

    //TODO: If we are in a situation where we're stuck on our side, don't exceute this line of code.
    quatnormalize(LookQuaternion, newquat);


    //Attempt to re-right the player

#define AUTO_RIGHT_COMP .8
    {
        //		float uptestcomp[3] = { 1, 0, 0 };
        float lefttest[3] = {1, 0, 0}; // 0, camera "up", -into screen
        float uptest[3] = {0, 1, 0}; // 0, camera "up", -into screen
        float fwdtest[3] = {0, 0, 1}; // 0, camera "up", -into screen
        float upout[3], fwdtestout[3], lefttestout[3];
        quatrotatevector(upout, LookQuaternion, uptest);
        quatrotatevector(fwdtestout, LookQuaternion, fwdtest);
        quatrotatevector(lefttestout, LookQuaternion, lefttest);
        upout[2] *= -1;
        fwdtestout[0] *= -1;
        lefttestout[0] *= -1;
        fwdtestout[1] *= -1;
        lefttestout[1] *= -1;

        lefttestout[2] = 0; //Force flat test.


        //		cross3d( rerotaxis, uptestcomp, upout );
        float irtcos = dot3d(upout, lefttestout) * AUTO_RIGHT_COMP; //how much effort to try to right?

        float cosofs = (3.14159 / 2.0);

        //Tricky: If we're upside-down we need to re-right ourselves.
        if (upout[2] < 0) {
            irtcos *= -1.0;
        }

        float uprotator[4];
        quatfromaxisangle(uprotator, fwdtest, acos(irtcos) - cosofs);
        quatrotateabout(LookQuaternion, LookQuaternion, uprotator);

    }
clend:
    probes.clear();

    gPosition = newp;
    gDirection = gpForward->Direction.vec();
    gTargetNormal = gpForward->Normal.vec();
    gTargetCompression = gpForward->InAreaWarp.vec();
    gTargetHit = gpForward->TargetLocation.vec();
    gTargetActualDistance = gpForward->Normal.a;
    gTargetProjDistance = gpForward->InAreaWarp.a;
    gTargetPerceivedDistance = gpForward->TargetLocation.a;


    gamemap.update();

    gOverallUpdateNo++;
    //printf( "%f %f %f\n", gPositionX, gPositionY, gPositionZ );

    if (!gGodMode) {
        gh->MapOffset = gPosition;
    }


    while (gh->MapOffset.x < 0) gh->MapOffset.x += GLH_SIZEX;
    while (gh->MapOffset.y < 0) gh->MapOffset.y += GLH_SIZEY;
    while (gh->MapOffset.z < 0) gh->MapOffset.z += GLH_SIZEZ;
    while (gh->MapOffset.x > GLH_SIZEX) gh->MapOffset.x -= GLH_SIZEX;
    while (gh->MapOffset.y > GLH_SIZEY) gh->MapOffset.y -= GLH_SIZEY;
    while (gh->MapOffset.z > GLH_SIZEZ) gh->MapOffset.z -= GLH_SIZEZ;

    //	printf( "%7.1f %7.1f %7.1f  /  %7.1f %7.1f %7.1f (%f %f %f)\n", NewYaw, NewPitch, NewRoll, Yaw, Pitch, Roll, gh->MapOffsetX, gh->MapOffsetY, gh->MapOffsetZ );





}

//Our rotation may have changed depending on physics.

void UpdatePositionAndRotation() {
    //Updated rotation here.
    glLoadIdentity();
    float mat44[16];
    quattomatrix(mat44, LookQuaternion);
    glMultMatrixf(mat44);
}

void ThisWindowStatus(int status) {
    gFocused = status == 1;
}

void draw() {
    static double TotalTime = 280;
    glClearColor(.1f, .1f, .2f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    double DeltaTime = glut.TackFPS();
    worldDeltaTime = DeltaTime;
    if (gKeyMap[9]) worldDeltaTime *= 10.;
    if (gKeyMap['`']) worldDeltaTime /= 10.;
    if (bPause) worldDeltaTime = 0;
    TotalTime += worldDeltaTime;
    glLoadIdentity();

    /*
            glRotatef( Pitch, 1., 0., 0. );
            glRotatef( Yaw, 0., 0., 1. );
            glRotatef( Roll, 0., 1., 0. );
     */
    float mat44[16];
    quattomatrix(mat44, LookQuaternion);
    glMultMatrixf(mat44);

    glPushMatrix();
    gh->DrawMap(worldDeltaTime, TotalTime);
    DrawSquare();
    glPopMatrix();

    glColor4f(1., 1., 1., 1.);
    glDisable(GL_TEXTURE_3D);
    glEnable(GL_TEXTURE_2D);
    SetupFor2D();

    glPushMatrix();
    glTranslatef(glut.miWidth / 2 - 7, glut.miHeight / 2 - 12, 0);
    DrawText("+");
    glPopMatrix();
    glTranslatef(10, 30, 0);
    DrawText(gDialog);
    glTranslatef(-10, -30, 0);

    glTranslatef(0, 125, 0);

    char tt[1024];
    if (show_debugging) {
        sprintf(tt, "%1.2f %1.2f %1.2f\n", gh->MapOffset.x, gh->MapOffset.y, gh->MapOffset.z);
        DrawText(tt);
    }

    float pers = glut.miWidth * glut.miHeight / (gh->LastPass1Time / 1. + gh->LastPass2Time / 1. + gh->LastPass3Time);
    glTranslatef(0, 25, 0);
    if (show_debugging) {
        sprintf(tt, "T1: %1.3f\nT2: %1.3f\nT3: %1.3f\n%f Perf\n", gh->LastPass1Time / 1000000., gh->LastPass2Time / 1000000., gh->LastPass3Time / 1000000., pers);
        DrawText(tt);
    }

    glTranslatef(30, -150, 0);
    sprintf(tt, "%3.2f\n", GameTimer);
    DrawText(tt);
    glTranslatef(300, 0, 0);
    sprintf(tt, "TRY %1.f\nInven: %d", GameAttempt, pickables_in_inventory);
    DrawText(tt);

    PopFrom2D();


    frame++;
    glutSwapBuffers();
    glutPostRedisplay();

}
TCC tcc;

int main(int argc, char ** argv) {
    for (unsigned i = 0; i < 256; i++)
        gKeyMap[i] = 0;
    unsigned int xSize = 720, ySize = 480;

    ifstream tccheader("tccinclude.h");
    if(!tccheader.is_open()) throw runtime_error("missing header");
    tcc.addheader(string((istreambuf_iterator<char>(tccheader)),
            istreambuf_iterator<char>()));
    
#define TCCADDFUN(var) tcc.add(#var, &tcc##var)
    tcc.add("swoovey",&swoovey);
    TCCADDFUN(Cell);
    TCCADDFUN(ClearCell);
    TCCADDFUN(ClearRange);
    TCCADDFUN(EmptyBox);
    TCCADDFUN(WarpSpace);
    TCCADDFUN(PlayerInRange);
    TCCADDFUN(PaintRange);
#define TCCADDINT(var, sym, type) tcc.add(#var, sym);tcc.addheader("extern "#type" "#var";")

#define TCCADD(var, type) TCCADDINT(var, &var, type)
#define TCCADDVEC(var) TCCADDINT(var##X, &var.x, int);\
        TCCADDINT(var##Y, &var.y, int);\
        TCCADDINT(var##Z, &var.z, int);
    TCCADDVEC(gPosition);
    TCCADDVEC(gDirection);
    TCCADD(gDaytime, float);

    
    float initialrotaxis[] = {1, 0, 0};
    quatfromaxisangle(LookQuaternion, initialrotaxis, -3.14159 / 2.);

    glut.Init(argc, (char**) argv, xSize, ySize, "No! Euclid!");
#ifndef EMSCRIPTEN
    glutWindowStatusFunc(ThisWindowStatus);
#endif
#ifdef _WIN32
	printf("Initializing GLEW.\n");
	printf("glewInit() = %d\n",glewInit());
#endif
    //For display purposes, we should depth test all of our surfaces.
    glEnable(GL_DEPTH_TEST);

    gh = new RTHelper(0);

    gh->MapOffset = {GLH_SIZEX / 2, GLH_SIZEY / 2, 5};

    glutKeyboardFunc(pKeyDown);
    glutKeyboardUpFunc(pKeyUp);


    glut.DrawAndDoMainLoop();
}
