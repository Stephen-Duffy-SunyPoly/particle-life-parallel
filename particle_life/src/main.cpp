
#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
	try {
		ofGLWindowSettings s;
		//s.setGLVersion(4, 6);	// OpenGL 4.6 ( GTX 600 and up )
		s.setSize(1600, 900);
		ofCreateWindow(s);
		ofRunApp(new ofApp());
		shutdownThreads();
	} catch (std::runtime_error &e) {
		std::cerr << "Program experienced a fatal error: " << e.what() << std::endl;
	}
}
