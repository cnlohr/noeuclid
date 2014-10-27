#ifndef _OGL_PARTS_H
#define _OGL_PARTS_H

#ifdef GLEXT
#include <GL/glew.h>
#endif

#include <cstdio>


#include <vector>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glext.h>

using std::vector;
using std::string;
using byte = unsigned char;
///Texture type for use with CPPGPGPU.
enum TextureType
{
	TTGRAYSCALE = 0,
	TTRGB,
	TTRGBA,
	TTRGBA16,
	TTRGBA32,
	TTUNDEFINED
};



///Basic element for turning shaders on and off
/** This class helps aide in the loading and use of shaders.  It allows loading of files
    through the LoadShader() function that actively looks for .frag and .vert files.  By use
    of the CheckForNewer() function, the class looks for changes, if any changes are found
    the shaders will be re-loaded, compiled and linked.  Once this node has a shader loaded,
    the shader can be activated or deactivated using the ActivateShader() and
    DeactivateShader() functions respectively.  Note that shaders do not stack.  When you
    call DeactiveShader(), it does not bring back previously activated shaders.  It simply
    turns all shaders off. */
class Shader
{
public:
	Shader(string sShaderName, string preamble = "");
	~Shader();

	///Suggested function for loading shaders.
	/** This function looks for {sShaderName}.vert and {sShaderName}.frag.  It will
	    attempt to load, compile and link the files.  If any errors are found, they will
	    be announced at STDOUT.  When Geometry shader support is added, it will search
	    for .geom files */
	bool LoadShader();

	///Explicitly load a fragment shader
	/** This function takes on raw code in the sShaderCode string and attemps to compile
	    it.  Any errors it runs into will be displayed at STDOUT.  Note you are
            discouraged to use this function, in favor of using LoadShader(). */
	bool LoadShaderFrag( const char * sShaderCode );

	///Explicitly load a vertex shader
	/** This function takes on raw code in the sShaderCode string and attemps to compile
	    it.  Any errors it runs into will be displayed at STDOUT. You should use
	    LoadShader() instead of this function. */
	bool LoadShaderVert( const char * sShaderCode );

	///Explicitly load a geometry shader
	/** This function takes on raw code in the sShaderCode string and attemps to compile
	    it.  Any errors it runs into will be displayed at STDOUT. You should use
	    LoadShader() instead of this function. */
	bool LoadShaderGeom( const char * sShaderCode );

	///Explicitly link all shaders currently loaded
	/** This takes vertexShader and fragmentShader and converts them into iProgramID.
	    this function is discouraged when using LoadShader(). */
	bool LinkShaders();

	///Activate shader with extra parameters
	/** Activate 'this' shader with the following uniform sampler2D values in 
	    vsAllSamplerLocs, this assigns numerically values 0-8 to the values in
	    vsAllSamplerLocs.  vsUniformFloats are uniform floats that get assigned values
	    vfUniformFloats.  Then it turns the shader on for all future rendering */
	bool ActivateShader( vector< string > &vsAllSamplerLocs, vector< string > &vsUniformFloats, vector < float > & vfUniformFloats );

	///Activate Shader using default uniform floats
	bool ActivateShader( vector< string > &vsAllSamplerLocs);

	///Activate Shader without any uniforms at all.
	bool ActivateShader() { vector<string> st; return ActivateShader( st ); }

	///Activate Shader using exactly one uniform sampler2D
	bool ActivateShader( const string & s ) { vector<string> st; st.push_back( s ); return ActivateShader( st ); }

	///Turn off all shaders
	bool DeactivateShader();

	///Check for newer version of 'this' shader
	void CheckForNewer();
        

	///Explicitly get the OpenGL ProgramID in the event you need it for advanced techniques
	GLhandleARB	GetProgramID() { return iProgramID; }
private:
	///Get the last modified time for sShaderName
	/* This function takes on iOut as being where to put the last time the shader was modified.
	   this value is system dependent and is necessiarly not linked to anything like seconds. */
	void GetTimeCodes( unsigned long * iOut, const char * sShaderName  );

	///The OpenGL Program ID
	GLhandleARB	iProgramID;

	///The OpenGL Geometry Program ID
	GLhandleARB	geometryShader;

	///The OpenGL Vertex Program ID
	GLhandleARB	vertexShader;

	///The OpenGL Fragment Program ID
	GLhandleARB	fragmentShader;

	///ASM Only information for vertex programs
	GLuint  vertexProgram;
	///ASM Only information for fragment programs
	GLuint	fragmentProgram;
	///ASM Only information for programs
	bool bIsGLSLAsm;
        string sShaderName, preamble;
};

///Element for setting texture states in OpenGL.
/** This can be used for loading .ppm files or just data straight from the drive.
    the idea is that you turn on textures like states.  Once a texture is loaded
    you can turn it on to affect anything that gets drawn.  Most systems have 4-8
    texture slots that can be activated and used as input for your shaders.

    Textures can have data inputted into them from either a file, CPU memory,
    stripped off of the screen or written by a render/frame buffer.
*/
class Texture
{
public:
	Texture() : iTextureType(GL_TEXTURE_2D) { iTexture = (GLuint)-1; }
	~Texture();

	///Setup the texture to behave as a dynamic texture.  This is to be used
	///when you write to it using the screen or a render/frame buffer.
	bool MakeDynamicTexture( int iWidth, int iHeight, TextureType tt );

	///Load the image from the drive in the format of a .ppm file.
	bool LoadTextureFile( const char * sFileName, bool bWithMipMaps = false );

	///Load the image from raw data on the CPU.
	bool LoadTexture( char * sRawData, int iWidth, int iHeight, TextureType tt, bool bWithMipMaps = false );

	///Activate the texture in the iPlaceth slot.
	bool ActivateTexture( int iPlace = 0 );

	///Deactivate the texture in the iPlaceth slot.
	bool DeactivateTexture( int iPlace = 0 );

	///Obtain the OpenGL Texture handle.
	unsigned int GetTexHandle() { return iTexture; }

	///Strip the texture from the screen into the texture.  Note, you must have
	//already used the MakeDynamicTexture function.
	bool StripFromScreen( int x, int y, int width, int height );

	///Get width of the texture
	inline int GetWidth( ) { return mWidth; }

	///Get height of the texture
	inline int GetHeight( ) { return mHeight; }

	///Switch between glNearest (1) or glNicest (0).  You want to use Nearest
	///for any data processing, and Nicest for anything that will be presented.
	void ChangeFilter( bool bNearest );

	///Set up all mip maps and switch to mip map zoom mode
	void ComputeMipMaps();

	///OpenGL Texture ID
	GLuint iTexture;

	///Texture Type
	TextureType mtt;

	///Width and Height
	int mWidth, mHeight;

	GLenum iTextureType; //Almost always GL_TEXTURE_2D
};

///Render / Frame Buffer
/** This object allows easy use of setting up the framebuffer and or renderbuffer.
    The general idea is that you can set one of these up once for each type of
    operation you plan to do (to have a depth buffer or not.)  Once configured,
    you tell it to start, perform drawing, and tell it to end.  Anything you did
    during the drawing phase will be written to TexturesToAttach.  In order to
    access more than the first buffer, you must use a shader and output the data
    to different buffers.  In general, most systems allow 4-8 textures to be
    attached. */
class RFBuffer
{
public:
	RFBuffer();
	~RFBuffer();

	///Setup the RFBuffer, this should only get called once ever, on init.
	bool Setup( bool bUseDepthBuffer = true );

	///Startup the framebuffer, this redirects the output to the selected textures.
	bool ConfigureAndStart( int iWidth, int iHeight, int iTextures, Texture * TexturesToAttach, bool bClear = false );

	///Redirect the drawing back to the regular framebuffer.  Note, you should use
	///GLUT.miWidth and GLUT.miHeight for the parameters here.
	bool End( int iRegVPX, int iRegVPY );

	///Get the width of the selcted framebuffer.
	int GetWidth() { return mWidth; }

	///Get the height of the selcted framebuffer.
	int GetHeight() { return mHeight; }

	///Get the Texture Type of the selcted framebuffer.
	TextureType GetTT() { return mtt; }
private:
	bool m_bUseDepthBuffer;
	int mWidth, mHeight;
	TextureType mtt;

	GLuint iRenderbuffer;
	GLuint iOutputBuffer;
	GLuint iNumTextures;
};

class VertexData;
class IndexData;

///Vertex Buffer Object
/** This class is used to hold Vertex data.  It can be ripped from the 0th slot on a
    RFBuffer when it's being used. Idealy, you would either load regular data into
    here when operating normally, or you would rip data off a framebuffer into this object.
    When loading data into this object, you would execute StripFromFrameBuffer()
    immediately before your framebuffer's End() command. */
class VertexData
{
public:
	VertexData() { iVertexData = 0; }
	~VertexData();

	///Set Verts to be NULL if you will be streaming to this.  Otherwise, put data in here.
	void Init( float * Verts, int iNumVerts, int iStride = 3 );

	///Allows you to change the data enclosed within the buffer.
	void UpdateData( float * Verts, int iNumVerts, int iStride = 3 );

	///Note: This can only be stripped from 0th texture in the framebuffer.
	void StripFromFrameBuffer( int iWidth, int iHeight, TextureType tt );

	///Turn a texture into this buffer.
	void StripFromTexture( Texture & t );

	///Get OpenGL Handle
	inline GLuint GetHandle() { return iVertexData; }

	int mStride;
	GLuint iVertexData;
};

///Index Data
/** This class is used for holding index data.  You cannot do much GPGPU with this, it is
    genearlly just used to hold index data that is to be loaded in on program start. */
class IndexData
{
public:
	IndexData() { mIndexData = (GLuint*)-1; }
	~IndexData();

	///Set up the Index buffer using an index array described by Indices.
	/** This class will handle deletion of your indices when it is deleted.  DO NOT
	    delete the pointer you pass into this function, otherwise it won't have them
	    Also, note that you cannot delete them while this function is using them. */
	void Init( int * Indices, int iNumIndices );

	///Get OpenGL Handle
	inline GLuint * GetHandle() { return mIndexData; }

	GLuint mIndexCount;
	GLuint * mIndexData;
};

///Draw a single square from [-1,-1] to [1,1]
void DrawSquare( float minx = -1., float miny = -1., float maxx = 1., float maxy = 1. );

///Draw the Index Data id  using the specified vertex data and shader using prescribed method (GL_POINTS/GL_LINES/etc.)
void DrawGeometry( VertexData ** vd, string * vsVertexNames, int iNumVertexDatas, IndexData * id, Shader * sShader, GLint mode );

///Setup matricies for computation.
/** Special command, for setting up the perspective and modelview matricies strictly for
    computation purpose.  This command and the PopFromDataRun() command should be placed
    around any geometry you will be drawing for computation purposes. */
void SetupForDataRun();
///Special command, for reversing effect of SetupForDataRun.
void PopFromDataRun();

///Setup Matricies for 2D Rasterization
void SetupFor2D();
///Go back into 3D mode
void PopFrom2D();

///Strip data from the render/frame buffer into a vertex buffer object. 
void StripDataFromBuffer( int ix, int iy, int iwidth, int iheight, TextureType tt, char * buffer );

///Write a buffer to filename in PPM format with 8 bits per pixel RGB.
void WritePPM( char *filename, char *sourceBuf, int w, int h );

///Font alignment helper, used for Vertical and Horizontal text alignment
enum TextAlignment
{
	TEXT_RIGHT,
	TEXT_CENTER,
	TEXT_LEFT,
	TEXT_HTOP,
	TEXT_HCENTER
};

///Draw text by the given name at the given location on screen.
bool DrawText( const char * text, float size = 1.0, const string & sFontName = "Arial", 
	float MaxWidth = 10000., float MaxHeight = 10000., TextAlignment Horiz = TEXT_LEFT, 
	TextAlignment Vert = TEXT_HTOP );

/********** GENERAL UTILITY SECTION  **********************************************************/

///OpenImage Utility function - This isn't ordinarily used, but has been found to be useful.  It
///allocates a buffer, and you give it pointers to width and height.  0 return indicates a failed
///load.
int OpenImage( const char * sImageName, byte ** buffer, int * width, int * height, int * channels = NULL );

///Generic image loader format
typedef int (*OpenImageFunction)( const char * sPPMName, byte ** buffer, int * width, int * height, int * channels );

///General Utility function to pull a line out of a file of iBuffersize or less.  You must create
/// the line* value.
int FileReadLine( FILE * f, char * line, int iBufferSize );

///Register a new image loader
void RegisterImageLoader( unsigned int fingerprint, OpenImageFunction f );

#define FINGERPRINTIZE( str ) (str[0] + (str[1]<<8) + (str[2]<<16))

///Execute arbitrary code before main
#define EXECUTE_BEFORE_MAIN( CLASSNAME, CODE ) \
	class RegClass##CLASSNAME { public: \
		RegClass##CLASSNAME() { \
		CODE \
		} \
	}; \
	RegClass##CLASSNAME AUTOREG##CLASSNAME;

#endif

/*
Copyright (c) 2005, 2006, 2007 Charles Lohr, David Chapman, Joshua Allen

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
