#pragma once

//--------------------------
// utils
#include "ofConstants.h"
#include "ofFileUtils.h"
#include "ofLog.h"
#include "ofSystemUtils.h"

#include "ofURLFileLoader.h"

#include "ofUtils.h"

#include "ofThread.h"
#include "ofThreadChannel.h"

#include "ofFpsCounter.h"
#include "ofJson.h"
#include "ofXml.h"

//--------------------------
// types
#include "ofColor.h"
#include "ofGraphicsBaseTypes.h"
#include "ofParameter.h"
#include "ofPoint.h"
#include "ofRectangle.h"
#include "ofTypes.h"

//--------------------------
// math
#include "ofMath.h"
#include "ofVectorMath.h"

//--------------------------
// communication
#if !defined(TARGET_OF_IOS) & !defined(TARGET_ANDROID) & !defined(TARGET_EMSCRIPTEN)
    #include "../communication/ofSerial.h"
    #include "../communication/ofArduino.h"
#endif

//--------------------------
// gl
#include "../gl/ofCubeMap.h"
#include "../gl/ofFbo.h"
#include "../gl/ofGLRenderer.h"
#include "../gl/ofGLUtils.h"
#include "../gl/ofLight.h"
#include "../gl/ofMaterial.h"
#include "../gl/ofShader.h"
#include "../gl/ofTexture.h"
#include "../gl/ofVbo.h"
#include "../gl/ofVboMesh.h"
// #include "ofGLProgrammableRenderer.h"
// #ifndef TARGET_PROGRAMMABLE_GL
//     #include "ofGLRenderer.h"
// #endif

//--------------------------
// graphics
#if !defined(TARGET_OF_IOS) & !defined(TARGET_ANDROID) & !defined(TARGET_EMSCRIPTEN)
    // #include "ofCairoRenderer.h"
    // #include "ofGraphicsCairo.h"
#endif
#include "ofGraphics.h"
#include "ofImage.h"
#include "ofPath.h"
#include "ofPixels.h"
#include "ofPolyline.h"
#include "ofRendererCollection.h"
#include "ofTessellator.h"
#include "ofTrueTypeFont.h"

//--------------------------
// app
#include "ofAppBaseWindow.h"
#include "ofAppRunner.h"
#include "ofBaseApp.h"
#include "ofMainLoop.h"
#include "ofWindowSettings.h"
#if !defined(TARGET_OF_IOS) & !defined(TARGET_ANDROID) & !defined(TARGET_EMSCRIPTEN) & !defined(TARGET_RASPBERRY_PI_LEGACY)
    #include "ofAppGLFWWindow.h"
//    #if !defined(TARGET_LINUX_ARM)
//        #include "ofAppGlutWindow.h"
//    #endif
#endif

//--------------------------
// audio
#ifndef TARGET_NO_SOUND
    #include "../sound/ofSoundStream.h"
    #include "../sound/ofSoundPlayer.h"
    #include "../sound/ofSoundBuffer.h"
#endif

//--------------------------
// video
// #include "../video/ofVideoGrabber.h"
// #include "../video/ofVideoPlayer.h"

//--------------------------
// events
#include "../events/ofEvents.h"

//--------------------------
// 3d
#include "../3d/of3dUtils.h"
#include "../3d/ofCamera.h"
#include "../3d/ofEasyCam.h"
#include "../3d/ofMesh.h"
#include "../3d/ofNode.h"

//--------------------------
#ifdef OF_LEGACY_INCLUDE_STD
using namespace std;
#else

// this will eventually be disabled by default
#define OF_USE_MINIMAL_STD
    #ifdef OF_USE_MINIMAL_STD
using std::cout;
using std::deque;
using std::endl;
using std::make_shared;
using std::map;
using std::max;
using std::pair;
using std::shared_ptr;
using std::string;
using std::stringstream;
using std::swap;
using std::to_string;
using std::vector;
using std::weak_ptr;
    #endif
#endif


// core: ---------------------------
#include <stdint.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>
#include <iostream>
#include <vector>
#include <memory>
#include <functional>
