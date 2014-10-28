#include <stdio.h>
#include "OGLParts.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <map>
#include <errno.h>
#include <fstream>
#include "scripthelpers.h"

using namespace std;

GLuint imTypes[6] = {GL_LUMINANCE8, GL_RGB, GL_RGBA, GL_RGBA16F, GL_RGBA32F, 0};
int channels[6] = {1, 3, 4, 4, 4, 0};
//For things that require GL_RGBA when dealing with floating point data; usually when dealing with verticies or host data.
GLuint imXTypes[6] = {GL_LUMINANCE, GL_RGB, GL_RGBA, GL_RGBA, GL_RGBA, 0};
GLuint byTypes[6] = {GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_FLOAT, GL_FLOAT, 0};

void DestroyShaderProgram(GLuint iProgramID) {
    GLint count = -1;

    if (!iProgramID)
        return;
    glGetProgramiv(iProgramID, GL_ATTACHED_SHADERS, &count);
    GLuint objects[count];
    //Iterate through all children.
    if (count > 0) {
        glGetAttachedShaders(iProgramID, count, NULL, objects);
    } else return;

    for (GLuint obj : objects) glDetachShader(iProgramID, obj);
    glDeleteShader(iProgramID);
    return;
}

Shader::Shader(string sShaderName, string preamble) : sShaderName(sShaderName), preamble(preamble) {
    fileChanged(sShaderName+".vert");
    fileChanged(sShaderName+".frag");
}

Shader::~Shader() {
    DestroyShaderProgram(iProgramID);
}

void Shader::LoadIfNewer() {
    if (fileChanged(sShaderName+".vert") || fileChanged(sShaderName+".frag")) {
        DestroyShaderProgram(iProgramID);
        LoadShader();
    }
}

bool Shader::LoadShader() {
    cout << "Compiling " << sShaderName << endl;
    if (!LoadShader(GL_FRAGMENT_SHADER, fragmentShader, preamble + readFile(sShaderName + ".frag"))) {
        printf("Loading fragment shader failed. Not linking.\n");
        return false;
    }
    if (!LoadShader(GL_VERTEX_SHADER, vertexShader, preamble + readFile(sShaderName + ".vert"))) {
        printf("Loading vertex shader failed. Not linking.\n");
        return false;
    }
    return LinkShaders();
}

// shaderType=GL_FRAGMENT_SHADER or GL_VERTEX_SHADER
bool Shader::LoadShader(int shaderType, GLuint& target, string sShaderCode) {
    if (sShaderCode.size() < 5) return false;
    GLint bCompiled;
    GLint stringLength;
    target = glCreateShader(shaderType);
    //Bind the shader to the text, setting that to be its source.
    const char* sShaderCodeChars = sShaderCode.c_str();
    glShaderSource(target, 1, &sShaderCodeChars, NULL);
    glCompileShader(target);
    //Did the shader compile?  Were there any errors?
    glGetShaderiv(target, GL_COMPILE_STATUS, &bCompiled);
    glGetShaderiv(target, GL_INFO_LOG_LENGTH, &stringLength);
    if (stringLength > 1 || !bCompiled) {
        char errorLog[stringLength+1];
        glGetShaderInfoLog(target, stringLength, NULL, errorLog);
        cout << "Compiling Shader response follows:" << errorLog;
        return bCompiled != 0;
    }
    return true;
}

bool Shader::LinkShaders() {
    GLint bLinked;
    GLint stringLength;
    //Create the actual shader prgoram
    iProgramID = glCreateProgram();
    printf("Linking Shaders. %d <- %d + %d + %d\n", iProgramID, vertexShader, fragmentShader, geometryShader);
    //Attach the fragment/vertex shader to it.
    if (vertexShader)
        glAttachShader(iProgramID, vertexShader);
    if (fragmentShader)
        glAttachShader(iProgramID, fragmentShader);
    if (geometryShader)
        glAttachShader(iProgramID, geometryShader);
    //Attempt to link the shader
    glLinkProgram(iProgramID);
    printf("Shaders Linked\n");


    //See if there were any errors.
    glGetProgramiv(iProgramID, GL_LINK_STATUS, &bLinked);
    glGetProgramiv(iProgramID, GL_INFO_LOG_LENGTH, &stringLength);
    printf("Shaders Linked.\n");
    if (stringLength > 1 || !bLinked) {
        char errorLog[stringLength+1];
        glGetProgramInfoLog(iProgramID, stringLength, NULL, errorLog);
        cout << "Linking shaders response follows:" << errorLog;
        return bLinked != 0;
    }
    return true;
}

bool Shader::ActivateShader(vector<string> &vsAllSamplerLocs, vector<pair<string,float>> &vUniformFloats) {
    glUseProgram(iProgramID);
    ActivateShader(vsAllSamplerLocs);
    for (pair<string,float>& v:vUniformFloats) {
        int iTexPosID = glGetUniformLocation(iProgramID, v.first.c_str());
        if (iTexPosID > -1) glUniform1f(iTexPosID, v.second);
    }
    return true;
}

bool Shader::ActivateShader(vector<string> &vsAllSamplerLocs) {
    glUseProgram(iProgramID);

    for (unsigned i = 0; i < vsAllSamplerLocs.size(); ++i) {
        int iTexPosID = glGetUniformLocation(iProgramID, vsAllSamplerLocs[i].c_str());
        if (iTexPosID > -1)
            glUniform1i(iTexPosID, i);
    }

    return true;
}

bool Shader::DeactivateShader() {
    glUseProgram(0);
    return true;
}

Texture::~Texture() {
    if ((int) iTexture == -1)
        return;
    glDeleteTextures(1, &iTexture);
}

bool Texture::MakeDynamicTexture(int iWidth, int iHeight, TextureType tt) {
    mtt = tt;
    mWidth = iWidth;
    mHeight = iHeight;

    if ((int) iTexture == -1)
        glGenTextures(1, &iTexture);
    if ((int) iTexture == -1)
        return false;

    glBindTexture(iTextureType, iTexture);
    glCopyTexImage2D(iTextureType, 0, imTypes[tt], 0, 0, iWidth, iHeight, 0);
    glTexParameteri(iTextureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(iTextureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(iTextureType, 0);
    return true;
}

bool Texture::LoadTexture(char * sRawData, int iWidth, int iHeight, TextureType tt, bool bWithMipMaps) {
    if ((int) iTexture == -1)
        glGenTextures(1, &iTexture);
    if ((int) iTexture == -1)
        return false;

    mtt = tt;
    mWidth = iWidth;
    mHeight = iHeight;

    glBindTexture(iTextureType, iTexture);

    if (iTextureType == GL_TEXTURE_2D) {
        if (bWithMipMaps) {
            gluBuild2DMipmaps(iTextureType, channels[mtt], mWidth, mHeight, imTypes[mtt], byTypes[mtt], sRawData);
            glTexParameterf(iTextureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameterf(iTextureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        } else {
            glTexImage2D(iTextureType, 0, imTypes[mtt], mWidth, mHeight, 0, imXTypes[mtt], byTypes[mtt], sRawData);
            glTexParameteri(iTextureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(iTextureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
    } else {

        glTexImage3D(iTextureType, 0, imTypes[mtt], mWidth, mWidth, mHeight, 0, imXTypes[mtt], byTypes[mtt], sRawData);
        glTexParameteri(iTextureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(iTextureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
    }
    glBindTexture(iTextureType, 0);
    return true;
}

bool Texture::ActivateTexture(int iPlace) {
    glActiveTexture(GL_TEXTURE0 + iPlace);
    glBindTexture(iTextureType, iTexture);
    glEnable(iTextureType);
    return true;
}

bool Texture::DeactivateTexture(int iPlace) {
    glActiveTexture(GL_TEXTURE0 + iPlace);
    glDisable(iTextureType);
    return true;
}

RFBuffer::RFBuffer() {
    mWidth = 0;
    mHeight = 0;
    mtt = TTGRAYSCALE;
    iRenderbuffer = 0;
    iOutputBuffer = 0;
}

RFBuffer::~RFBuffer() {
    if (iRenderbuffer != 0) {
        glDeleteRenderbuffers(1, &iRenderbuffer);
        glDeleteFramebuffers(1, &iOutputBuffer);
    }
}

bool RFBuffer::Setup(bool bUseDepthBuffer) {
    m_bUseDepthBuffer = bUseDepthBuffer;
    if (m_bUseDepthBuffer)
        glGenRenderbuffers(1, &iRenderbuffer);
    glGenFramebuffers(1, &iOutputBuffer);
    return true;
}

bool RFBuffer::ConfigureAndStart(int iWidth, int iHeight, int iTextures, Texture * TexturesToAttach, bool bClear) {
    const GLenum buffers[8] = {GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT,
        GL_COLOR_ATTACHMENT3_EXT, GL_COLOR_ATTACHMENT4_EXT, GL_COLOR_ATTACHMENT5_EXT,
        GL_COLOR_ATTACHMENT6_EXT, GL_COLOR_ATTACHMENT7_EXT};
    mWidth = iWidth;
    mHeight = iHeight;
    iNumTextures = iTextures;
    if (m_bUseDepthBuffer) {
        glBindRenderbuffer(GL_RENDERBUFFER_EXT, iRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, mWidth, mHeight);
    }
    glBindFramebuffer(GL_FRAMEBUFFER_EXT, iOutputBuffer);
    for (int i = 0; i < iTextures; i++)
        glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i,
            GL_TEXTURE_2D, TexturesToAttach[i].GetTexHandle(), 0);
    if (m_bUseDepthBuffer)
        glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT,
            GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, iRenderbuffer);

    //Added by Peter Bessman to verify nonexecution of the following code.
    glDrawBuffers(iTextures, buffers);
    glViewport(0, 0, mWidth, mHeight);

    //Check to see if there were any errors with the framebuffer
    switch ((GLenum) glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT)) {
        case GL_FRAMEBUFFER_COMPLETE_EXT: break; //GOOD!
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

    if (bClear)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    return true;
}

bool RFBuffer::End(int iRegVPX, int iRegVPY) {

    for (unsigned i = 0; i < iNumTextures; i++) {
        glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i,
                GL_TEXTURE_2D, 0, 0);

        glActiveTexture(GL_TEXTURE0 + i);
        glDisable(GL_TEXTURE_2D);
    }
    glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
    glBindRenderbuffer(GL_RENDERBUFFER_EXT, 0);
    glViewport(0, 0, iRegVPX, iRegVPY);
    return true;
}

VertexData::~VertexData() {
    if (iVertexData == 0)
        return;
    glDeleteBuffers(1, &iVertexData);
}

void VertexData::Init(float * Verts, int iNumVerts, int iStride) {
    mStride = iStride;
    glGenBuffers(1, &iVertexData);
    if (!Verts) {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, iVertexData);
        glBufferData(GL_PIXEL_PACK_BUFFER, iNumVerts * 4 * sizeof ( float), 0, GL_STREAM_COPY);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    } else {
        UpdateData(Verts, iNumVerts, iStride);
    }
}

void VertexData::UpdateData(float * Verts, int iNumVerts, int iStride) {
    mStride = iStride;
    glBindBuffer(GL_ARRAY_BUFFER, iVertexData);
    glBufferData(GL_ARRAY_BUFFER, iNumVerts * iStride * sizeof ( float), Verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

IndexData::~IndexData() {
    delete mIndexData;
}

void IndexData::Init(int * Indices, int iNumIndices) {
    mIndexData = (GLuint*) Indices;
    mIndexCount = iNumIndices;
}

int FileReadLine(FILE * f, char * line, int iBufferSize) {
    int iPlace = 0;

    //Read the first character.
    char cCh = fgetc(f);

    //If we have a newline, or a windows-type newline, quit.
    while (cCh != '\n' && cCh != '\r' && !feof(f)) {
        line[iPlace] = cCh;
        iPlace++;
        if (iPlace >= iBufferSize)
            return 0;
        cCh = fgetc(f);
    }
    //If it's a windows-type newline (\r\n) then, throw away the true \n.
    if (cCh == '\r')
        fgetc(f);

    line[iPlace] = '\0';

    return iPlace;
}

map< unsigned int, OpenImageFunction > m_AllLoaders;

void RegisterImageLoader(unsigned int fingerprint, OpenImageFunction f) {
    m_AllLoaders[ fingerprint ] = f;
}

int OpenImage(const char * sImageName, byte ** buffer, int * width, int * height, int * channels) {
    unsigned int fingerprint = 0;
    char cfingerprint[3];
    FILE * f = fopen(sImageName, "rb");
    if (!fread(cfingerprint, 1, 3, f))
        return 0;
    fclose(f);
    fingerprint = FINGERPRINTIZE(cfingerprint);
    if (m_AllLoaders.find(fingerprint) != m_AllLoaders.end())
        return (m_AllLoaders[fingerprint])(sImageName, buffer, width, height, channels);
    return 0;
}

//
//NOTE: I have no IDEA at ALL if this is being parsed right, I just exported some PPM's from GIMP
//      and this thing seems to load them all ok. (DO NOT TRUST THIS CODE TO BE RELIABLE)
//

int OpenPPM(const char * sPPMName, byte ** buffer, int * width, int * height, int * channels) {
    byte Magic[2];
    char line[512];
    int maxval;
    int totalsize;

    *width = 0;
    *height = 0;

    FILE * f = fopen(sPPMName, "rb");
    if (!f) {
        printf("Could not open specified ppm, %s.\n", sPPMName);
        return 0;
    }

    if (!fread(&Magic[0], 2, 1, f))
        return 0;
    if (Magic[0] != 'P' || Magic[1] != '6') {
        fclose(f);
        printf("The PPM: %s does not contain the P6 magic word.\n", sPPMName);
        return 0;
    }

    FileReadLine(f, line, 512);

    //No idea if this part is right, I'd expect it to be wrong.  It just happens to work with gimp.
    while (FileReadLine(f, line, 512) && !feof(f))
        if (sscanf(line, "%d %d", width, height) == 2)
            break;

    while (FileReadLine(f, line, 512) && !feof(f))
        if (sscanf(line, "%d", &maxval) == 1)
            break;

    if (*width <= 0 || *width >= 8192 || *height <= 0 || *height >= 8192) {
        fclose(f);
        printf("Invalid image size set for PPM: %s.  (Or PPM Read Failed.)\n", sPPMName);
        return 0;
    }

    if (maxval != 255) {
        fclose(f);
        printf("Unsupported color-byte type for PPM: %s\n", sPPMName);
        return 0;
    }

    totalsize = 3 * (*height) * (*width);
    *buffer = (byte*) malloc(totalsize);
    if (!fread(*buffer, totalsize, 1, f)) {
        fclose(f);
        return 0;
    }
    fclose(f);

    if (channels)
        *channels = 3;
    return totalsize;
}

EXECUTE_BEFORE_MAIN(register_ppm,{RegisterImageLoader(FINGERPRINTIZE("P6\n"), OpenPPM);});



//Text Functions

///General Font Utility for drawing text to the screen

class TextUtil {
public:

    ///Get a pointer to the currently loaded font by the name of sFontName
    /** This function opens both [sFontName].ini and [sFontName].ppm.  The
        font must be in the style of a StepMania font.  The .ppm is an encoded
        alpha -and- luminance texture.  [R G G 1.0-B] is how it gets
        translated.  The return value is a pointer to the text that will draw. */
    static TextUtil * CheckAndLoadFont(const string & sFontName);

private:
    ///Draw <this> loaded font
    void DrawFont(const char * todraw, float MaxWidth = 100000., float MaxHeight = 10000., TextAlignment Horiz = TEXT_LEFT, TextAlignment Vert = TEXT_HTOP);

    ///Load sFontName into map.
    bool LoadFont(const string & sFontName);


    string Name;
    Texture FontFull;
    vector< float > fFontWidths;
    float Top;
    float Baseline;
    float LineSpacing;

    int FontWidth;
    int FontHeight;

    static vector< TextUtil > g_AllFonts;
    friend bool DrawText(const char * text, float size, const string & sFontName, float MaxWidth, float MaxHeight, TextAlignment Horiz, TextAlignment Vert);
};

vector< TextUtil > TextUtil::g_AllFonts;

void TextUtil::DrawFont(const char * todraw, float MaxWidth, float MaxHeight, TextAlignment Horiz, TextAlignment Vert) {
    vector< float > verts;
    vector< float > uvs;

    vector< int > allLines;

    int chars = 0;
    int Line = 0;
    float CurX = 0;
    int LastLine = 0;
    int LastSpace = 0;

    if (!*todraw)
        return;

    // Begin Target  TODO: Rewrite the following code
    string outstring = todraw;
    for (const char * CurChar = outstring.c_str(); *CurChar != 0; CurChar++) {
        if (*CurChar == '\n') {
            LastLine = chars;
            CurX = 0;
            Line++;
            if (allLines.size() <= unsigned(chars))
                allLines.resize(chars + 1);
            allLines[chars] = int(Line);
            if (*CurChar == ' ')
                LastSpace = CurChar - outstring.c_str();
            CurX += fFontWidths[*CurChar];
            if (*CurChar != ' ') {
                if (CurX > MaxWidth) {
                    if (LastSpace > LastLine) {
                        outstring[LastSpace] = '\n';
                        LastLine = LastSpace;
                        CurChar = &outstring.c_str()[LastSpace];
                        LastSpace = CurChar - outstring.c_str();
                        CurX = 0;
                    } else {
                        if (CurChar > outstring.c_str() && *(CurChar - 1) != '\n') {
                            outstring.insert(CurChar - outstring.c_str(), "\n");
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
    for (const char * CurChar = outstring.c_str(); *CurChar != 0; CurChar++) {
        if (*CurChar == '\n') {
            m_fLineWidths.push_back(CurX);
            CurX = 0;
            Line++;
        } else {
            if (verts.size() <= unsigned((chars + 1)*8)) {
                verts.resize((chars + 1)*16);
                uvs.resize((chars + 1)*16);
            }

            CurX += fFontWidths[*CurChar];

            chars++;
        }
    }
    m_fLineWidths.push_back(CurX);
    fTotalHeight = Line * LineSpacing + (FontWidth / 16);


    CurX = 0;
    Line = 0;
    chars = 0;
    //Target Point
    for (const char * CurChar = outstring.c_str(); *CurChar != 0; CurChar++) {
        if (*CurChar == '\n') {
            CurX = 0;
            Line++;
        } else {
            float CenterU, CenterV, WidthU, WidthV;
            float MaxX, MaxY, MinX, MinY;

            if (verts.size() <= unsigned((chars + 1)*8)) {
                verts.resize((chars + 1)*16);
                uvs.resize((chars + 1)*16);
            }

            CenterU = float(int((*CurChar) % 16)) / 16.0 + 1.0 / 32.0 - (0.5 / float(FontWidth));
            CenterV = float(int((*CurChar) / 16)) / 16.0 + 1.0 / 32.0;
            WidthU = fFontWidths[*CurChar] / float(FontWidth)* 0.5;
            WidthV = 1.0 / 16.0 * 0.5;

            //printf( "%d\n", Hori) ;
            MinX = CurX + ((Horiz == TEXT_LEFT) ? 0 : ((Horiz == TEXT_CENTER) ? (-m_fLineWidths[Line] / 2.) : (-m_fLineWidths[Line])));
            CurX += fFontWidths[*CurChar];
            MaxX = CurX + ((Horiz == TEXT_LEFT) ? 0 : ((Horiz == TEXT_CENTER) ? (-m_fLineWidths[Line] / 2.) : (-m_fLineWidths[Line])));

            MinY = Line * LineSpacing - ((Vert == TEXT_HCENTER) ? (fTotalHeight / 2.) : 0.0);
            MaxY = Line * LineSpacing + (FontWidth / 16) - ((Vert == TEXT_HCENTER) ? (fTotalHeight / 2.) : 0.0);

            verts[chars * 8 + 0] = MinX;
            verts[chars * 8 + 1] = MinY;
            verts[chars * 8 + 2] = MaxX;
            verts[chars * 8 + 3] = MinY;
            verts[chars * 8 + 4] = MaxX;
            verts[chars * 8 + 5] = MaxY;
            verts[chars * 8 + 6] = MinX;
            verts[chars * 8 + 7] = MaxY;

            uvs[chars * 8 + 0] = CenterU - WidthU;
            uvs[chars * 8 + 1] = CenterV - WidthV;
            uvs[chars * 8 + 2] = CenterU + WidthU;
            uvs[chars * 8 + 3] = CenterV - WidthV;
            uvs[chars * 8 + 4] = CenterU + WidthU;
            uvs[chars * 8 + 5] = CenterV + WidthV;
            uvs[chars * 8 + 6] = CenterU - WidthU;
            uvs[chars * 8 + 7] = CenterV + WidthV;
            chars++;
        }
    }

    FontFull.ActivateTexture(0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, &uvs[0]);
    glVertexPointer(2, GL_FLOAT, 0, &verts[0]);
    glDrawArrays(GL_QUADS, 0, chars * 4);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    FontFull.DeactivateTexture(0);
}

bool DrawText(const char * text, float size, const string & sFontName, float MaxWidth, float MaxHeight, TextAlignment Horiz, TextAlignment Vert) {
    TextUtil * t = TextUtil::CheckAndLoadFont(sFontName);
    if (t) {
        glPushMatrix();
        glScalef(size, size, 1.0);
        t->DrawFont(text, MaxWidth / size, MaxHeight / size, Horiz, Vert);
        glPopMatrix();
        return true;
    } else
        return false;
}

TextUtil * TextUtil::CheckAndLoadFont(const string & sFontName) {
    for (unsigned i = 0; i < g_AllFonts.size(); i++)
        if (g_AllFonts[i].Name == sFontName)
            return &g_AllFonts[i];

    g_AllFonts.resize(g_AllFonts.size() + 1);
    if (g_AllFonts[g_AllFonts.size() - 1].LoadFont(sFontName))
        return &g_AllFonts[g_AllFonts.size() - 1];


    g_AllFonts.resize(g_AllFonts.size() - 1);
    return 0;
}

bool TextUtil::LoadFont(const string & sFontName) {
    byte * RawLoadedData;
    unsigned int * FontMapData;
    char line[256];
    float AddToAll = 0;
    FILE * f;

    if (!(f = fopen((sFontName + ".ini").c_str(), "rb"))) {
        printf("Could not open font INI file '%s'\n", (sFontName + ".ini").c_str());
        return false;
    }

    if (!OpenImage((sFontName + ".ppm").c_str(), &RawLoadedData, &FontHeight, &FontWidth)) {
        printf("Could not open font PPM file '%s'\n", (sFontName + ".ppm").c_str());
        fclose(f);
        return false;
    }
    FontMapData = (unsigned int*) malloc(FontWidth * FontHeight * 4);
    for (int y = 0; y < FontHeight; y++)
        for (int x = 0; x < FontWidth; x++) {
            //printf( "%d %d %d\n", x, y, x+y*FontWidth );
            FontMapData[x + y * FontWidth] =
                    ((unsigned int) RawLoadedData[(x + y * FontWidth)*3 + 0])+
                    ((unsigned int) RawLoadedData[(x + y * FontWidth)*3 + 1] << 8) +
                    ((unsigned int) RawLoadedData[(x + y * FontWidth)*3 + 1] << 16) +
                    ((255 - (unsigned int) RawLoadedData[(x + y * FontWidth)*3 + 2]) << 24);
        }
    free(RawLoadedData);

    FontFull.LoadTexture((char*) FontMapData, FontWidth, FontHeight, TTRGBA, false);

    glFlush();
    free(FontMapData);

    fFontWidths.resize(256);
    while (FileReadLine(f, line, 128) >= 0 && !feof(f)) {
        char * equals = strchr(line, '=');
        if (!equals)
            continue;
        equals[0] = '\0';
        equals++;

        int character;
        if (sscanf(line, "%d", &character) == 1) {
            if (character > 255) {
                printf("Error.  Malformed property '%s' in font '%s'\n", line, sFontName.c_str());
                continue;
            }
            fFontWidths[character] = atof(equals);
        } else if (strcmp(line, "AddToAllWidths") == 0)
            AddToAll = atof(equals);
        else if (strcmp(line, "Top") == 0)
            Top = atof(equals);
        else if (strcmp(line, "Baseline") == 0)
            Baseline = atof(equals);
        else if (strcmp(line, "LineSpacing") == 0)
            LineSpacing = atof(equals);
    }
    fclose(f);
    Name = sFontName;
    for (unsigned i = 0; i < 256; i++)
        fFontWidths[i] += AddToAll;

    return true;
}

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
