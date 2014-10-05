
#include "GLUTCore.h"
#include <stdio.h>
#if defined( _WIN32 )
#include <time.h>
#else
#include <sys/time.h>
#endif

#ifdef GLEXT
#include "../WindowsBuild/GLEXTHandler.h"
#endif

#include <stdlib.h>
#ifdef WIN32
#include <GL/gl.h>
#include <windows.h>
#endif

EXPORT float GLUTCore::fGLUTCamDistance;
EXPORT GLUTCore GLUT;

//Default usage parameters for the mouse motion functs.
int oldX, oldY, buttonPressed = 0, rbuttonPressed = 0;

void DefaultMousePress( int b, int state, int x, int y )
{
	GLUT.miLastMouseX = x; GLUT.miLastMouseY = y;
	if( b == GLUT_LEFT_BUTTON )
		buttonPressed = !buttonPressed;
	if( b == GLUT_RIGHT_BUTTON )
		rbuttonPressed = !rbuttonPressed;
}

void DefaultMouseDrag( int x, int y )
{
	//Find the amount moved from last frame to this frame.
	float dx = x - GLUT.miLastMouseX;
	float dy = y - GLUT.miLastMouseY;
	GLUT.miLastMouseX = x;
	GLUT.miLastMouseY = y;
	if( buttonPressed )
	{
		//Update the absolute difference of mouse location since start.
		GLUT.miSetMouseX += (int)dx;
		GLUT.miSetMouseY += (int)dy;
	}
	if( rbuttonPressed )
	{
		GLUTCore::fGLUTCamDistance += dy/10.0;
		DefaultReshape( GLUT.miWidth, GLUT.miHeight );
	}
	glLoadIdentity();
	glTranslatef( 0.0f, 0.0f, -GLUTCore::fGLUTCamDistance );
	glRotatef( GLUT.miSetMouseY*0.5f, 1.0f, 0.0f, 0.0f );
	glRotatef( GLUT.miSetMouseX*0.5f, 0.0f, 0.0f, 1.0f );
}

void DefaultReshape( int Width, int Height )
{
	//set up a projection, rotation and general camera stuff with respect to mouse input
	GLUT.miWidth = Width;
	GLUT.miHeight = Height;
	glViewport( 0, 0, (GLint)Width, (GLint)Height );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, (float)Width/(float)Height, 0.1f, 100.0f );
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef( 0.0f, 0.0f, -GLUTCore::fGLUTCamDistance );
	glRotatef( GLUT.miSetMouseY*0.5f, 1.0f, 0.0f, 0.0f );
	glRotatef( GLUT.miSetMouseX*0.5f, 0.0f, 0.0f, 1.0f );
}

void DefaultDraw()
{
	glClearColor(.1,.1,.2,0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBegin( GL_QUADS );
	glVertex3f( -1.0f, -1.0f, 0.0f );
	glVertex3f( 1.0f, -1.0f, 0.0f );
	glVertex3f( 1.0f, 1.0f, 0.0f );
	glVertex3f( -1.0f, 1.0f, 0.0f );
	glEnd();

	//Swap the buffers and tell GLUT to call the redraw function again.
	glutSwapBuffers();
	glutPostRedisplay();
}

GLUTCore::GLUTCore()
{
	mDF = DefaultDraw;
	mRS = DefaultReshape;
	mMP = DefaultMousePress;
	mMD = DefaultMouseDrag;
	miSetMouseX = 0;
	miSetMouseY = 0;
}

float GLUTCore::TackFPS( bool bSilent )
{
#ifdef WIN32
	//On Win32, there's some bugs with the GLUT causing it not to exit,
    //this checks and exits.  After benchmarking, this proved to add minimal
    //overhead.
	if( !wglGetCurrentDC() ) exit ( 0 );
#endif
	iNumFrames++;
	#if defined( WIN32 )
        _int64 perfCount;
		QueryPerformanceCounter((LARGE_INTEGER *)&perfCount);
		QueryPerformanceFrequency((LARGE_INTEGER *)&perfFreq);
		_int64 ticks = lastPerfCount - perfCount;
		if( lastFPS + perfFreq > perfCount )
		{
			if( !bSilent )
				printf( "FPS: %d\n", iNumFrames);
			iNumFrames = 0;
			lastFPS = perfCount;
		}
		float elapsed = (double)ticks / perfFreq;
		lastPerfCount = perfCount;
		return elapsed;
	#else // not WIN32
	unsigned iCurSecond, iCurMillisecond;
	struct timeval T;
	gettimeofday( &T, 0 );
	iCurSecond = T.tv_sec;
	iCurMillisecond = T.tv_usec;
	if( iCurSecond != iLastSecond )
	{
		iLastSecond = iCurSecond;
		if( !bSilent )
			printf( "FPS: %d\n", iNumFrames );
		iNumFrames = 0;
	}
	iDeltaMS = (iCurSecond - iTmrLastSecond)*1000000 + (iCurMillisecond - iTmrLastMillisecond );
	iTmrLastSecond = iCurSecond;
	iTmrLastMillisecond = iCurMillisecond;
	return ((float)iDeltaMS)/1000000.0f;
	#endif // WIN32
}

void WMClose()
{
	exit( 0 );
}

void GLUTCore::Init( int argc, char**argv, int iWidth, int iHeight, const char * sName )
{
	fGLUTCamDistance = 5.0;
	iNumFrames = 0;
#ifndef WIN32
	iLastSecond = 0;
#endif
	miWidth = iWidth;
	miHeight = iHeight;
	/* ask for a window at 0,0 with dimensions winWidth, winHeight */
	/* need color, depth (for 3D drawing) and double buffer (smooth display) */
	glutInit( &argc, argv );
	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( iWidth, iHeight );
	glutInitDisplayMode( GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE );
	glutCreateWindow(sName);
	
	/* set callback functions to be called by GLUT for drawing, window
	resize, keypress, mouse button press, and mouse movement */
	glutDisplayFunc(mDF);
	glutReshapeFunc(mRS);
	glutMouseFunc(  mMP);
	glutMotionFunc( mMD);
	glutPassiveMotionFunc( mMD );
	
#ifdef GLEXT
	SetupAllExtensions();
#endif

}

#if !defined( WIN32 ) && !defined( __APPLE__ )
#include <GL/freeglut_ext.h>
#endif

void (*GLUTCore::GetProcAddress( const char * csp ))()
{
#ifdef WIN32
	return (void (__cdecl *)(void))wglGetProcAddress( csp );
#else
	return (void(*)())glutGetProcAddress( csp );
#endif
}

void GLUTCore::DrawAndDoMainLoop()
{
	//The first time you run TackFPS it will return a trash value, so get it out of the way now
	GLUT.TackFPS( true );

	//Pre-emptively call the resize and draw functions to get the cycle going
	mRS( miWidth, miHeight );
	mDF();
	glutMainLoop();
}

//Copyright 2007 Charles Lohr, Dr. Marc Olano under the MIT/X11 License
