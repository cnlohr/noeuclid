#ifndef _GLUT_CORE
#define _GLUT_CORE


#ifdef WIN32
#include "winclude.h"
#define EXPORT_MAND __declspec(dllexport)
#ifdef CPPGPGPUMODULE
#define EXPORT __declspec(dllimport)
#else
#define EXPORT EXPORT_MAND
#endif
#pragma warning( disable : 4251)
#else
#define EXPORT
#define EXPORT_MAND
#endif


// Apple's annoying non-standard GL include location
#if defined(__APPLE__) || defined(MACOSX)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

///Function prototype for the redraw function in GLUT
typedef void (*DrawFunct)();

///Function prototype for the function that gets called whenever the viewport gets resized
typedef void (*ReshapeFunct)( int Width, int Height );

///Function that gets called whever the mouse button is clicked in GLUT
typedef void (*MousePressFunct)( int b, int state, int x, int y );

///Function that gets called while the mouse is being dragged in GLUT
typedef void (*MouseDragFunct)( int x, int y );

///Default reshape function inside of GLUTCore
void DefaultReshape( int Width, int Height );

///Default draw function held within GLUTCore
void DefaultDraw();

///Default mouse pressing function in GLUTCore
void DefaultMousePress( int b, int state, int x, int y );

///Default mouse dragging function
void DefaultMouseDrag( int x, int y );

///GLUT Core Class
/** This class provides a C++ interface to a simple GL window using GLUT.
    By utilizing this class, it provides a common interface for other things in
    the CPPGPGPU library without increasing code common to all of the projects.
*/
class EXPORT GLUTCore
{
public:
	GLUTCore();

	///Initialization function, call this before doing anything else
	/** This must be called AFTER you use GLUTCore's callbacks and before
	    pretty much anything else with GLUT you can simply call this
	    immediately after main with the parameters from your main() function
	    as well as the initial width, height and title for the window */
	void Init( int argc, char**argv, int iWidth, int iHeight, const char * sName );
	
	///Main GLUT Loop call
	/** Call this after you've done everything you want to have execute once.
	    This is what causes the different callbacks to get called over and
	    over. */
	void DrawAndDoMainLoop();

	///Sets the startup drawing function.
	void SetDrawFunct( DrawFunct DF ) { mDF = DF; }

	///Sets the startup reshaping function.
	void SetReshapeFunct( ReshapeFunct RS ) { mRS = RS; }

	///Sets the startup mouse press function.
	void SetMousePressFunct( MousePressFunct MP ) { mMP = MP; }

	///Sets the startup mouse drag function.
	void SetMouseDragFunct( MouseDragFunct MD ) { mMD = MD; }

	///FPS Tack and Get Delta Time
	/** This function updates the internal (GLUTCore) timers and if a
	    second has been past, it will display FPS.  It will always return
	    the total time between last frame and this frame */
	float TackFPS( bool bSilent = false );

	///Set camera distance for default distance on reshape
	/** Whenever the viewport is resized, the default change viewport
	    function gets called and that looks at the camera distance to
	    generate the apropriate camera transformations */
	void SetCamDist( const float fDist ) { fGLUTCamDistance = fDist; }

	///The current Width and Height of the window
	int miWidth, miHeight;

	///The last location of the mouse
	int miLastMouseX, miLastMouseY;

	///The current location of the mouse
	int miSetMouseX, miSetMouseY;

	int miZoomY;

	///This value gets set by SetCamDistance (See that function)
	static float fGLUTCamDistance;
	int GetDeltaMS() { return iDeltaMS; }

	///Get the proc address for an extension.
	void (*GetProcAddress( const char * csp ))();
private:
	///Delta time between last frame and this frame
	int iDeltaMS;

	///Total number of frames ever seen
	unsigned iNumFrames;
#ifdef WIN32
	_int64 lastPerfCount;
	_int64 perfFreq;
	_int64 lastFPS;
#else
	///The last seen second.  If this is different, FPS will be printed upon tack.
	unsigned iLastSecond;

	///The last second seen by the frametimer.
	unsigned iTmrLastSecond;
	///The last millisecond seen by the frametimer.
	unsigned iTmrLastMillisecond;
#endif
	///Current drawing function
	DrawFunct mDF;
	///Current drawing function
	ReshapeFunct mRS;
	///Current drawing function
	MousePressFunct mMP;
	///Current drawing function
	MouseDragFunct mMD;
};


///Instantiation of the GLUTCore object
extern EXPORT GLUTCore GLUT;

#endif

//Copyright 2007 Charles Lohr under the MIT/X11 License
