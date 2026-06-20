/*
    GLAD OpenGL Loader - Implementation
    
    This is a minimal implementation that loads OpenGL function pointers.
    For production use, generate a proper glad loader from https://glad.dav1d.de/
*/

#include <glad/glad.h>

#ifdef _WIN32
#include <windows.h>
static HMODULE libGL;

static void* get_proc(const char* name) {
    void* p = (void*)wglGetProcAddress(name);
    if (p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1)) {
        p = (void*)GetProcAddress(libGL, name);
    }
    return p;
}

static int open_gl(void) {
    libGL = LoadLibraryW(L"opengl32.dll");
    return libGL != NULL;
}

static void close_gl(void) {
    if (libGL) {
        FreeLibrary(libGL);
        libGL = NULL;
    }
}
#else
#include <dlfcn.h>
static void* libGL;

static void* get_proc(const char* name) {
    return dlsym(libGL, name);
}

static int open_gl(void) {
    libGL = dlopen("libGL.so.1", RTLD_LAZY | RTLD_GLOBAL);
    if (!libGL) libGL = dlopen("libGL.so", RTLD_LAZY | RTLD_GLOBAL);
    return libGL != NULL;
}

static void close_gl(void) {
    if (libGL) {
        dlclose(libGL);
        libGL = NULL;
    }
}
#endif

/* Function pointer definitions */
PFNGLCLEARCOLORPROC glClearColor;
PFNGLCLEARPROC glClear;
PFNGLVIEWPORTPROC glViewport;
PFNGLENABLEPROC glEnable;
PFNGLDISABLEPROC glDisable;
PFNGLBLENDFUNCPROC glBlendFunc;
PFNGLCULLFACEPROC glCullFace;
PFNGLPOLYGONMODEPROC glPolygonMode;
PFNGLGETSTRINGPROC glGetString;
PFNGLDRAWARRAYSPROC glDrawArrays;
PFNGLDRAWELEMENTSPROC glDrawElements;
PFNGLBINDTEXTUREPROC glBindTexture;
PFNGLDELETETEXTURESPROC glDeleteTextures;
PFNGLTEXIMAGE2DPROC glTexImage2D;
PFNGLTEXPARAMETERIPROC glTexParameteri;

PFNGLCREATEBUFFERSPROC glCreateBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLBUFFERSUBDATAPROC glBufferSubData;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;

PFNGLCREATEVERTEXARRAYSPROC glCreateVertexArrays;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLVERTEXATTRIBIPOINTERPROC glVertexAttribIPointer;
PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;

PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLDELETEPROGRAMPROC glDeleteProgram;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;

PFNGLUNIFORM1IPROC glUniform1i;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM2FVPROC glUniform2fv;
PFNGLUNIFORM3FVPROC glUniform3fv;
PFNGLUNIFORM4FVPROC glUniform4fv;
PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;

PFNGLCREATETEXTURESPROC glCreateTextures;
PFNGLTEXTURESTORAGE2DPROC glTextureStorage2D;
PFNGLTEXTURESUBIMAGE2DPROC glTextureSubImage2D;
PFNGLGENERATETEXTUREMIPMAPPROC glGenerateTextureMipmap;
PFNGLTEXTUREPARAMETERIPROC glTextureParameteri;
PFNGLBINDTEXTUREUNITPROC glBindTextureUnit;

PFNGLCREATEFRAMEBUFFERSPROC glCreateFramebuffers;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;

static void load_gl_functions(GLADloadproc load) {
    glClearColor = (PFNGLCLEARCOLORPROC)load("glClearColor");
    glClear = (PFNGLCLEARPROC)load("glClear");
    glViewport = (PFNGLVIEWPORTPROC)load("glViewport");
    glEnable = (PFNGLENABLEPROC)load("glEnable");
    glDisable = (PFNGLDISABLEPROC)load("glDisable");
    glBlendFunc = (PFNGLBLENDFUNCPROC)load("glBlendFunc");
    glCullFace = (PFNGLCULLFACEPROC)load("glCullFace");
    glPolygonMode = (PFNGLPOLYGONMODEPROC)load("glPolygonMode");
    glGetString = (PFNGLGETSTRINGPROC)load("glGetString");
    glDrawArrays = (PFNGLDRAWARRAYSPROC)load("glDrawArrays");
    glDrawElements = (PFNGLDRAWELEMENTSPROC)load("glDrawElements");
    glBindTexture = (PFNGLBINDTEXTUREPROC)load("glBindTexture");
    glDeleteTextures = (PFNGLDELETETEXTURESPROC)load("glDeleteTextures");
    glTexImage2D = (PFNGLTEXIMAGE2DPROC)load("glTexImage2D");
    glTexParameteri = (PFNGLTEXPARAMETERIPROC)load("glTexParameteri");

    glCreateBuffers = (PFNGLCREATEBUFFERSPROC)load("glCreateBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC)load("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)load("glBufferData");
    glBufferSubData = (PFNGLBUFFERSUBDATAPROC)load("glBufferSubData");
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)load("glDeleteBuffers");

    glCreateVertexArrays = (PFNGLCREATEVERTEXARRAYSPROC)load("glCreateVertexArrays");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)load("glBindVertexArray");
    glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)load("glDeleteVertexArrays");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)load("glEnableVertexAttribArray");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)load("glVertexAttribPointer");
    glVertexAttribIPointer = (PFNGLVERTEXATTRIBIPOINTERPROC)load("glVertexAttribIPointer");
    glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISORPROC)load("glVertexAttribDivisor");

    glCreateShader = (PFNGLCREATESHADERPROC)load("glCreateShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)load("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC)load("glCompileShader");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)load("glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)load("glGetShaderInfoLog");
    glDeleteShader = (PFNGLDELETESHADERPROC)load("glDeleteShader");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)load("glCreateProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC)load("glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)load("glLinkProgram");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)load("glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)load("glGetProgramInfoLog");
    glUseProgram = (PFNGLUSEPROGRAMPROC)load("glUseProgram");
    glDeleteProgram = (PFNGLDELETEPROGRAMPROC)load("glDeleteProgram");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)load("glGetUniformLocation");

    glUniform1i = (PFNGLUNIFORM1IPROC)load("glUniform1i");
    glUniform1f = (PFNGLUNIFORM1FPROC)load("glUniform1f");
    glUniform2fv = (PFNGLUNIFORM2FVPROC)load("glUniform2fv");
    glUniform3fv = (PFNGLUNIFORM3FVPROC)load("glUniform3fv");
    glUniform4fv = (PFNGLUNIFORM4FVPROC)load("glUniform4fv");
    glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)load("glUniformMatrix3fv");
    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)load("glUniformMatrix4fv");

    glCreateTextures = (PFNGLCREATETEXTURESPROC)load("glCreateTextures");
    glTextureStorage2D = (PFNGLTEXTURESTORAGE2DPROC)load("glTextureStorage2D");
    glTextureSubImage2D = (PFNGLTEXTURESUBIMAGE2DPROC)load("glTextureSubImage2D");
    glGenerateTextureMipmap = (PFNGLGENERATETEXTUREMIPMAPPROC)load("glGenerateTextureMipmap");
    glTextureParameteri = (PFNGLTEXTUREPARAMETERIPROC)load("glTextureParameteri");
    glBindTextureUnit = (PFNGLBINDTEXTUREUNITPROC)load("glBindTextureUnit");

    glCreateFramebuffers = (PFNGLCREATEFRAMEBUFFERSPROC)load("glCreateFramebuffers");
    glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)load("glBindFramebuffer");
    glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)load("glDeleteFramebuffers");
    glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)load("glFramebufferTexture2D");
    glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)load("glCheckFramebufferStatus");
}

int gladLoadGLLoader(GLADloadproc load) {
    load_gl_functions(load);
    return 1;
}
