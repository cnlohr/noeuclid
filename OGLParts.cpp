#include <stdio.h>
#include "OGLParts.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <map>
#include <errno.h>

using namespace std;

GLuint imTypes[6] = { GL_LUMINANCE8, GL_RGB, GL_RGBA, GL_RGBA16F_ARB, GL_RGBA32F_ARB, 0 };

//For things that require GL_RGBA when dealing with floating point data; usually when dealing with verticies or host data.
GLuint imXTypes[6] ={ GL_LUMINANCE, GL_RGB, GL_RGBA, GL_RGBA, GL_RGBA, 0 };
GLuint byTypes[6] = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_FLOAT, GL_FLOAT, 0 };
int channels[6] = { 1, 3, 4, 4, 4, 0 };

#ifndef __APPLE__
PFNGLPROGRAMPARAMETERIEXTPROC glProgramParameteriEXT_v = 0;
#endif

void DestroyShaderProgram( GLhandleARB iProgramID )
{
	GLhandleARB *objects = NULL;
	GLint i, count = -1;

	if( !iProgramID )
		return;

	//If we can't destroy the object, then don't try.
	glGetObjectParameterivARB(iProgramID, GL_OBJECT_ATTACHED_OBJECTS_ARB, &count);

	//Iterate through all children.
	if (count > 0)
	{
		objects = (GLhandleARB *)malloc(count*sizeof(GLhandleARB));
		glGetAttachedObjectsARB(iProgramID, count, NULL, objects);
	}
	else
		return;

	for ( i = 0; i < count; ++i)
	{
		glDetachObjectARB(iProgramID, objects[i]);
	}

	glDeleteObjectARB(iProgramID);
	free( objects );
	return;
}

Shader::Shader()
{
	iProgramID = (GLhandleARB)NULL; 
	vertexShader = (GLhandleARB)NULL;
	fragmentShader = (GLhandleARB)NULL;
	geometryShader = (GLhandleARB)NULL;
	iTimeCode[0] = 0;
	iTimeCode[1] = 0;
	iTimeCode[2] = 0;
	bIsGLSLAsm = 0;
//	sOldFilename = ""; 
}

Shader::~Shader()
{
	DestroyShaderProgram( iProgramID );
}

void Shader::CheckForNewer( const char * sShaderName )
{
	unsigned long iCurTimes[3];
	GetTimeCodes( iCurTimes, sShaderName );
	if( iCurTimes[0] != iTimeCode[0] || iCurTimes[1] != iTimeCode[1] || iCurTimes[2] != iTimeCode[2] )
	{
		DestroyShaderProgram( iProgramID );
		LoadShader( sShaderName );
	}
}

void Shader::GetTimeCodes( unsigned long * iOut, const char * sShaderName  )
{
	struct stat sb;
	stat( (string(sShaderName)+".vert").c_str(), &sb );
	iOut[0] = sb.st_mtime;
	stat( (string(sShaderName)+".frag").c_str(), &sb );
	iOut[1] = sb.st_mtime;
	stat( (string(sShaderName)+".geom").c_str(), &sb );
	iOut[2] = sb.st_mtime;
}

bool Shader::LoadShader( const char * sShaderName )
{
	GetTimeCodes( iTimeCode, sShaderName );
	string s1 = sShaderName, s2 = sShaderName, s3 = sShaderName;
	FILE * f1;
	FILE * f2;
	FILE * f3; //geometry
	char * Buffer;
	int i;

	s1 += ".frag";
	s2 += ".vert";
	s3 += ".geom";
	f1 = fopen( (char*)s1.c_str(), "rb" );
	f2 = fopen( (char*)s2.c_str(), "rb" );
	f3 = fopen( (char*)s3.c_str(), "rb" );
	if( f1 == 0 || f2 == 0 )
	{
		if( !f1 )
			printf( "Could not open %s.\n", (char*)s1.c_str() );
		if( !f2 )
			printf( "Could not open %s.\n", (char*)s2.c_str() );
		return false;
	}
	if( f1 )
	{
		fseek( f1, 0, SEEK_END );
		i = ftell( f1 );
		fseek( f1, 0, SEEK_SET );
		Buffer = (char*)malloc( i+1 );
		if( !fread( Buffer, 1, i, f1 ) )
			return false;
		fclose( f1 );
		printf( "Compiling: %s\n", s1.c_str() );
		Buffer[i] = '\0';
		if( !LoadShaderFrag( Buffer ) )
		{
			free( Buffer );
			if( f2 )
				fclose( f2 );
			if( f3 )
				fclose( f3 );
			printf( "Reporting failed shaderload. Not linking.\n" );
			return false;
		}
		free( Buffer );
	}
	if( f2 )
	{
		fseek( f2, 0, SEEK_END );
		i = ftell( f2 );
		fseek( f2, 0, SEEK_SET );
		Buffer = (char*)malloc( i+1 );
		if( !fread( Buffer, 1, i, f2 ) )
			return false;
		fclose( f2 );
		Buffer[i] = '\0';
		printf( "Compiling: %s\n", s2.c_str() );
		if( !LoadShaderVert( Buffer ) )
		{
			if( f3 )
				fclose( f3 );
			free( Buffer );
			printf( "Reporting failed shaderload. Not linking.\n" );
			return false;
		}
		free( Buffer );
	}
	if( f3 )
	{
		fseek( f3, 0, SEEK_END );
		i = ftell( f3 );
		fseek( f3, 0, SEEK_SET );
		Buffer = (char*)malloc( i+1 );
		if( !fread( Buffer, 1, i, f3 ) )
			return false;
		fclose( f3 );
		Buffer[i] = '\0';
		printf( "Compiling: %s\n", s3.c_str() );
		if( !LoadShaderGeom( Buffer ) )
		{
			free( Buffer );
			printf( "Reporting failed shaderload. Not linking.\n" );
			return false;
		}
		free( Buffer );
	}
	return LinkShaders();
}

bool Shader::LoadShaderFrag( const char * sShaderCode )
{
	if( strlen( sShaderCode ) < 5 )
		return false;

	if( strncmp( sShaderCode, "!!ARB", 5 ) == 0 )
	{
		glGenProgramsARB( 1, &fragmentProgram );
		glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, fragmentProgram );
		glProgramStringARB( GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, (GLsizei)strlen(sShaderCode), (GLubyte*)sShaderCode );
		if (glGetError() != GL_NO_ERROR)
		{
			puts( "ERROR Compiling ASM Fragment Shader Error follows:" );
			puts( (char*)glGetString(GL_PROGRAM_ERROR_STRING_ARB) );
			return false;
		}
		bIsGLSLAsm = 1;
	}
	else
	{
		GLint bFragCompiled;
		GLint stringLength;
		fragmentShader = glCreateShaderObjectARB( GL_FRAGMENT_SHADER_ARB );
		glShaderSourceARB( fragmentShader, 1, &sShaderCode, NULL );
		glCompileShaderARB( fragmentShader );
	
		glGetObjectParameterivARB( fragmentShader, GL_OBJECT_COMPILE_STATUS_ARB, &bFragCompiled );
		glGetObjectParameterivARB( fragmentShader, GL_OBJECT_INFO_LOG_LENGTH_ARB, &stringLength );
		if ( stringLength > 1 )
		{
			char * tmpstr = (char*)malloc( stringLength + 1 );
			glGetInfoLogARB( fragmentShader, stringLength, NULL, tmpstr );
			puts( "Compiling Fragment Shader response follows:" );
			puts( tmpstr );
			free( tmpstr );
			return bFragCompiled!=0;
		}
	}
	return true;
}

bool Shader::LoadShaderVert( const char * sShaderCode )
{
	if( strlen( sShaderCode ) < 5 )
		return false;

	if( strncmp( sShaderCode, "!!ARB", 5 ) == 0 )
	{
		glGenProgramsARB( 1, &vertexProgram );
		glBindProgramARB( GL_VERTEX_PROGRAM_ARB, vertexProgram );
		glProgramStringARB( GL_VERTEX_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, (GLsizei)strlen(sShaderCode), (GLubyte*)sShaderCode );
		if (glGetError() != GL_NO_ERROR)
		{
			puts( "ERROR Compiling ASM Fragment Shader Error follows:" );
			puts( (char*)glGetString(GL_PROGRAM_ERROR_STRING_ARB) );
			return false;
		}
		bIsGLSLAsm = 1;
	}
	else
	{
		GLint bVertCompiled;
		GLint stringLength;
		//Create a new vertex shader
		vertexShader = glCreateShaderObjectARB( GL_VERTEX_SHADER_ARB );
		//Bind the shader to the text, setting that to be its source.
		glShaderSourceARB( vertexShader, 1, &sShaderCode, NULL );
		//Compile the shader
		glCompileShaderARB( vertexShader );
		//Did the shader compile?  Were there any errors?
		glGetObjectParameterivARB( vertexShader, GL_OBJECT_COMPILE_STATUS_ARB, &bVertCompiled );
		glGetObjectParameterivARB( vertexShader, GL_OBJECT_INFO_LOG_LENGTH_ARB, &stringLength );
		if( stringLength > 1 )
		{
			char * tmpstr = (char*)malloc( stringLength + 1 );
			glGetInfoLogARB( vertexShader, stringLength, NULL, tmpstr );
			puts( "Compiling Vertex Shader response follows:" );
			puts( tmpstr );
			free( tmpstr );
			return bVertCompiled!=0;
		}
	}
	return true;
}

bool Shader::LoadShaderGeom( const char * sShaderCode )
{
	if( strlen( sShaderCode ) < 5 )
		return false;

	if( strncmp( sShaderCode, "!!ARB", 5 ) == 0 )
	{
		puts( "ERROR: ASM geometry shaders are not supported." );
		return false;
	}
	else
	{
		GLint bGeomCompiled;
		GLint stringLength;
		//Create a new geometry shader
		geometryShader = glCreateShaderObjectARB( GL_GEOMETRY_SHADER_EXT );
		//Bind the shader to the text, setting that to be its source.
		glShaderSourceARB( geometryShader, 1, &sShaderCode, NULL );
		//Compile the shader
		glCompileShaderARB( geometryShader );
		//Did the shader compile?  Were there any errors?
		glGetObjectParameterivARB( geometryShader, GL_OBJECT_COMPILE_STATUS_ARB, &bGeomCompiled );
		glGetObjectParameterivARB( geometryShader, GL_OBJECT_INFO_LOG_LENGTH_ARB, &stringLength );
		if( bGeomCompiled == 0 )
		{
			char * tmpstr = (char*)malloc( stringLength + 1 );
			glGetInfoLogARB( geometryShader, stringLength, NULL, tmpstr );
			puts( "Compiling Geometry Shader response follows:" );
			puts( tmpstr );

			free( tmpstr );
			return bGeomCompiled!=0;
		}
	}
	return true;
}

bool Shader::LinkShaders()
{
	if( bIsGLSLAsm )
	{
		return true;
	}
	else
	{
		GLint bLinked;
		GLint stringLength;
		//Create the actual shader prgoram
		iProgramID = glCreateProgramObjectARB();
		printf( "Linking Shaders. %d <- %d + %d + %d\n", iProgramID, vertexShader, fragmentShader, geometryShader );
		//Attach the fragment/vertex shader to it.
		if( vertexShader )
			glAttachObjectARB( iProgramID, vertexShader );
		if( fragmentShader )
			glAttachObjectARB( iProgramID, fragmentShader );
		if( geometryShader )
			glAttachObjectARB( iProgramID, geometryShader );
		//Attempt to link the shader
		glLinkProgramARB( iProgramID );
		printf( "Shaders Linked\n" );
		//If we're using a geometry shader, we have to do a little extra.
		if( geometryShader )
		{
#ifndef __APPLE__
			if( glProgramParameteriEXT_v == 0 )
			{
				glProgramParameteriEXT_v = (PFNGLPROGRAMPARAMETERIEXTPROC)GLUT.GetProcAddress( "glProgramParameteriEXT" );
			}
			if( glProgramParameteriEXT_v )
			{
				glProgramParameteriEXT_v( iProgramID, GL_GEOMETRY_INPUT_TYPE_EXT, GL_TRIANGLES );
				glProgramParameteriEXT_v( iProgramID, GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP );

				int ierror, i;
				GLint imaxvert;
				glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT,&imaxvert);
				if( (ierror = glGetError()) != 0 )
				{
					puts( "ERROR: You cannot load a geometry shader when there are still errors left in OpenGL." );
					puts( "Please track down the error remaining by using glGetError() to cordon off your code." );
					printf( "The last error received was: %d\n", ierror );
				}
				for( i = 1; i < imaxvert; i++ )
				{
					glProgramParameteriEXT_v(iProgramID,GL_GEOMETRY_VERTICES_OUT_EXT,imaxvert/i);
					if( glGetError() == 0 )
						break;
				}
				printf( "Geometry Shader loaded with a total of %d max verticies.  Because there are %d max vertices, and %d preceived components per vert.\n", imaxvert/i, imaxvert, i );
			}
#endif
		}


		//See if there were any errors.
		glGetObjectParameterivARB( iProgramID, GL_OBJECT_LINK_STATUS_ARB, &bLinked );
		glGetObjectParameterivARB( iProgramID, GL_OBJECT_INFO_LOG_LENGTH_ARB, &stringLength );
		printf( "Shaders Linked.\n" );
		if ( stringLength > 1 || bLinked == 0 )
		{
			char * tmpstr = (char*)malloc( stringLength + 1 );
			glGetInfoLogARB( iProgramID, stringLength, NULL, tmpstr );
			puts( "Linking shaders. response follows:" );
			puts( tmpstr );
			free( tmpstr );
			return bLinked!=0;
		}
		return true;
	}
}

bool Shader::ActivateShader( vector< string > &vsAllSamplerLocs, vector< string > &vsUniformFloats, vector < float > & vfUniformFloats )
{
	unsigned i;

	if( bIsGLSLAsm )
	{
		glEnable( GL_VERTEX_PROGRAM_ARB );
		glBindProgramARB( GL_VERTEX_PROGRAM_ARB, vertexProgram);
		glEnable( GL_FRAGMENT_PROGRAM_ARB );
		glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, fragmentProgram);

		for( i = 0; i < vfUniformFloats.size(); ++i )
		{
			glProgramEnvParameter4fARB(GL_VERTEX_PROGRAM_ARB, i, vfUniformFloats[i],0,0,0 );
			glProgramEnvParameter4fARB(GL_FRAGMENT_PROGRAM_ARB, i, vfUniformFloats[i],0,0,0 );
		}
	}
	else
	{
		glUseProgramObjectARB( iProgramID );
	
		for( i = 0; i < vsAllSamplerLocs.size(); ++i )
		{
			int iTexPosID = glGetUniformLocationARB( iProgramID, vsAllSamplerLocs[i].c_str() );
			if( iTexPosID > -1 )
				glUniform1iARB( iTexPosID, i );
		}
	
		for( i = 0; i < vsUniformFloats.size(); ++i )
		{
			int iTexPosID = glGetUniformLocationARB( iProgramID, vsUniformFloats[i].c_str() );
			if( iTexPosID > -1 )
				glUniform1fARB( iTexPosID, vfUniformFloats[i] );
		}	
	}
	return true;
}

bool Shader::ActivateShader( vector< string > &vsAllSamplerLocs)
{
	unsigned i;

	glUseProgramObjectARB( iProgramID );

	for( i = 0; i < vsAllSamplerLocs.size(); ++i )
	{
		int iTexPosID = glGetUniformLocationARB( iProgramID, vsAllSamplerLocs[i].c_str() );
		if( iTexPosID > -1 )
			glUniform1iARB( iTexPosID, i );
	}

	return true;
}

bool Shader::DeactivateShader()
{
	glUseProgramObjectARB( 0 );
	return true;
}

Texture::~Texture()
{
	if( (int)iTexture == -1 )
		return;
	glDeleteTextures( 1, &iTexture );
}

bool Texture::StripFromScreen( int x, int y, int width, int height )
{
	glActiveTextureARB( GL_TEXTURE0_ARB );
	glBindTexture( iTextureType, iTexture );
	glEnable( iTextureType );

	glCopyTexImage2D( iTextureType, 0, imXTypes[mtt], x, y, width, height, 0);

	glDisable( iTextureType );
        glBindTexture( iTextureType, 0 );
	return true;
}

bool Texture::MakeDynamicTexture( int iWidth, int iHeight, TextureType tt )
{
	mtt = tt;
	mWidth = iWidth;
	mHeight = iHeight;

	if( (int)iTexture == -1 )
		glGenTextures( 1, &iTexture );
	if( (int)iTexture == -1 )
		return false;

	glBindTexture( iTextureType, iTexture );
	glCopyTexImage2D( iTextureType, 0, imTypes[tt], 0, 0, iWidth, iHeight, 0 );
	glTexParameteri( iTextureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( iTextureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture( iTextureType, 0 );
	return true;
}

bool Texture::LoadTextureFile( const char * sFileName, bool bWithMipMaps )
{
	unsigned char * Buffer;
	if( (int)iTexture == -1 )
		glGenTextures( 1, &iTexture );
	if( (int)iTexture == -1 )
	{
		puts( "Texture failed to generate" );
		return false;
	}
	if( !OpenImage( sFileName, &Buffer, &mWidth, &mHeight ) )
	{
		puts( "PPM Failed to open" );
		return false;
	}
	mtt = TTRGB;

	glBindTexture( iTextureType, iTexture );
	if( bWithMipMaps )
	{
		gluBuild2DMipmaps( iTextureType, channels[mtt], mWidth, mHeight, imTypes[mtt], byTypes[mtt], Buffer );
		glTexParameterf( iTextureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		glTexParameterf( iTextureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	}
	else
	{
		glTexImage2D( iTextureType, 0, imTypes[mtt], mWidth, mHeight, 0, imTypes[mtt], byTypes[mtt], Buffer );
		glTexParameteri( iTextureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri( iTextureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	glBindTexture( iTextureType, 0 );
	free( Buffer );
	return true;
}

bool Texture::LoadTexture( char * sRawData, int iWidth, int iHeight, TextureType tt, bool bWithMipMaps )
{
	if( (int)iTexture == -1 )
		glGenTextures( 1, &iTexture );
	if( (int)iTexture == -1 )
		return false;

	mtt = tt;
	mWidth = iWidth;
	mHeight = iHeight;

	glBindTexture( iTextureType, iTexture );

	if( iTextureType == GL_TEXTURE_2D )
	{
		if( bWithMipMaps )
		{
			gluBuild2DMipmaps( iTextureType, channels[mtt], mWidth, mHeight, imTypes[mtt], byTypes[mtt], sRawData );
			glTexParameterf( iTextureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
			glTexParameterf( iTextureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		}
		else
		{
			glTexImage2D( iTextureType, 0, imTypes[mtt], mWidth, mHeight, 0, imXTypes[mtt], byTypes[mtt], sRawData );
			glTexParameteri( iTextureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameteri( iTextureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
	}
	else
	{
		
		glTexImage3DEXT( iTextureType, 0, imTypes[mtt], mWidth, mWidth, mHeight, 0, imXTypes[mtt], byTypes[mtt], sRawData );
		glTexParameteri( iTextureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( iTextureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
	}
	glBindTexture( iTextureType, 0 );
	return true;
}

void Texture::ChangeFilter( bool bNearest )
{
	glBindTexture( iTextureType, iTexture );
	glTexParameteri( iTextureType, GL_TEXTURE_MIN_FILTER, (bNearest)?GL_NEAREST:GL_LINEAR );
	glTexParameteri( iTextureType, GL_TEXTURE_MAG_FILTER, (bNearest)?GL_NEAREST:GL_LINEAR );
	glBindTexture( iTextureType, 0 );
}

bool Texture::ActivateTexture( int iPlace )
{
	glActiveTextureARB( GL_TEXTURE0_ARB + iPlace );
	glBindTexture( iTextureType, iTexture );
	glEnable( iTextureType );
	return true;
}

bool Texture::DeactivateTexture( int iPlace )
{
	glActiveTextureARB( GL_TEXTURE0_ARB + iPlace );
	glDisable( iTextureType );
	return true;
}

RFBuffer::RFBuffer()
{
	mWidth = 0; mHeight = 0; mtt = TTGRAYSCALE;
	iRenderbuffer = 0;
	iOutputBuffer = 0;
}

RFBuffer::~RFBuffer()
{
	if( iRenderbuffer != 0 )
	{
		glDeleteRenderbuffersEXT( 1, &iRenderbuffer );
		glDeleteFramebuffersEXT( 1, &iOutputBuffer );
	}
}

bool RFBuffer::Setup( bool bUseDepthBuffer )
{
	m_bUseDepthBuffer = bUseDepthBuffer;
	if( m_bUseDepthBuffer )
		glGenRenderbuffersEXT( 1, &iRenderbuffer );
	glGenFramebuffersEXT( 1, &iOutputBuffer );
	return true;
}

bool RFBuffer::ConfigureAndStart( int iWidth, int iHeight, int iTextures, Texture * TexturesToAttach, bool bClear )
{
	const GLenum buffers[8] = {   GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT,
			GL_COLOR_ATTACHMENT3_EXT, GL_COLOR_ATTACHMENT4_EXT, GL_COLOR_ATTACHMENT5_EXT,
			GL_COLOR_ATTACHMENT6_EXT, GL_COLOR_ATTACHMENT7_EXT };
	mWidth = iWidth; mHeight = iHeight;
	iNumTextures = iTextures;
	if( m_bUseDepthBuffer )
	{
		glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, iRenderbuffer );
		glRenderbufferStorageEXT( GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, mWidth, mHeight );
	}
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, iOutputBuffer );
	for( int i = 0; i < iTextures; i++ )
		glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i, 
			GL_TEXTURE_2D, TexturesToAttach[i].GetTexHandle(), 0 );
	if( m_bUseDepthBuffer  )
		glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, 
			GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, iRenderbuffer );

	//Added by Peter Bessman to verify nonexecution of the following code.
	#ifdef _WIN32
	if (glDrawBuffersARB)
	#endif
		glDrawBuffersARB( iTextures, buffers );
	glViewport( 0, 0, mWidth, mHeight );

	//Check to see if there were any errors with the framebuffer
	switch( (GLenum)glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) )
	{
		case GL_FRAMEBUFFER_COMPLETE_EXT: break;  //GOOD!
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
			printf("OpenGL Framebuffer error: GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT\n");
			return false;
		case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
			printf("OpenGL Framebuffer error: GL_FRAMEBUFFER_UNSUPPORTED_EXT\n");
			return false;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
			printf("OpenGL Framebuffer error: GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT\n");
			return false;
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
			printf("OpenGL Framebuffer error: GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT\n");
			return false;
		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
			printf("OpenGL Framebuffer error: GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT\n");
			return false;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
			printf("OpenGL Framebuffer error: GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT\n");
			return false;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
			printf("OpenGL Framebuffer error: GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT\n");
			return false;
		default:
			printf("UNKNWON error with OpenGL Framebuffer\n");
			return false;
	}

	if( bClear )
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	return true;
}

bool RFBuffer::End( int iRegVPX, int iRegVPY )
{
	
	for( unsigned i = 0; i < iNumTextures; i++ )
	{
		glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i, 
			GL_TEXTURE_2D, 0, 0 );

		glActiveTextureARB( GL_TEXTURE0_ARB + i );
		glDisable( GL_TEXTURE_2D );
	}
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
	glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, 0 );
	glViewport( 0, 0, iRegVPX, iRegVPY );
	return true;
}

VertexData::~VertexData()
{
	if( iVertexData == 0 )
		return;
	glDeleteBuffersARB( 1, &iVertexData );
}

void VertexData::Init( float * Verts, int iNumVerts, int iStride )
{
	mStride = iStride;
	glGenBuffersARB( 1, &iVertexData );
	if( !Verts )
	{
		glBindBufferARB( GL_PIXEL_PACK_BUFFER_ARB, iVertexData );
		glBufferDataARB( GL_PIXEL_PACK_BUFFER_ARB, iNumVerts*4 * sizeof( float ), 0, GL_STREAM_COPY );
		glBindBufferARB( GL_PIXEL_PACK_BUFFER_ARB, 0 );
		//glBufferDataARB( GL_ARRAY_BUFFER_ARB, iNumVerts * iStride*4, 0, GL_DYNAMIC_DRAW );
	}
	else
	{
		UpdateData( Verts, iNumVerts, iStride );
	}
}

void VertexData::UpdateData( float * Verts, int iNumVerts, int iStride )
{
	mStride = iStride;
	glBindBufferARB( GL_ARRAY_BUFFER_ARB, iVertexData  );
	glBufferDataARB( GL_ARRAY_BUFFER_ARB, iNumVerts * iStride * sizeof( float ), Verts, GL_STATIC_DRAW_ARB );
	glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );
}

#define BUFFER_OFFSET(i) ((char *)NULL + (i))
void VertexData::StripFromFrameBuffer( int iWidth, int iHeight, TextureType tt )
{
	glBindBufferARB( GL_PIXEL_PACK_BUFFER_ARB, iVertexData );
	glReadPixels( 0, 0, iWidth, iHeight, GL_RGBA, GL_FLOAT,BUFFER_OFFSET( 0 ) );
	glBindBufferARB( GL_PIXEL_PACK_BUFFER_ARB, 0 );
}

void VertexData::StripFromTexture( Texture & t )
{
	glBindTexture( GL_TEXTURE_2D, t.iTexture );
	glBindBufferARB( GL_PIXEL_PACK_BUFFER_ARB, iVertexData );
	glGetTexImage(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindBufferARB( GL_PIXEL_PACK_BUFFER_ARB, 0 );
	glBindTexture( GL_TEXTURE_2D, 0 );
}

IndexData::~IndexData()
{
	delete mIndexData;
}

void IndexData::Init( int * Indices, int iNumIndices )
{
	mIndexData = (GLuint*)Indices; mIndexCount = iNumIndices;
}

int FileReadLine( FILE * f, char * line, int iBufferSize )
{
        int iPlace = 0;

        //Read the first character.
        char cCh = fgetc( f );

        //If we have a newline, or a windows-type newline, quit.
        while( cCh != '\n' && cCh != '\r' && !feof( f ))
        {
                line[iPlace] = cCh;
                iPlace++;
                if( iPlace >= iBufferSize )
                        return 0;
                cCh = fgetc( f );
        }
        //If it's a windows-type newline (\r\n) then, throw away the true \n.
        if( cCh == '\r' )
                fgetc( f );

        line[iPlace] = '\0';

        return iPlace;
}

map< unsigned int, OpenImageFunction > m_AllLoaders;

void RegisterImageLoader( unsigned int fingerprint, OpenImageFunction f )
{
	m_AllLoaders[ fingerprint ] = f;
}

int OpenImage( const char * sImageName, unsigned char ** buffer, int * width, int * height, int * channels )
{
	unsigned int fingerprint = 0;
	char cfingerprint[3];
	FILE * f = fopen( sImageName, "rb" );
	if( !fread( cfingerprint, 1, 3, f ) )
		return 0;
	fclose( f );
	fingerprint = FINGERPRINTIZE( cfingerprint );
	if( m_AllLoaders.find( fingerprint ) != m_AllLoaders.end() )
		return (m_AllLoaders[fingerprint])(sImageName, buffer, width, height, channels );
	return 0;
}

//
//NOTE: I have no IDEA at ALL if this is being parsed right, I just exported some PPM's from GIMP
//      and this thing seems to load them all ok. (DO NOT TRUST THIS CODE TO BE RELIABLE)
//
int OpenPPM( const char * sPPMName, unsigned char ** buffer, int * width, int * height, int * channels )
{
        unsigned char Magic[2];
        char line[512];
        int maxval;
        int totalsize;

        *width = 0;
        *height = 0;

        FILE * f = fopen( sPPMName, "rb" );
        if( !f )
        {
                printf( "Could not open specified ppm, %s.\n", sPPMName );
                return 0;
        }

        if( ! fread( &Magic[0], 2, 1, f ) )
		return 0;
        if( Magic[0] != 'P' || Magic[1] != '6' )
        {
                fclose( f );
                printf( "The PPM: %s does not contain the P6 magic word.\n", sPPMName );
                return 0;
        }

        FileReadLine( f, line, 512 );

        //No idea if this part is right, I'd expect it to be wrong.  It just happens to work with gimp.
        while( FileReadLine( f, line, 512 ) && !feof( f ))
                if( sscanf( line, "%d %d", width, height ) == 2 )
                        break;

        while( FileReadLine( f, line, 512 ) && !feof( f ))
                if( sscanf( line, "%d", &maxval ) == 1 )
                        break;

        if( *width <= 0 || *width >= 8192 || *height <=0 || *height >= 8192 )
        {
                fclose( f );
                printf( "Invalid image size set for PPM: %s.  (Or PPM Read Failed.)\n", sPPMName );
                return 0;
        }

        if( maxval != 255 )
        {
                fclose( f );
                printf( "Unsupported color-byte type for PPM: %s\n", sPPMName );
                return 0;
        }

        totalsize = 3 * (*height) * (*width);
        *buffer = (unsigned char*)malloc( totalsize );
        if(! fread( *buffer, totalsize, 1, f ) )
	{
		fclose( f );
		return 0;
	}	
        fclose( f );

	if( channels )
		*channels = 3;
        return totalsize;
}

EXECUTE_BEFORE_MAIN( register_ppm, { RegisterImageLoader( FINGERPRINTIZE( "P6\n" ), OpenPPM ); } );

void DrawSquare( float minx, float miny, float maxx, float maxy )
{
	glBegin( GL_QUADS );
	glTexCoord2f( 0.0f, 0.0f );
	glVertex3f( minx, miny, 0.0f );
	glTexCoord2f( 0.0f, 1.0f );
	glVertex3f( minx, maxy, 0.0f );
	glTexCoord2f( 1.0f, 1.0f );
	glVertex3f( maxx, maxy, 0.0f );
	glTexCoord2f( 1.0f, 0.0f );
	glVertex3f( maxx, miny, 0.0f );
	glEnd();
}


void DrawGeometry( VertexData ** vd, string * vsVertexNames, int iNumVertexDatas, IndexData * id, Shader * sShader, GLint mode  )
{
	int i;
	glBindBufferARB( GL_ARRAY_BUFFER_ARB, vd[0]->iVertexData );
	glVertexPointer( vd[0]->mStride, GL_FLOAT, 0, 0 );

	if( sShader )
		for( i = 1; i < iNumVertexDatas; ++i )
		{
			int iTexPosID = glGetAttribLocationARB( sShader->GetProgramID(), vsVertexNames[i].c_str() );
			glBindBufferARB( GL_ARRAY_BUFFER_ARB, vd[i]->iVertexData );
			glEnableVertexAttribArrayARB( iTexPosID );
			glVertexAttribPointerARB( iTexPosID, vd[i]->mStride, GL_FLOAT, 0, 0, 0 );
		}


	glEnableClientState( GL_VERTEX_ARRAY );
	glDrawElements( mode, id->mIndexCount, GL_UNSIGNED_INT, id->mIndexData );
	glDisableClientState( GL_VERTEX_ARRAY );
}

void SetupForDataRun()
{
	glDisable( GL_LIGHTING );
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

void PopFromDataRun()
{
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
}

void SetupFor2D()
{
	glDisable( GL_DEPTH_TEST );
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();
	glScalef( 1.0, -1.0, 0.0 );
	glTranslatef( -1.0, -1.0, 0.0 );
	glScalef( 2.0/float(GLUT.miWidth), 2.0/float(GLUT.miHeight ), 0.0 );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
}

void PopFrom2D()
{
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
	glDisable( GL_BLEND );
	glEnable( GL_DEPTH_TEST );
}

void StripDataFromBuffer( int ix, int iy, int iwidth, int iheight, TextureType tt, char * buffer )
{
	glReadPixels( ix, iy, iwidth, iheight, imXTypes[tt], byTypes[tt], buffer );
}

void WritePPM( char *filename, char *sourceBuf, int w, int h )
{
	FILE *f = fopen( filename, "wb" );
	if( !f )
	{
		fprintf( stderr, "ZOMG could not write file %s! %s\n", filename, strerror(errno) );
		return;
	}
	fprintf( f, "P6\n%d %d\n%d\n", w, h, 255 );
	fwrite( sourceBuf, 1, w * h * 3, f );
	fclose( f );
}

//Text Functions

///General Font Utility for drawing text to the screen
class TextUtil
{
public:

	///Get a pointer to the currently loaded font by the name of sFontName
	/** This function opens both [sFontName].ini and [sFontName].ppm.  The
	    font must be in the style of a StepMania font.  The .ppm is an encoded
	    alpha -and- luminance texture.  [R G G 1.0-B] is how it gets
	    translated.  The return value is a pointer to the text that will draw. */
	static TextUtil * CheckAndLoadFont( const string & sFontName );

private:
	///Draw <this> loaded font
	void DrawFont( const char * todraw, float MaxWidth = 100000., float MaxHeight = 10000., TextAlignment Horiz = TEXT_LEFT, TextAlignment Vert = TEXT_HTOP  );

	///Load sFontName into map.
	bool LoadFont( const string & sFontName );


	string Name;
	Texture FontFull;
	vector< float > fFontWidths;
	float Top;
	float Baseline;
	float LineSpacing;

	int FontWidth;
	int FontHeight;

	static vector< TextUtil > g_AllFonts;
	friend bool DrawText( const char * text, float size, const string & sFontName, float MaxWidth, float MaxHeight, TextAlignment Horiz, TextAlignment Vert );
};

vector< TextUtil > TextUtil::g_AllFonts;

void TextUtil::DrawFont( const char * todraw, float MaxWidth, float MaxHeight, TextAlignment Horiz, TextAlignment Vert )
{
	vector< float > verts;
	vector< float > uvs;

	vector< int > allLines;

	int chars = 0;
	int Line = 0;
	float CurX = 0;
	int LastLine = 0;
	int LastSpace = 0;

	if( !*todraw )
		return;

// Begin Target  TODO: Rewrite the following code
	string outstring = todraw;
	for( const char * CurChar = outstring.c_str(); *CurChar != 0; CurChar++ )
	{
		if( *CurChar == '\n' )
		{
			LastLine = chars;
			CurX = 0;
			Line++;
			if( allLines.size() <= unsigned(chars) )
				allLines.resize( chars + 1 );
			allLines[chars] = int(Line);
			if( *CurChar == ' ' )
				LastSpace = CurChar - outstring.c_str();
			CurX += fFontWidths[*CurChar];
			if( *CurChar != ' ' )
			{
				if( CurX > MaxWidth )
				{
					if( LastSpace > LastLine )
					{
						outstring[LastSpace] = '\n';
						LastLine = LastSpace;
						CurChar = &outstring.c_str()[LastSpace];
						LastSpace = CurChar - outstring.c_str();
						CurX = 0;
					}
					else
					{
						if( CurChar > outstring.c_str() && *(CurChar-1) != '\n' )
						{
							outstring.insert( CurChar - outstring.c_str(), "\n" );
							outstring[LastSpace] = '\n';
							LastLine = LastSpace;
							CurChar = &outstring.c_str()[LastSpace];
							LastSpace = CurChar - outstring.c_str();
							CurX = 0;
						}
					}
				}
			}
			chars++;
		}
	}

	CurX = 0;
	Line = 0;
	chars = 0;

	//Now figure out all line widths and height of total text
	vector< float > m_fLineWidths;
	float fTotalHeight;
	for( const char * CurChar = outstring.c_str(); *CurChar != 0; CurChar++ )
	{
		if( *CurChar == '\n' )
		{
			m_fLineWidths.push_back( CurX );
			CurX = 0;
			Line++;
		}
		else
		{
			float CenterU, CenterV, WidthU, WidthV;
			float MaxX, MaxY, MinX, MinY;

			if( verts.size() <= unsigned((chars+1)*8) )
			{
				verts.resize( (chars+1)*16 );
				uvs.resize( (chars+1)*16 );
			}

			CenterU = float(int((*CurChar)%16))/16.0+1.0/32.0-(0.5/float(FontWidth));
			CenterV = float(int((*CurChar)/16))/16.0+1.0/32.0;
			WidthU = fFontWidths[*CurChar]/float(FontWidth)* 0.5;
			WidthV = 1.0/16.0*0.5;

			MinX = CurX;
			CurX += fFontWidths[*CurChar];

			MaxX = CurX;
			MinY = Line*LineSpacing;
			MaxY = Line*LineSpacing+(FontWidth/16);

			chars++;
		}
	}
	m_fLineWidths.push_back( CurX );
	fTotalHeight = Line*LineSpacing+(FontWidth/16);


	CurX = 0;
	Line = 0;
	chars = 0;
//Target Point
	for( const char * CurChar = outstring.c_str(); *CurChar != 0; CurChar++ )
	{
		if( *CurChar == '\n' )
		{
			CurX = 0;
			Line++;
		}
		else
		{
			float CenterU, CenterV, WidthU, WidthV;
			float MaxX, MaxY, MinX, MinY;

			if( verts.size() <= unsigned((chars+1)*8) )
			{
				verts.resize( (chars+1)*16 );
				uvs.resize( (chars+1)*16 );
			}

			CenterU = float(int((*CurChar)%16))/16.0+1.0/32.0-(0.5/float(FontWidth));
			CenterV = float(int((*CurChar)/16))/16.0+1.0/32.0;
			WidthU = fFontWidths[*CurChar]/float(FontWidth)* 0.5;
			WidthV = 1.0/16.0*0.5;

			//printf( "%d\n", Hori) ;
			MinX = CurX + ((Horiz==TEXT_LEFT)?0:((Horiz==TEXT_CENTER)?(-m_fLineWidths[Line]/2.):(-m_fLineWidths[Line])));
			CurX += fFontWidths[*CurChar];
			MaxX = CurX + ((Horiz==TEXT_LEFT)?0:((Horiz==TEXT_CENTER)?(-m_fLineWidths[Line]/2.):(-m_fLineWidths[Line])));

			MinY = Line*LineSpacing - ((Vert==TEXT_HCENTER)?(fTotalHeight/2.):0.0);
			MaxY = Line*LineSpacing+(FontWidth/16) - ((Vert==TEXT_HCENTER)?(fTotalHeight/2.):0.0);

			verts[chars*8+0] = MinX;
			verts[chars*8+1] = MinY;
			verts[chars*8+2] = MaxX;
			verts[chars*8+3] = MinY;
			verts[chars*8+4] = MaxX;
			verts[chars*8+5] = MaxY;
			verts[chars*8+6] = MinX;
			verts[chars*8+7] = MaxY;

			uvs[chars*8+0] = CenterU - WidthU;
			uvs[chars*8+1] = CenterV - WidthV;
			uvs[chars*8+2] = CenterU + WidthU;
			uvs[chars*8+3] = CenterV - WidthV;
			uvs[chars*8+4] = CenterU + WidthU;
			uvs[chars*8+5] = CenterV + WidthV;
			uvs[chars*8+6] = CenterU - WidthU;
			uvs[chars*8+7] = CenterV + WidthV;
			chars++;
		}
	}

	FontFull.ActivateTexture( 0 );
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	glTexCoordPointer( 2, GL_FLOAT, 0, &uvs[0] );
	glVertexPointer( 2, GL_FLOAT, 0, &verts[0] );
	glDrawArrays( GL_QUADS, 0, chars * 4 );
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	FontFull.DeactivateTexture( 0 );
}

bool DrawText( const char * text, float size, const string & sFontName, float MaxWidth, float MaxHeight, TextAlignment Horiz, TextAlignment Vert )
{
	TextUtil * t = TextUtil::CheckAndLoadFont( sFontName );
	if( t )
	{
		glPushMatrix();
		glScalef( size, size, 1.0 );
		t->DrawFont( text, MaxWidth/size, MaxHeight/size, Horiz, Vert );
		glPopMatrix();
		return true;
	}
	else
		return false;
}

TextUtil * TextUtil::CheckAndLoadFont( const string & sFontName )
{
	for( unsigned i = 0; i < g_AllFonts.size(); i++ )
		if( g_AllFonts[i].Name == sFontName )
			return &g_AllFonts[i];

	g_AllFonts.resize( g_AllFonts.size() + 1 );
	if( g_AllFonts[g_AllFonts.size()-1].LoadFont( sFontName ) )
		return &g_AllFonts[g_AllFonts.size()-1];

	
	g_AllFonts.resize( g_AllFonts.size() - 1 );
	return 0;
}

bool TextUtil::LoadFont( const string & sFontName )
{
	unsigned char * RawLoadedData;
	unsigned int * FontMapData;
	char line[256];
	float AddToAll = 0;
	FILE * f;

	if( !( f = fopen( (sFontName+".ini").c_str(), "rb" ) ) )
	{
		printf( "Could not open font INI file '%s'\n", (sFontName+".ini").c_str() );
		return false;
	}

	if( !OpenImage( (sFontName+".ppm").c_str(), &RawLoadedData, &FontHeight, &FontWidth ) )
	{
		printf( "Could not open font PPM file '%s'\n", (sFontName+".ppm").c_str() );
		fclose( f );
		return false;
	}
	FontMapData = (unsigned int*)malloc( FontWidth * FontHeight * 4 );
	for( int y = 0; y < FontHeight; y++ )
		for( int x = 0; x < FontWidth; x++ )
		{
			//printf( "%d %d %d\n", x, y, x+y*FontWidth );
			FontMapData[x+y*FontWidth] = 
				((unsigned int)RawLoadedData[(x+y*FontWidth)*3+0])+
				((unsigned int)RawLoadedData[(x+y*FontWidth)*3+1]<<8) +
				((unsigned int)RawLoadedData[(x+y*FontWidth)*3+1]<<16) +
				((255-(unsigned int)RawLoadedData[(x+y*FontWidth)*3+2])<<24);
		}
	free( RawLoadedData);

	FontFull.LoadTexture( (char*)FontMapData, FontWidth, FontHeight, TTRGBA, false );

	glFlush();
	free( FontMapData );

	fFontWidths.resize(256);
	while( FileReadLine( f, line, 128 ) >= 0 && !feof( f ) )
	{
		char * equals = strchr( line, '=' );
		if( !equals )
			continue;
		equals[0] = '\0';
		equals++;

		int character;
		if( sscanf( line, "%d", &character ) == 1 )
		{
			if( character > 255 )
			{
				printf( "Error.  Malformed property '%s' in font '%s'\n", line, sFontName.c_str() );
				continue;
			}
			fFontWidths[character] = atof( equals );
		} 
		else if( strcmp( line, "AddToAllWidths" ) == 0 )
			AddToAll = atof( equals );
		else if( strcmp( line, "Top" ) == 0 )
			Top = atof( equals );
		else if( strcmp( line, "Baseline" ) == 0 )
			Baseline = atof( equals );
		else if( strcmp( line, "LineSpacing" ) == 0 )
			LineSpacing = atof( equals );
	}
	fclose( f );
	Name = sFontName;
	for( unsigned i = 0; i < 256; i++ )
		fFontWidths[i] += AddToAll;

	return true;
}

Overridedfopen overridedfopen;

#undef fopen

FILE * cppgpgpufopen( const char * fn, const char * mode )
{
	FILE * ret;
	if( overridedfopen )
	{
		ret = overridedfopen( fn, mode );
		if( ret )
			return ret;
	}

	return fopen( fn, mode );
}

void SetOverrideFOpenFunction( Overridedfopen fo )
{
	overridedfopen = fo;
}

/* INCOMPLETE

bool ProcessFileToString( const char * file, string & ret, int depth )
{
#define LINE_MAX 1024
	char line[LINE_MAX];
	int lineno = 0;

	FILE * f = fopen( file, "rb" );
	if( !f )
	{
		fprintf( stderr, "Could not open \"%s\"\n", file );
		return false;
	}

	while( !feof( f ) && !ferror( f ) )
	{
		lineno++;
		char * l = fgets( line, LINE_MAX-2, f );
		if( !l )
		{
			fprintf( stderr, "Could not read line %s:%d\n", file, lineno );
			return false;
		}

		unsigned long linelen = srlen( line );
		unsigned long ilp;
		for( ilp = 0; ilp < linelen; ilp++ )
		{
			if( ilp != '\n' && ilp != '\t' && ilp != ' ' ) break;
		}
		if( ilp < linelen )
		{
			if( strncmp( l+ilp, "#include ", 8 ) == 0 )
			{
			}
		}
		else
		{
			ret += l;
		}
	}
	fclose( f );
}
*/

/*
Copyright (c) 2005-2008 Charles Lohr, David Chapman, Joshua Allen

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
