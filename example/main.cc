#define USE_OPENGL2
#include "OpenGLWindow/OpenGLInclude.h"
#ifdef _WIN32
#include "OpenGLWindow/Win32OpenGLWindow.h"
#elif defined __APPLE__
#include "OpenGLWindow/MacOpenGLWindow.h"
#else
// assume linux
#include "OpenGLWindow/X11OpenGLWindow.h"
#endif

#include "nanovg.h"
#define NANOVG_GL2_IMPLEMENTATION
#include "nanovg_gl.h"

#include "perf.h"

#include <cassert>
#include <string>
#include <fstream>
#include <vector>

NVGcontext* vg;
b3gDefaultOpenGLWindow *window = 0;

#ifdef _WIN32
#ifdef __cplusplus
extern "C" {
#endif
#	include <windows.h>
#	include <mmsystem.h>
#ifdef __cplusplus
}
#endif
#pragma comment( lib, "winmm.lib" )
#else
#if defined(__unix__) || defined(__APPLE__)
#		include <sys/time.h>
#	else
#		include <ctime>
#	endif
#endif

bool mouse_btn = false;
int mouse_btn_button = 0;
int mouse_btn_state = 0;
float mouse_btn_x = 0.0;
float mouse_btn_y = 0.0;

bool keyboard_btn = false;
int keyboard_code = 0;
int keyboard_state = 0;

class timer{
	public:
#ifdef _WIN32
		typedef DWORD time_t;
		
		timer() {::timeBeginPeriod( 1 );}
		~timer(){::timeEndPeriod(1);}
			
		void start(){t_[0] = ::timeGetTime();}
		void end()  {t_[1] = ::timeGetTime();}
				
		time_t sec() {return (time_t)( (t_[1]-t_[0]) / 1000 );}
		time_t msec(){return (time_t)( (t_[1]-t_[0]) );}
		time_t usec(){return (time_t)( (t_[1]-t_[0]) * 1000 );}
		
#else 
#if defined(__unix__) || defined(__APPLE__)
		typedef unsigned long int time_t;

		
		void start(){gettimeofday(tv+0, &tz);}
		void end()  {gettimeofday(tv+1, &tz);}
		
		time_t sec() {return (time_t)(tv[1].tv_sec-tv[0].tv_sec);}
		time_t msec(){return this->sec()*1000 + (time_t)((tv[1].tv_usec-tv[0].tv_usec)/1000);}
		time_t usec(){return this->sec()*1000000 + (time_t)(tv[1].tv_usec-tv[0].tv_usec);}

#else //C timer
		//using namespace std;
		typedef clock_t time_t;
		
		void start(){t_[0] = clock();}
		void end()  {t_[1] = clock();}
		
		time_t sec() {return (time_t)( (t_[1]-t_[0]) / CLOCKS_PER_SEC );}
		time_t msec(){return (time_t)( (t_[1]-t_[0]) * 1000 / CLOCKS_PER_SEC );}
		time_t usec(){return (time_t)( (t_[1]-t_[0]) * 1000000 / CLOCKS_PER_SEC );}
	
#endif
#endif
		
	private:
		
#ifdef _WIN32
		DWORD t_[2];
#else
#if defined(__unix__) || defined(__APPLE__)
		struct timeval tv[2];
		struct timezone tz;
#else
		time_t t_[2];
#endif
#endif
		
};

unsigned char fclamp(float x)
{
  int i = (int)x;
  
  if (i < 0) i = 0;
  if (i > 255) i = 255;

  return (unsigned char)(i);
}

void checkErrors(const char *desc) {
  GLenum e = glGetError();
  if (e != GL_NO_ERROR) {
    fprintf(stderr, "OpenGL error in \"%s\": %d (%d)\n", desc, e, e);
    exit(20);
  }
}

void keyboardCallback(int keycode, int state) {
  keyboard_btn = true;
  keyboard_code = keycode;
  keyboard_state = state;
}

void mouseButtonCallback(int button, int state, float x, float y) {
  mouse_btn = true;
  mouse_btn_button = button;
  mouse_btn_state = state;
  mouse_btn_x = x;
  mouse_btn_y = y;
}

void mouseMoveCallback(float x, float y) {
  //printf("mouse move, x: %.2f, y: %.2f\n", x, y);
}

int main(int argc, char** argv) {
  int width = 1024;
  int height = 600;

  PerfGraph fps;
  initGraph(&fps, GRAPH_RENDER_FPS, "Frame Time");

  window = new b3gDefaultOpenGLWindow;
  b3gWindowConstructionInfo ci;
#ifdef USE_OPENGL2
  ci.m_openglVersion = 2;
#endif
  ci.m_width = width;
  ci.m_height = height;
  window->createWindow(ci);

  window->setWindowTitle("what2watch");

#ifndef __APPLE__
#ifndef _WIN32
  // some Linux implementations need the 'glewExperimental' to be true
  glewExperimental = GL_TRUE;
#endif
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    exit(-1);
  }

  if (!GLEW_VERSION_2_1) {
    fprintf(stderr, "OpenGL 2.1 is not available\n");
    exit(-1);
  }
#endif

  window->setMouseButtonCallback(mouseButtonCallback);
  window->setMouseMoveCallback(mouseMoveCallback);
  checkErrors("mouse");
  window->setKeyboardCallback(keyboardCallback);
  checkErrors("keyboard");

  vg = nvgCreateGL2(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);

  nvgCreateFont(vg, "sans", "../example/Roboto-Regular.ttf");
  nvgCreateFont(vg, "sans-bold", "../example/Roboto-Bold.ttf");
  nvgCreateFont(vg, "icons", "../example/entypo.ttf");

  timer tm;

  tm.start();
  double prevt = 0.0;
  tm.end();
  prevt = tm.msec() / 1000.0; // [s]

  while (!window->requestedExit()) {
    window->startRendering();

    double t, dt;

    tm.end();
    t = tm.msec() / 1000.0; // [s]
    
    dt = t - prevt;
    prevt = t;
    updateGraph(&fps, dt);

    // Update and render
    glViewport(0, 0, width, height);
    glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

    nvgBeginFrame(vg, width, height, width / (float)height);

    if (mouse_btn) {
      // emit mouse-button event here
      mouse_btn = false;
    }

    if (keyboard_btn) {
      // emit keyboard event here
      keyboard_btn = false;
    }

    renderGraph(vg, 5,5, &fps);

    nvgEndFrame(vg);

    window->endRendering();
  }

  nvgDeleteGL2(vg);

  delete window;

  return 0;
}

