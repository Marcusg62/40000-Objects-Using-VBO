///////////////////////////////////////////////////////////////////////////////
// main.cpp
// ========
// testing OpenGL buffer object, GL_ARB_vertex_buffer_object extension
// draw a unit cube using glDrawElements()
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2006-11-14
// UPDATED: 2018-08-09
///////////////////////////////////////////////////////////////////////////////

// in order to get function prototypes from glext.h, define GL_GLEXT_PROTOTYPES before including glext.h
#define GL_GLEXT_PROTOTYPES

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include "glExtension.h"                // helper for OpenGL extensions


// GLUT CALLBACK functions
void displayCB();
void reshapeCB(int w, int h);
void timerCB(int millisec);
void idleCB();
void keyboardCB(unsigned char key, int x, int y);
void mouseCB(int button, int stat, int x, int y);
void mouseMotionCB(int x, int y);

// CALLBACK function when exit() called ///////////////////////////////////////
void exitCB();


void initGL();
int  initGLUT(int argc, char **argv);
bool initSharedMem();
void clearSharedMem();
void initLights();
void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ);
GLuint createVBO(const void* data, int dataSize, GLenum target=GL_ARRAY_BUFFER, GLenum usage=GL_STATIC_DRAW);
void deleteVBO(GLuint vboId);
void drawString(const char *str, int x, int y, float color[4], void *font);
void drawString3D(const char *str, float pos[3], float color[4], void *font);
void showInfo();
void toOrtho();
void toPerspective();


// constants
const int   SCREEN_WIDTH    = 800;
const int   SCREEN_HEIGHT   = 600;
const float CAMERA_DISTANCE = 15.0f;
const int   TEXT_WIDTH      = 8;
const int   TEXT_HEIGHT     = 13;


// global variables
void *font = GLUT_BITMAP_8_BY_13;
GLuint vboId = 0;                   // ID of VBO for vertex arrays
GLuint iboId = 0;                   // ID of VBO for index array
int screenWidth;
int screenHeight;
bool mouseLeftDown;
bool mouseRightDown;
float mouseX, mouseY;
float cameraAngleX;
float cameraAngleY;
float cameraDistance;
bool vboSupported, vboUsed;
int drawMode = 0;

float g_eyeSpeed = 0.01;
float g_eyeHeight = 2;
float g_eyeRadius = 10;
bool updatePositions = true;

float myClock = 0.0;
float scale = 0.2;
float myTime = 0;
float fps = 0;
float frames = 0;
float base_time = 0;
std::vector<float> eyePosition = { 0, g_eyeHeight, g_eyeRadius };



// unit cube //////////////////////////////////////////////////////////////////
//    v6----- v5
//   /|      /|
//  v1------v0|
//  | |     | |
//  | v7----|-v4
//  |/      |/
//  v2------v3

// vertex coords array for glDrawElements() ===================================
// A cube has 6 sides and each side has 4 vertices, therefore, the total number
// of vertices is 24 (6 sides * 4 verts), and 72 floats in the vertex array
// since each vertex has 3 components (x,y,z) (= 24 * 3)
//GLfloat vertices[]  = {
//     .03f, .03f, .03f,  -.03f, .03f, .03f,  -.03f,-.03f, .03f,  .03f,-.03f, .03f,   // v0,v1,v2,v3 (front)
//     .03f, .03f, .03f,   .03f,-.03f, .03f,   .03f,-.03f,-.03f,  .03f, .03f,-.03f,   // v0,v3,v4,v5 (right)
//     .03f, .03f, .03f,   .03f, .03f,-.03f,  -.03f, .03f,-.03f, -.03f, .03f, .03f,   // v0,v5,v6,v1 (top)
//    -.03f, .03f, .03f,  -.03f, .03f,-.03f,  -.03f,-.03f,-.03f, -.03f,-.03f, .03f,   // v1,v6,v7,v2 (left)
//    -.03f,-.03f,-.03f,   .03f,-.03f,-.03f,   .03f,-.03f, .03f, -.03f,-.03f, .03f,   // v7,v4,v3,v2 (bottom)
//     .03f,-.03f,-.03f,  -.03f,-.03f,-.03f,  -.03f, .03f,-.03f,  .03f, .03f,-.03f,    // v4,v7,v6,v5 (back)
//	 //v0 = 0.12f,.03f, .03f,
//	 //v1 = -.12f, .03f, .03f,
//	 //v2 = -.12f,-.03f, .03f,
//	 //v3 = .12f,-.03f, .03f,
//	 //v4 =  .12f,-.03f,-.03f,
//	 //v5 = .12f, .03f,-.03f,
//	 //v6 = -.12f, .03f,-.03f,
//	 //v7 = -.12f,-.03f,-.03f,
//
//	 0.12f,.03f, .03f,  -.12f, .03f, .03f,  -.12f,-.03f, .03f,  .12f,-.03f, .03f,  // v0,v1,v2,v3 (front)
//	 0.12f,.03f, .03f,  .12f,-.03f, .03f,  .12f,-.03f,-.03f,     .12f, .03f,-.03f,   // v0,v3,v4,v5 (right)
//	 0.12f,.03f, .03f,  .12f, .03f,-.03f, -.12f, .03f,-.03f,  -.03f, .03f, .03f,   // v0,v5,v6,v1 (top)
//	 -.12f, .03f, .03f, -.12f, .03f,-.03f, -.12f,-.03f,-.03f, -.12f,-.03f, .03f, // v1,v6,v7,v2 (left)
//	 -.12f,-.03f,-.03f,  .12f,-.03f,-.03f,  .12f,-.03f, .03f, -.12f,-.03f, .03f,   // v7,v4,v3,v2 (bottom)
//	 0.12f,-.03f,-.03f,  -.12f,-.03f,-.03f, -.12f, .03f,-.03f,  .12f, .03f,-.03f,    // v4,v7,v6,v5 (back)
//}; 


//float v0 = 1.5, 1.5, .3;
//float v1 = -1.5, 1.5, .3;
//float v2 = -1.5, -1.5, .3;
//float v3 = 1.5, -1.5, .3;

//float v4 = 1.5, 1.5, .3;
//float v5 = 1.5, 1.5, .3;
//float v6 = 1.5, 1.5, .3;
//float v7 = 1.5, 1.5, .3;

GLfloat vertices[] = {
	//.5f, .5f, .5f,  -.5f, .5f, .5f,  -.5f,-.5f, .5f,  .5f,-.5f, .5f,   // v0,v1,v2,v3 (front)
	// .5f, .5f, .5f,   .5f,-.5f, .5f,   .5f,-.5f,-.5f,  .5f, .5f,-.5f,   // v0,v3,v4,v5 (right)
	// .5f, .5f, .5f,   .5f, .5f,-.5f,  -.5f, .5f,-.5f, -.5f, .5f, .5f,   // v0,v5,v6,v1 (top)
	//-.5f, .5f, .5f,  -.5f, .5f,-.5f,  -.5f,-.5f,-.5f, -.5f,-.5f, .5f,   // v1,v6,v7,v2 (left)
	//-.5f,-.5f,-.5f,   .5f,-.5f,-.5f,   .5f,-.5f, .5f, -.5f,-.5f, .5f,   // v7,v4,v3,v2 (bottom)
	// .5f,-.5f,-.5f,  -.5f,-.5f,-.5f,  -.5f, .5f,-.5f,  .5f, .5f,-.5f,    // v4,v7,v6,v5 (back)

	 	1.5f, .5f, .5f,  .5f, .5f, .5f,  .5f,-.5f, .5f,  1.5f,-.5f, .5f,   // v0,v1,v2,v3 (front)
	 1.5f, .5f, .5f,   1.5f,-.5f, .5f,   1.5f,-.5f,-.5f,  1.5f, .5f,-.5f,   // v0,v3,v4,v5 (right)
	 1.5f, .5f, .5f,   1.5f, .5f,-.5f,  .5f, .5f,-.5f, .5f, .5f, .5f,   // v0,v5,v6,v1 (top)
	.5f, .5f, .5f,  .5f, .5f,-.5f,  .5f,-.5f,-.5f, .5f,-.5f, .5f,   // v1,v6,v7,v2 (left)
	.5f,-.5f,-.5f,   1.5f,-.5f,-.5f,   1.5f,-.5f, .5f, .5f,-.5f, .5f,   // v7,v4,v3,v2 (bottom)
	 1.5f,-.5f,-.5f,  .5f,-.5f,-.5f,  .5f, .5f,-.5f,  1.5f, .5f,-.5f,    // v4,v7,v6,v5 (back)z
};

// normal array
GLfloat normals[] = {
     0, 0, 1,   0, 0, 1,   0, 0, 1,   0, 0, 1,  // v0,v1,v2,v3 (front)
     1, 0, 0,   1, 0, 0,   1, 0, 0,   1, 0, 0,  // v0,v3,v4,v5 (right)
     0, 1, 0,   0, 1, 0,   0, 1, 0,   0, 1, 0,  // v0,v5,v6,v1 (top)
    -1, 0, 0,  -1, 0, 0,  -1, 0, 0,  -1, 0, 0,  // v1,v6,v7,v2 (left)
     0,-1, 0,   0,-1, 0,   0,-1, 0,   0,-1, 0,  // v7,v4,v3,v2 (bottom)
     0, 0,-1,   0, 0,-1,   0, 0,-1,   0, 0,-1   // v4,v7,v6,v5 (back)
	 
};

// color array
GLfloat colors[] = {
     1, 1, 1,   1, 0, 0,   1, 0, 0,   1, 0, 1,  // v0,v1,v2,v3 (front)
     1, 1, 1,   1, 0, 1,   0, 0, 1,   0, 1, 1,  // v0,v3,v4,v5 (right)
     1, 1, 1,   0, 1, 1,   0, 1, 0,   1, 1, 0,  // v0,v5,v6,v1 (top)
     1, 1, 0,   0, 1, 0,   0, 0, 0,   1, 0, 0,  // v1,v6,v7,v2 (left)
     0, 0, 0,   0, 0, 1,   1, 0, 1,   1, 0, 0,  // v7,v4,v3,v2 (bottom)
     0, 0, 1,   0, 0, 0,   0, 1, 0,   0, 1, 1   // v4,v7,v6,v5 (back)

};

// index array for glDrawElements() ===========================================
// A cube has 36 indices = 6 sides * 2 tris * 3 verts
GLuint indices[] = {
     0, 1, 2,   2, 3, 0,    // v0-v1-v2, v2-v3-v0 (front)
     4, 5, 6,   6, 7, 4,    // v0-v3-v4, v4-v5-v0 (right)
     8, 9,10,  10,11, 8,    // v0-v5-v6, v6-v1-v0 (top)
    12,13,14,  14,15,12,    // v1-v6-v7, v7-v2-v1 (left)
    16,17,18,  18,19,16,    // v7-v4-v3, v3-v2-v7 (bottom)
    20,21,22,  22,23,20,     // v4-v7-v6, v6-v5-v4 (back)

	0, 1, 2,   2, 3, 0,    // v0-v1-v2, v2-v3-v0 (front)
	 4, 5, 6,   6, 7, 4,    // v0-v3-v4, v4-v5-v0 (right)
	 8, 9,10,  10,11, 8,    // v0-v5-v6, v6-v1-v0 (top)
	12,13,14,  14,15,12,    // v1-v6-v7, v7-v2-v1 (left)
	16,17,18,  18,19,16,    // v7-v4-v3, v3-v2-v7 (bottom)
	20,21,22,  22,23,20     // v4-v7-v6, v6-v5-v4 (back)

};



///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    initSharedMem();

    // init GLUT and GL
    initGLUT(argc, argv);
    initGL();

    // register exit callback
    atexit(exitCB);

    // get OpenGL info
    glExtension& ext = glExtension::getInstance();
    vboSupported = vboUsed = ext.isSupported("GL_ARB_vertex_buffer_object");
    if(vboSupported)
    {
        // create vertex buffer objects, you need to delete them when program exits
        // Try to put both vertex coords array, vertex normal array and vertex color in the same buffer object.
        // glBufferData with NULL pointer reserves only memory space.
        // Copy actual data with multiple calls of glBufferSubData for vertex positions, normals, colours, etc.
        // target flag is GL_ARRAY_BUFFER, and usage flag is GL_STATIC_DRAW
        glGenBuffers(1, &vboId);
        glBindBuffer(GL_ARRAY_BUFFER, vboId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(normals)+sizeof(colors), 0, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);                             // copy vertices starting from 0 offest
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(normals), normals);                // copy normals after vertices
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(normals), sizeof(colors), colors);  // copy colours after normals
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &iboId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        std::cout << "Video card supports GL_ARB_vertex_buffer_object." << std::endl;
    }
    else
    {
        std::cout << "[WARNING] Video card does NOT support GL_ARB_vertex_buffer_object." << std::endl;
    }

    // the last GLUT call (LOOP)
    // window will be shown and display callback is triggered by events
    // NOTE: this call never return main().
    glutMainLoop(); /* Start GLUT event-processing loop */

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// initialize GLUT for windowing
///////////////////////////////////////////////////////////////////////////////
int initGLUT(int argc, char **argv)
{
    // GLUT stuff for windowing
    // initialization openGL window.
    // it is called before any other GLUT routine
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);   // display mode

    glutInitWindowSize(screenWidth, screenHeight);  // window size

    glutInitWindowPosition(100, 100);               // window location

    // finally, create a window with openGL context
    // Window will not displayed until glutMainLoop() is called
    // it returns a unique ID
    int handle = glutCreateWindow(argv[0]);         // param is the title of window

    // register GLUT callback functions
    glutDisplayFunc(displayCB);
    //glutTimerFunc(33, timerCB, 33);                 // redraw only every given millisec
    glutIdleFunc(idleCB);                           // redraw when idle
    glutReshapeFunc(reshapeCB);
    glutKeyboardFunc(keyboardCB);
    glutMouseFunc(mouseCB);
    glutMotionFunc(mouseMotionCB);






    return handle;
}



///////////////////////////////////////////////////////////////////////////////
// initialize OpenGL
// disable unused features
///////////////////////////////////////////////////////////////////////////////
void initGL()
{
    glShadeModel(GL_SMOOTH);                    // shading mathod: GL_SMOOTH or GL_FLAT
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      // 4-byte pixel alignment

    // enable /disable features
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    //glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);

     // track material ambient and diffuse from surface color, call it before glEnable(GL_COLOR_MATERIAL)
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    glClearColor(0, 0, 0, 0);                   // background color
    glClearStencil(0);                          // clear stencil buffer
    glClearDepth(1.0f);                         // 0 is near, 1 is far
    glDepthFunc(GL_LEQUAL);

    initLights();

}



///////////////////////////////////////////////////////////////////////////////
// write 2d text using GLUT
// The projection matrix must be set to orthogonal before call this function.
///////////////////////////////////////////////////////////////////////////////
void drawString(const char *str, int x, int y, float color[4], void *font)
{
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
    glDisable(GL_LIGHTING);     // need to disable lighting for proper text color
    glDisable(GL_TEXTURE_2D);

    glColor4fv(color);          // set text color
    glRasterPos2i(x, y);        // place text position

    // loop all characters in the string
    while(*str)
    {
        glutBitmapCharacter(font, *str);
        ++str;
    }

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glPopAttrib();
}



///////////////////////////////////////////////////////////////////////////////
// draw a string in 3D space
///////////////////////////////////////////////////////////////////////////////
void drawString3D(const char *str, float pos[3], float color[4], void *font)
{
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
    glDisable(GL_LIGHTING);     // need to disable lighting for proper text color
    glDisable(GL_TEXTURE_2D);

    glColor4fv(color);          // set text color
    glRasterPos3fv(pos);        // place text position

    // loop all characters in the string
    while(*str)
    {
        glutBitmapCharacter(font, *str);
        ++str;
    }

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glPopAttrib();
}



///////////////////////////////////////////////////////////////////////////////
// initialize global variables
///////////////////////////////////////////////////////////////////////////////
bool initSharedMem()
{
    screenWidth = SCREEN_WIDTH;
    screenHeight = SCREEN_HEIGHT;

    mouseLeftDown = mouseRightDown = false;
    mouseX = mouseY = 0;

    cameraAngleX = cameraAngleY = 0.0f;
    cameraDistance = CAMERA_DISTANCE;

    drawMode = 0; // 0:fill, 1: wireframe, 2:points

    return true;
}



///////////////////////////////////////////////////////////////////////////////
// clean up global vars
///////////////////////////////////////////////////////////////////////////////
void clearSharedMem()
{
    // clean up VBOs
    if(vboSupported)
    {
        deleteVBO(vboId);
        deleteVBO(iboId);
        vboId = iboId = 0;
    }
}



///////////////////////////////////////////////////////////////////////////////
// initialize lights
///////////////////////////////////////////////////////////////////////////////
void initLights()
{
    // set up light colors (ambient, diffuse, specular)
    GLfloat lightKa[] = {.2f, .2f, .2f, 1.0f};  // ambient light
    GLfloat lightKd[] = {.7f, .7f, .7f, 1.0f};  // diffuse light
    GLfloat lightKs[] = {1, 1, 1, 1};           // specular light
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightKa);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightKd);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightKs);

    // position the light
    float lightPos[4] = {0, 0, 1, 0}; // directional light
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glEnable(GL_LIGHT0);                        // MUST enable each light source after configuration
}



///////////////////////////////////////////////////////////////////////////////
// set camera position and lookat direction
///////////////////////////////////////////////////////////////////////////////
void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(posX, posY, posZ, targetX, targetY, targetZ, 0, 1, 0); // eye(x,y,z), focal(x,y,z), up(x,y,z)
}



///////////////////////////////////////////////////////////////////////////////
// generate vertex buffer object and bind it with its data
// You must give 2 hints about data usage; target and mode, so that OpenGL can
// decide which data should be stored and its location.
// VBO works with 2 different targets; GL_ARRAY_BUFFER for vertex arrays
// and GL_ELEMENT_ARRAY_BUFFER for index array in glDrawElements().
// The default target is GL_ARRAY_BUFFER.
// By default, usage mode is set as GL_STATIC_DRAW.
// Other usages are GL_STREAM_DRAW, GL_STREAM_READ, GL_STREAM_COPY,
// GL_STATIC_DRAW, GL_STATIC_READ, GL_STATIC_COPY,
// GL_DYNAMIC_DRAW, GL_DYNAMIC_READ, GL_DYNAMIC_COPY.
///////////////////////////////////////////////////////////////////////////////
GLuint createVBO(const void* data, int dataSize, GLenum target, GLenum usage)
{
    GLuint id = 0;  // 0 is reserved, glGenBuffersARB() will return non-zero id if success

    glGenBuffers(1, &id);                           // create a vbo
    glBindBuffer(target, id);                       // activate vbo id to use
    glBufferData(target, dataSize, data, usage);    // upload data to video card

    // check data size in VBO is same as input array, if not return 0 and delete VBO
    int bufferSize = 0;
    glGetBufferParameteriv(target, GL_BUFFER_SIZE, &bufferSize);
    if(dataSize != bufferSize)
    {
        glDeleteBuffers(1, &id);
        id = 0;
        std::cout << "[createVBO()] Data size is mismatch with input array\n";
    }

    return id;      // return VBO id
}



///////////////////////////////////////////////////////////////////////////////
// destroy a VBO
// If VBO id is not valid or zero, then OpenGL ignores it silently.
///////////////////////////////////////////////////////////////////////////////
void deleteVBO(GLuint vboId)
{
    glDeleteBuffers(1, &vboId);
}



///////////////////////////////////////////////////////////////////////////////
// display info messages
///////////////////////////////////////////////////////////////////////////////
void showInfo()
{
    // backup current model-view matrix
    glPushMatrix();                     // save current modelview matrix
    glLoadIdentity();                   // reset modelview matrix

    // set to 2D orthogonal projection
    glMatrixMode(GL_PROJECTION);     // switch to projection matrix
    glPushMatrix();                  // save current projection matrix
    glLoadIdentity();                // reset projection matrix
    gluOrtho2D(0, screenWidth, 0, screenHeight);  // set to orthogonal projection

    float color[4] = {1, 1, 1, 1};

    std::stringstream ss;
    ss << "FPS: " << fps << std::ends;  // add 0(ends) at the end
    drawString(ss.str().c_str(), 1, screenHeight-TEXT_HEIGHT, color, font);
    ss.str(""); // clear buffer


    // restore projection matrix
    glPopMatrix();                   // restore to previous projection matrix

    // restore modelview matrix
    glMatrixMode(GL_MODELVIEW);      // switch to modelview matrix
    glPopMatrix();                   // restore to previous modelview matrix
}



///////////////////////////////////////////////////////////////////////////////
// set projection matrix as orthogonal
///////////////////////////////////////////////////////////////////////////////
void toOrtho()
{
    // set viewport to be the entire window
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);

    // set orthographic viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenWidth, 0, screenHeight, -1, 1);

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}



///////////////////////////////////////////////////////////////////////////////
// set the projection matrix as perspective
///////////////////////////////////////////////////////////////////////////////
void toPerspective()
{
    // set viewport to be the entire window
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);

    // set perspective viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)(screenWidth)/screenHeight, 0.1f, 100.0f); // FOV, AspectRatio, NearClip, FarClip

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}






//=============================================================================
// CALLBACKS
//=============================================================================

void displayCB()
{
    // clear buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // save the initial ModelView matrix before modifying ModelView matrix
    glPushMatrix();
	frames++;

	eyePosition[0] = sin(frames * g_eyeSpeed) * g_eyeRadius; // x 
	eyePosition[2] = cos(frames * g_eyeSpeed) * g_eyeRadius; // z 

    // tramsform camera
    //glRotatef(cameraAngleX, 1, 0, 0);   // pitch
    //glRotatef(eyePosition[0], 0, 1, 0);   // heading
	//gluLookAt(eyePosition[0], eyePosition[1], eyePosition[2], 0, 0, 0, 0, 1, 0);
	setCamera(eyePosition[0], g_eyeHeight, eyePosition[2], 0, 0, 0);


		// bind VBOs with IDs and set the buffer offsets of the bound VBOs
// When buffer object is bound with its ID, all pointers in gl*Pointer()
// are treated as offset instead of real pointer.
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);

	// enable vertex arrays
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	// before draw, specify vertex and index arrays with their offsets
	glNormalPointer(GL_FLOAT, 0, (void*)sizeof(vertices));
	glColorPointer(3, GL_FLOAT, 0, (void*)(sizeof(vertices) + sizeof(normals)));
	glVertexPointer(3, GL_FLOAT, 0, 0);

	glDrawElements(GL_TRIANGLES,            // primitive type
						72,                      // # of indices
						GL_UNSIGNED_INT,         // data type
						(void*)0);               // ptr to indices

        glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);

        // it is good idea to release VBOs with ID 0 after use.
        // Once bound with 0, all pointers in gl*Pointer() behave as real
        // pointer, so, normal vertex array operations are re-activated
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    

    // draw a cube using vertex array method
    // notice that only difference between VBO and VA is binding buffers and offsets
	// update fps
		myTime = glutGet(GLUT_ELAPSED_TIME);
		if ((myTime - base_time) > 1000.0)
		{
			fps = frames * 1000.0 / (myTime - base_time);
			base_time = myTime;
			frames = 0;
		}
    // draw info messages
    showInfo();

    glPopMatrix();

    glutSwapBuffers();

}


void reshapeCB(int w, int h)
{
    screenWidth = w;
    screenHeight = h;
    toPerspective();
}


void timerCB(int millisec)
{
    glutTimerFunc(millisec, timerCB, millisec);
    glutPostRedisplay();
}


void idleCB()
{
    glutPostRedisplay();
}


void keyboardCB(unsigned char key, int x, int y)
{
    switch(key)
    {
    case 27: // ESCAPE
        exit(0);
        break;

    case ' ':
        if(vboSupported)
            vboUsed = !vboUsed;
        break;

    case 'd': // switch rendering modes (fill -> wire -> point)
    case 'D':
        ++drawMode;
        drawMode %= 3;
        if(drawMode == 0)        // fill mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
        }
        else if(drawMode == 1)  // wireframe mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }
        else                    // point mode
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }
        break;

    default:
        ;
    }
}


void mouseCB(int button, int state, int x, int y)
{
    //mouseX = x;
    //mouseY = y;

    //if(button == GLUT_LEFT_BUTTON)
    //{
    //    if(state == GLUT_DOWN)
    //    {
    //        mouseLeftDown = true;
    //    }
    //    else if(state == GLUT_UP)
    //        mouseLeftDown = false;
    //}

    //else if(button == GLUT_RIGHT_BUTTON)
    //{
    //    if(state == GLUT_DOWN)
    //    {
    //        mouseRightDown = true;
    //    }
    //    else if(state == GLUT_UP)
    //        mouseRightDown = false;
    //}
}


void mouseMotionCB(int x, int y)
{
    //if(mouseLeftDown)
    //{
    //    cameraAngleY += (x - mouseX);
    //    cameraAngleX += (y - mouseY);
    //    mouseX = x;
    //    mouseY = y;
    //}
    //if(mouseRightDown)
    //{
    //    cameraDistance -= (y - mouseY) * 0.2f;
    //    mouseY = y;
    //}
}



void exitCB()
{
    clearSharedMem();
}
