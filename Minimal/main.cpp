/************************************************************************************

Authors     :   Bradley Austin Davis <bdavis@saintandreas.org>
Copyright   :   Copyright Brad Davis. All Rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

************************************************************************************/

#include <iostream>
#include <memory>
#include <exception>
#include <algorithm>

#include <Windows.h>

#define __STDC_FORMAT_MACROS 1

#define FAIL(X) throw std::runtime_error(X)

#define SPHERE_FRAG "shader.frag"
#define SPHERE_VERT "shader.vert"
#define BOUNDING_FRAG "bounding.frag"
#define BOUNDING_VERT "bounding.vert"
#define BULLET_FRAG "bullet.frag"
#define BULLET_VERT "bullet.vert"
#define MODEL_FRAG "model.frag"
#define MODEL_VERT "model.vert"

//sound path
#define SOUND_PATH "sound/gun_shot.mp3"
#define SHELL_PATH "sound/shell_falls.mp3"
#define BGM "sound/BGM.mp3"
#define FIRING_BGM "sound/firing_music.mp3"

///////////////////////////////////////////////////////////////////////////////
//
// GLM is a C++ math library meant to mirror the syntax of GLSL 
//

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <boost/circular_buffer.hpp>
#include "Skybox.h"
#include "Model.h"
#include "Mesh.h"
#include "BoundingBox.h"
#include "irrKlang.h"

// Import the most commonly used types into the default namespace
using glm::ivec3;
using glm::ivec2;
using glm::uvec2;
using glm::mat3;
using glm::mat4;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::quat;


double iod = 0.0;
double original_iod = 0.0;
int frameLag=0;
int renderLag = 0;
int last;
bool isLeft = false;
bool fire = false;
bool finishFire;
bool fired;
bool soundPlayed;
int frameCtr=0;
int frameHead = 0;
bool render = true;
bool superRot = false;
bool showBounding = false;

glm::vec3 handPos;
glm::vec3 shootDir;
glm::vec3 lEyePos;
glm::vec3 rEyePos;
glm::vec3 headPos;

glm::mat4 rotationMtx=mat4(1.0);
glm::mat4 prevMt;
glm::mat4 camMt;
glm::mat4 ctrMt;
glm::mat4 curPose;
glm::mat4 prevPose;

boost::circular_buffer<glm::mat4> ringBuf(30);
boost::circular_buffer<glm::vec3> ctrBuf(30);
// Sound System
irrklang::ISoundEngine * SoundEngine1;
irrklang::ISoundEngine * SoundEngine2;


bool RT;
bool LT;

///////////////////////////////////////////////////////////////////////////////
//
// GLEW gives cross platform access to OpenGL 3.x+ functionality.  
//

#include <GL/glew.h>

bool checkFramebufferStatus(GLenum target = GL_FRAMEBUFFER)
{
  GLuint status = glCheckFramebufferStatus(target);
  switch (status)
  {
  case GL_FRAMEBUFFER_COMPLETE:
    return true;
    break;

  case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
    std::cerr << "framebuffer incomplete attachment" << std::endl;
    break;

  case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
    std::cerr << "framebuffer missing attachment" << std::endl;
    break;

  case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
    std::cerr << "framebuffer incomplete draw buffer" << std::endl;
    break;

  case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
    std::cerr << "framebuffer incomplete read buffer" << std::endl;
    break;

  case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
    std::cerr << "framebuffer incomplete multisample" << std::endl;
    break;

  case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
    std::cerr << "framebuffer incomplete layer targets" << std::endl;
    break;

  case GL_FRAMEBUFFER_UNSUPPORTED:
    std::cerr << "framebuffer unsupported internal format or image" << std::endl;
    break;

  default:
    std::cerr << "other framebuffer error" << std::endl;
    break;
  }

  return false;
}

bool checkGlError()
{
  GLenum error = glGetError();
  if (!error)
  {
    return false;
  }
  else
  {
    switch (error)
    {
    case GL_INVALID_ENUM:
      std::cerr <<
        ": An unacceptable value is specified for an enumerated argument.The offending command is ignored and has no other side effect than to set the error flag.";
      break;
    case GL_INVALID_VALUE:
      std::cerr <<
        ": A numeric argument is out of range.The offending command is ignored and has no other side effect than to set the error flag";
      break;
    case GL_INVALID_OPERATION:
      std::cerr <<
        ": The specified operation is not allowed in the current state.The offending command is ignored and has no other side effect than to set the error flag..";
      break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      std::cerr <<
        ": The framebuffer object is not complete.The offending command is ignored and has no other side effect than to set the error flag.";
      break;
    case GL_OUT_OF_MEMORY:
      std::cerr <<
        ": There is not enough memory left to execute the command.The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
      break;
    case GL_STACK_UNDERFLOW:
      std::cerr <<
        ": An attempt has been made to perform an operation that would cause an internal stack to underflow.";
      break;
    case GL_STACK_OVERFLOW:
      std::cerr << ": An attempt has been made to perform an operation that would cause an internal stack to overflow.";
      break;
    }
    return true;
  }
}

void glDebugCallbackHandler(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* msg,
                            GLvoid* data)
{
  OutputDebugStringA(msg);
  std::cout << "debug call: " << msg << std::endl;
}

//////////////////////////////////////////////////////////////////////
//
// GLFW provides cross platform window creation
//

#include <GLFW/glfw3.h>

namespace glfw
{
  inline GLFWwindow* createWindow(const uvec2& size, const ivec2& position = ivec2(INT_MIN))
  {
    GLFWwindow* window = glfwCreateWindow(size.x, size.y, "glfw", nullptr, nullptr);
    if (!window)
    {
      FAIL("Unable to create rendering window");
    }
    if ((position.x > INT_MIN) && (position.y > INT_MIN))
    {
      glfwSetWindowPos(window, position.x, position.y);
    }
    return window;
  }
}

// A class to encapsulate using GLFW to handle input and render a scene
class GlfwApp
{
protected:
  uvec2 windowSize;
  ivec2 windowPosition;
  GLFWwindow* window{nullptr};
  unsigned int frame{0};

public:
  GlfwApp()
  {
    // Initialize the GLFW system for creating and positioning windows
    if (!glfwInit())
    {
      FAIL("Failed to initialize GLFW");
    }
    glfwSetErrorCallback(ErrorCallback);
  }

  virtual ~GlfwApp()
  {
    if (nullptr != window)
    {
      glfwDestroyWindow(window);
    }
    glfwTerminate();
  }

  virtual int run()
  {
    preCreate();

    window = createRenderingTarget(windowSize, windowPosition);

    if (!window)
    {
      std::cout << "Unable to create OpenGL window" << std::endl;
      return -1;
    }

    postCreate();

    initGl();

    while (!glfwWindowShouldClose(window))
    {
      ++frame;
      glfwPollEvents();
      update();
      draw();
      finishFrame();
    }

    shutdownGl();

    return 0;
  }

protected:
  virtual GLFWwindow* createRenderingTarget(uvec2& size, ivec2& pos) = 0;

  virtual void draw() = 0;

  void preCreate()
  {
    glfwWindowHint(GLFW_DEPTH_BITS, 16);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
  }

  void postCreate()
  {
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwMakeContextCurrent(window);

    // Initialize the OpenGL bindings
    // For some reason we have to set this experminetal flag to properly
    // init GLEW if we use a core context.
    glewExperimental = GL_TRUE;
    if (0 != glewInit())
    {
      FAIL("Failed to initialize GLEW");
    }
    glGetError();

    if (GLEW_KHR_debug)
    {
      GLint v;
      glGetIntegerv(GL_CONTEXT_FLAGS, &v);
      if (v & GL_CONTEXT_FLAG_DEBUG_BIT)
      {
        //glDebugMessageCallback(glDebugCallbackHandler, this);
      }
    }
  }

  virtual void initGl()
  {
  }

  virtual void shutdownGl()
  {
  }

  virtual void finishFrame()
  {
    glfwSwapBuffers(window);
  }

  virtual void destroyWindow()
  {
    glfwSetKeyCallback(window, nullptr);
    glfwSetMouseButtonCallback(window, nullptr);
    glfwDestroyWindow(window);
  }

  virtual void onKey(int key, int scancode, int action, int mods)
  {
    if (GLFW_PRESS != action)
    {
      return;
    }

    switch (key)
    {
    case GLFW_KEY_ESCAPE:
      glfwSetWindowShouldClose(window, 1);
      return;
    }
  }

  virtual void update()
  {
  }

  virtual void onMouseButton(int button, int action, int mods)
  {
  }

protected:
  virtual void viewport(const ivec2& pos, const uvec2& size)
  {
    glViewport(pos.x, pos.y, size.x, size.y);
  }

private:

  static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
  {
    GlfwApp* instance = (GlfwApp *)glfwGetWindowUserPointer(window);
    instance->onKey(key, scancode, action, mods);
  }

  static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
  {
    GlfwApp* instance = (GlfwApp *)glfwGetWindowUserPointer(window);
    instance->onMouseButton(button, action, mods);
  }

  static void ErrorCallback(int error, const char* description)
  {
    FAIL(description);
  }
};

//////////////////////////////////////////////////////////////////////
//
// The Oculus VR C API provides access to information about the HMD
//

#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>

namespace ovr
{
  // Convenience method for looping over each eye with a lambda
  template <typename Function>
  inline void for_each_eye(Function function)
  {
    for (ovrEyeType eye = ovrEyeType::ovrEye_Left;
         eye < ovrEyeType::ovrEye_Count;
         eye = static_cast<ovrEyeType>(eye + 1))
    {
      function(eye);
    }
  }

  inline mat4 toGlm(const ovrMatrix4f& om)
  {
    return glm::transpose(glm::make_mat4(&om.M[0][0]));
  }

  inline mat4 toGlm(const ovrFovPort& fovport, float nearPlane = 0.01f, float farPlane = 10000.0f)
  {
    return toGlm(ovrMatrix4f_Projection(fovport, nearPlane, farPlane, true));
  }

  inline vec3 toGlm(const ovrVector3f& ov)
  {
    return glm::make_vec3(&ov.x);
  }

  inline vec2 toGlm(const ovrVector2f& ov)
  {
    return glm::make_vec2(&ov.x);
  }

  inline uvec2 toGlm(const ovrSizei& ov)
  {
    return uvec2(ov.w, ov.h);
  }

  inline quat toGlm(const ovrQuatf& oq)
  {
    return glm::make_quat(&oq.x);
  }

  inline mat4 toGlm(const ovrPosef& op)
  {
    mat4 orientation = glm::mat4_cast(toGlm(op.Orientation));
    mat4 translation = glm::translate(mat4(), ovr::toGlm(op.Position));
    return translation * orientation;
  }

  inline ovrMatrix4f fromGlm(const mat4& m)
  {
    ovrMatrix4f result;
    mat4 transposed(glm::transpose(m));
    memcpy(result.M, &(transposed[0][0]), sizeof(float) * 16);
    return result;
  }

  inline ovrVector3f fromGlm(const vec3& v)
  {
    ovrVector3f result;
    result.x = v.x;
    result.y = v.y;
    result.z = v.z;
    return result;
  }

  inline ovrVector2f fromGlm(const vec2& v)
  {
    ovrVector2f result;
    result.x = v.x;
    result.y = v.y;
    return result;
  }

  inline ovrSizei fromGlm(const uvec2& v)
  {
    ovrSizei result;
    result.w = v.x;
    result.h = v.y;
    return result;
  }

  inline ovrQuatf fromGlm(const quat& q)
  {
    ovrQuatf result;
    result.x = q.x;
    result.y = q.y;
    result.z = q.z;
    result.w = q.w;
    return result;
  }
}

class RiftManagerApp
{
protected:
  ovrSession _session;
  ovrHmdDesc _hmdDesc;
  ovrGraphicsLuid _luid;

public:
  RiftManagerApp()
  {
    if (!OVR_SUCCESS(ovr_Create(&_session, &_luid)))
    {
      FAIL("Unable to create HMD session");
    }

    _hmdDesc = ovr_GetHmdDesc(_session);
  }

  ~RiftManagerApp()
  {
    ovr_Destroy(_session);
    _session = nullptr;
  }
};

#include <glm/gtx/string_cast.hpp>
class RiftApp : public GlfwApp, public RiftManagerApp
{
public:

private:
  GLuint _fbo{0};
  GLuint _depthBuffer{0};
  ovrTextureSwapChain _eyeTexture;

  GLuint _mirrorFbo{0};
  ovrMirrorTexture _mirrorTexture;

  ovrEyeRenderDesc _eyeRenderDescs[2];

  mat4 _eyeProjections[2];
  ovrPosef lastEye[2], renderEye[2];

  ovrLayerEyeFov _sceneLayer;
  ovrViewScaleDesc _viewScaleDesc;

  uvec2 _renderTargetSize;
  uvec2 _mirrorSize;

public:

  RiftApp()
  {
    using namespace ovr;
    _viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;

    memset(&_sceneLayer, 0, sizeof(ovrLayerEyeFov));
    _sceneLayer.Header.Type = ovrLayerType_EyeFov;
    _sceneLayer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;

    ovr::for_each_eye([&](ovrEyeType eye)
    {
      ovrEyeRenderDesc& erd = _eyeRenderDescs[eye] = ovr_GetRenderDesc(_session, eye, _hmdDesc.DefaultEyeFov[eye]);
      ovrMatrix4f ovrPerspectiveProjection =
        ovrMatrix4f_Projection(erd.Fov, 0.01f, 1000.0f, ovrProjection_ClipRangeOpenGL);
      _eyeProjections[eye] = ovr::toGlm(ovrPerspectiveProjection);
      _viewScaleDesc.HmdToEyePose[eye] = erd.HmdToEyePose;

	  iod = abs(_viewScaleDesc.HmdToEyePose[0].Position.x - _viewScaleDesc.HmdToEyePose[1].Position.x);
	  original_iod = abs(_viewScaleDesc.HmdToEyePose[0].Position.x - _viewScaleDesc.HmdToEyePose[1].Position.x);
      ovrFovPort& fov = _sceneLayer.Fov[eye] = _eyeRenderDescs[eye].Fov;
      auto eyeSize = ovr_GetFovTextureSize(_session, eye, fov, 1.0f);
      _sceneLayer.Viewport[eye].Size = eyeSize;
      _sceneLayer.Viewport[eye].Pos = {(int)_renderTargetSize.x, 0};

      _renderTargetSize.y = std::max(_renderTargetSize.y, (uint32_t)eyeSize.h);
      _renderTargetSize.x += eyeSize.w;
    });
    // Make the on screen window 1/4 the resolution of the render target
    _mirrorSize = _renderTargetSize;
    _mirrorSize /= 4;
  }

protected:
  GLFWwindow* createRenderingTarget(uvec2& outSize, ivec2& outPosition) override
  {
    return glfw::createWindow(_mirrorSize);
  }

  void initGl() override
  {
    GlfwApp::initGl();

    // Disable the v-sync for buffer swap
    glfwSwapInterval(0);

    ovrTextureSwapChainDesc desc = {};
    desc.Type = ovrTexture_2D;
    desc.ArraySize = 1;
    desc.Width = _renderTargetSize.x;
    desc.Height = _renderTargetSize.y;
    desc.MipLevels = 1;
    desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
    desc.SampleCount = 1;
    desc.StaticImage = ovrFalse;
    ovrResult result = ovr_CreateTextureSwapChainGL(_session, &desc, &_eyeTexture);
    _sceneLayer.ColorTexture[0] = _eyeTexture;
    if (!OVR_SUCCESS(result))
    {
      FAIL("Failed to create swap textures");
    }

    int length = 0;
    result = ovr_GetTextureSwapChainLength(_session, _eyeTexture, &length);
    if (!OVR_SUCCESS(result) || !length)
    {
      FAIL("Unable to count swap chain textures");
    }
    for (int i = 0; i < length; ++i)
    {
      GLuint chainTexId;
      ovr_GetTextureSwapChainBufferGL(_session, _eyeTexture, i, &chainTexId);
      glBindTexture(GL_TEXTURE_2D, chainTexId);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    // Set up the framebuffer object
    glGenFramebuffers(1, &_fbo);
    glGenRenderbuffers(1, &_depthBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, _depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _renderTargetSize.x, _renderTargetSize.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    ovrMirrorTextureDesc mirrorDesc;
    memset(&mirrorDesc, 0, sizeof(mirrorDesc));
    mirrorDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
    mirrorDesc.Width = _mirrorSize.x;
    mirrorDesc.Height = _mirrorSize.y;
    if (!OVR_SUCCESS(ovr_CreateMirrorTextureGL(_session, &mirrorDesc, &_mirrorTexture)))
    {
      FAIL("Could not create mirror texture");
    }
    glGenFramebuffers(1, &_mirrorFbo);
  }

  void onKey(int key, int scancode, int action, int mods) override
  {
    if (GLFW_PRESS == action)
      switch (key)
      {
      case GLFW_KEY_R:
        ovr_RecenterTrackingOrigin(_session);
        return;
      }

    GlfwApp::onKey(key, scancode, action, mods);
  }


  void draw() final override
  {

	  //controller
	  ovrInputState inputState;
	  // Query Touch controllers. Query their parameters:
	  double displayMidpointSeconds = ovr_GetPredictedDisplayTime(_session, 0);
	  ovrTrackingState trackState = ovr_GetTrackingState(_session, displayMidpointSeconds, ovrTrue);

	  // Process controller status. Useful to know if controller is being used at all, and if the cameras can see it. 
	  // Bits reported:
	  // Bit 1: ovrStatus_OrientationTracked  = Orientation is currently tracked (connected and in use)
	  // Bit 2: ovrStatus_PositionTracked     = Position is currently tracked (false if out of range)
	  unsigned int handStatus[2];
	  handStatus[0] = trackState.HandStatusFlags[0];
	  handStatus[1] = trackState.HandStatusFlags[1];
	  // Display status for debug purposes:
	  //std::cerr << "handStatus[left]  = " << handStatus[ovrHand_Left] << std::endl;
	  //std::cerr << "handStatus[right] = " << handStatus[ovrHand_Right] << std::endl;

	  // Process controller position and orientation:
	  ovrPosef handPoses[2];  // These are position and orientation in meters in room coordinates, relative to tracking origin. Right-handed cartesian coordinates.
								// ovrQuatf     Orientation;
								// ovrVector3f  Position;
	  handPoses[0] = trackState.HandPoses[0].ThePose;
	  handPoses[1] = trackState.HandPoses[1].ThePose;

	  

	  ovrVector3f handPosition[2];
	  handPosition[0] = handPoses[0].Position;
	  handPosition[1] = handPoses[1].Position;

	  //get shooting direction
	  glm::quat orientation=ovr::toGlm(handPoses[1].Orientation);
	  glm::mat4 rotMtx = glm::mat4_cast(orientation);
	  glm::vec4 forward = glm::inverse(rotMtx)*glm::vec4(0, 0, -1, 1);
	  shootDir = glm::vec3(forward);
	  ctrBuf.push_back(ovr::toGlm(handPosition[ovrHand_Right]));
	 
	  handPos = ovr::toGlm(handPosition[ovrHand_Right]);
	  rotationMtx = rotMtx;
	 // std::cout << glm::to_string(rotationMtx) << std::endl;


    ovrPosef eyePoses[2];
    ovr_GetEyePoses(_session, frame, true, _viewScaleDesc.HmdToEyePose, eyePoses, &_sceneLayer.SensorSampleTime);

    int curIndex;
    ovr_GetTextureSwapChainCurrentIndex(_session, _eyeTexture, &curIndex);
    GLuint curTexId;
    ovr_GetTextureSwapChainBufferGL(_session, _eyeTexture, curIndex, &curTexId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ovr::for_each_eye([&](ovrEyeType eye) {
		renderEye[eye] = lastEye[eye];
		if (getBState() == 0) {
			renderEye[eye] = eyePoses[eye];
		}
		else if (getBState() == 1) {
			renderEye[eye].Position = eyePoses[eye].Position;
		}
		else if (getBState() == 2) {
			renderEye[eye].Orientation = eyePoses[eye].Orientation;
		}
		lastEye[eye] = renderEye[eye];

		lEyePos = glm::vec3(ovr::toGlm(renderEye[ovrEye_Left])[3][0], ovr::toGlm(renderEye[ovrEye_Left])[3][1], ovr::toGlm(renderEye[ovrEye_Left])[3][2]);
		rEyePos = glm::vec3(ovr::toGlm(renderEye[ovrEye_Right])[3][0], ovr::toGlm(renderEye[ovrEye_Right])[3][1], ovr::toGlm(renderEye[ovrEye_Right])[3][2]);
		headPos = (lEyePos + rEyePos) / 2.0f;
		/*if (getXState() == 0) {
			const auto& vp = _sceneLayer.Viewport[eye];
			glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);
			_sceneLayer.RenderPose[eye] = eyePoses[eye];
			renderScene(_eyeProjections[eye], ovr::toGlm(eyePoses[eye]));
		}
		else if (getXState() == 1) {
			
			const auto& vp = _sceneLayer.Viewport[eye];
			glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);
			_sceneLayer.RenderPose[eye] = eyePoses[ovrEye_Left];
			renderScene(_eyeProjections[ovrEye_Left], ovr::toGlm(renderEye[ovrEye_Left]));
		}*/
		const auto& vp = _sceneLayer.Viewport[eye];
		glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);
		_sceneLayer.RenderPose[eye] = eyePoses[eye];
		
			if (eye == ovrEye_Left) {
				isLeft = true;
				renderScene(_eyeProjections[ovrEye_Left], ovr::toGlm(renderEye[ovrEye_Left]), true);
				
			}
			else {
				isLeft = false;
				renderScene(_eyeProjections[ovrEye_Right], ovr::toGlm(renderEye[ovrEye_Right]), false);
				
			}
			
	});
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    ovr_CommitTextureSwapChain(_session, _eyeTexture);
    ovrLayerHeader* headerList = &_sceneLayer.Header;
    ovr_SubmitFrame(_session, frame, &_viewScaleDesc, &headerList, 1);

    GLuint mirrorTextureId;
    ovr_GetMirrorTextureBufferGL(_session, _mirrorTexture, &mirrorTextureId);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _mirrorFbo);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTextureId, 0);
    glBlitFramebuffer(0, 0, _mirrorSize.x, _mirrorSize.y, 0, _mirrorSize.y, _mirrorSize.x, 0, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	_viewScaleDesc.HmdToEyePose[0].Position.x = (float)(-iod / 2);
	_viewScaleDesc.HmdToEyePose[1].Position.x = (float)(iod / 2);
  }

  virtual void renderScene(const glm::mat4& projection, const glm::mat4& headPose,bool left) = 0;
  virtual int getAState() = 0;
  virtual int getBState() = 0;
  virtual int getXState() = 0;
};

//////////////////////////////////////////////////////////////////////
//
// The remainder of this code is specific to the scene we want to 
// render.  I use glfw to render an array of cubes, but your 
// application would perform whatever rendering you want
//

struct Light {
	glm::vec3 direction;
	glm::vec3 color;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
};


#include <vector>
#include "shader.h"
#include "Cube.h"

// a class for building and rendering cubes
class Scene
{


  
  // Program
  std::vector<glm::mat4> instance_positions;
  GLuint instanceCount;
  GLuint shaderID;
  GLuint sphereShader;
  GLuint boundingShader;
  GLuint bulletShader;
  GLuint modelShader;

  GLuint uProjection, uModelview, model;

  std::unique_ptr<TexturedCube> cube;
  std::unique_ptr<Skybox> skybox_l;
  std::unique_ptr<Skybox> skybox_r;
  std::unique_ptr<Skybox> skybox;

  Model* gun;
  Model* bullet;
  BoundingBox* modelBounding,*bulletBounding;

  Light light;

  

  const unsigned int GRID_SIZE{5};

public:
	int buttonA = 0, buttonB = 0, buttonX = 0;
	bool buttonAPressed = false, buttonBPressed = false, buttonXPressed = false,LTPressed=false,RTPressed=false;
	bool LHPressed = false, RHPressed = false, buttonYPressed=false;
	float scalor = 0.1f;
	int eye;

	

  Scene()
  {
    // Create two cube
    instance_positions.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -0.3)));
    instance_positions.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -0.9)));

    instanceCount = instance_positions.size();

    // Shader Program 
    shaderID = LoadShaders("skybox.vert", "skybox.frag");
	sphereShader = LoadShaders(SPHERE_VERT, SPHERE_FRAG);
	boundingShader = LoadShaders(BOUNDING_VERT, BOUNDING_FRAG);
	bulletShader = LoadShaders(BULLET_VERT, BULLET_FRAG);
	modelShader = LoadShaders(MODEL_VERT, MODEL_FRAG);
	//models
    cube = std::make_unique<TexturedCube>("cube"); 
	gun = new Model("model/gun/schofield-pistol-low.obj");
	bullet = new Model("sphere.obj");
	bullet->toWorld = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
	  // 10m wide sky box: size doesn't matter though
    skybox_l = std::make_unique<Skybox>("skybox");
	skybox_l->toWorld = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f));
	skybox_r = std::make_unique<Skybox>("skybox");
	skybox_r->toWorld = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f));
	skybox = std::make_unique<Skybox>("skybox");
	skybox->toWorld = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f));
	//initialize bounding boxes
	bullet = new Model("sphere.obj");
	bulletBounding = new BoundingBox(bullet->boundingbox, bullet->boxVertices);
	
	//TODO
	//temp model bouding box

	//init sound
	SoundEngine1 = irrklang::createIrrKlangDevice();
	SoundEngine2 = irrklang::createIrrKlangDevice();
	//init light
	light.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
	light.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
	light.diffuse = glm::vec3(0.5f);
	light.specular = glm::vec3(1.0f);

	SoundEngine2->setSoundVolume ( 0.2);
	SoundEngine2->play2D(BGM, GL_TRUE);
	
  }

  ~Scene() {
	  delete(gun);
	  delete(bulletBounding);
	  delete(SoundEngine1);
	  delete(SoundEngine2);
	  glDeleteProgram(shaderID);
	  glDeleteProgram(sphereShader);
	  glDeleteProgram(boundingShader);
	  glDeleteProgram(bulletShader);
  }

  void render(const glm::mat4& projection, const glm::mat4& view,bool left)
  {
	  glm::mat4 inverse = glm::translate(glm::mat4(1.0f), -handPos);
	  glm::mat4 T = glm::translate(glm::mat4(1.0f), handPos);
	  glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.001f, 0.001f, 0.001f));
	  glm::mat4 modelMatrix = T * rotationMtx*scale*inverse;
	  modelMatrix = modelMatrix * glm::rotate(glm::mat4(1.0), 1.01f* glm::pi<float>(), glm::vec3(0, 1, 0));
	  //glm::rotate(modelMatrix,0.5, glm::vec3(0, 1, 0));
	 // modelMatrix
	  
	  glUseProgram(modelShader);
	  setUpLight();
	  uProjection = glGetUniformLocation(modelShader, "projection");
	  uModelview = glGetUniformLocation(modelShader, "view");
	  model = glGetUniformLocation(modelShader, "model");
	  // Now send these values to the shader program
	  glUniformMatrix4fv(uProjection, 1, GL_FALSE, &projection[0][0]);
	  glUniformMatrix4fv(uModelview, 1, GL_FALSE, &view[0][0]);
	  glUniformMatrix4fv(model, 1, GL_FALSE, &modelMatrix[0][0]);
	  gun->Draw(modelShader);
	  if (RT) {
		  cout << rotationMtx[0][0] << endl;
		  cout << rotationMtx[0][1] << endl;
		  cout << rotationMtx[0][2] << endl;
	  }


	  //set bullet position when not firing
	  glUseProgram(bulletShader);
	  if (!fire) {
		  inverse = glm::translate(glm::mat4(1.0f), -handPos);
		  T = glm::translate(glm::mat4(1.0f), handPos);
		  scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 0.05f, 0.05f));
		  modelMatrix = T * rotationMtx*scale*inverse*bullet->toWorld;
		  
		  uProjection = glGetUniformLocation(bulletShader, "projection");
		  uModelview = glGetUniformLocation(bulletShader, "view");
		  model = glGetUniformLocation(bulletShader, "model");
		  // Now send these values to the shader program
		  glUniformMatrix4fv(uProjection, 1, GL_FALSE, &projection[0][0]);
		  glUniformMatrix4fv(uModelview, 1, GL_FALSE, &view[0][0]);
		  glUniformMatrix4fv(model, 1, GL_FALSE, &modelMatrix[0][0]);
		  //bullet->Draw(bulletShader);
		 
		
	  }
	  //if firing
	  else if (fire) {
		
		  inverse = glm::translate(glm::mat4(1.0f), -handPos);
		  T = glm::translate(glm::mat4(1.0f), handPos);
		  scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 0.08f));
		  modelMatrix = T * rotationMtx*scale*inverse*bullet->toWorld;
		  uProjection = glGetUniformLocation(bulletShader, "projection");
		  uModelview = glGetUniformLocation(bulletShader, "view");
		  model = glGetUniformLocation(bulletShader, "model");
		  // Now send these values to the shader program
		  glUniformMatrix4fv(uProjection, 1, GL_FALSE, &projection[0][0]);
		  glUniformMatrix4fv(uModelview, 1, GL_FALSE, &view[0][0]);
		  glUniformMatrix4fv(model, 1, GL_FALSE, &modelMatrix[0][0]);
		  if (!bullet->isFired) {
			  bullet->viewdir = shootDir;
		  }
		  bullet->duration = 250;
		  bullet->Draw(bulletShader);
		  cout << glm::to_string(bullet->toWorld )<< endl;
		  //bullet->toWorld = gun->toworld;
		  if (bullet->duration != 0) {
			  bullet->Draw(bulletShader);
			  finishFire = false;
		  }
		 
		  
		  bullet->fire();
		  bulletBounding->toWorld = bullet->toWorld;
		  if (!soundPlayed) {
			  SoundEngine2->setSoundVolume( 0.2);
			  //SoundEngine2->play2D(FIRING_BGM, GL_TRUE);
		  }
		  SoundEngine1->stopAllSounds();
		  fired = true;
	  }
	  if (finishFire) {
		  bullet = new Model("sphere.obj");
		  fired = false;
		  inverse = glm::translate(glm::mat4(1.0f), -handPos);
		  T = glm::translate(glm::mat4(1.0f), handPos);
		  scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
		  modelMatrix = T * scale*inverse*bullet->toWorld;

		  uProjection = glGetUniformLocation(bulletShader, "projection");
		  uModelview = glGetUniformLocation(bulletShader, "view");
		  model = glGetUniformLocation(bulletShader, "model");
		  // Now send these values to the shader program
		  glUniformMatrix4fv(uProjection, 1, GL_FALSE, &projection[0][0]);
		  glUniformMatrix4fv(uModelview, 1, GL_FALSE, &view[0][0]);
		  glUniformMatrix4fv(model, 1, GL_FALSE, &modelMatrix[0][0]);
		  //bullet->toWorld = gun->toworld;
		  bullet->Draw(sphereShader);

	  }
	  if (RT) {
		 
			  SoundEngine1->setSoundVolume(0.3);
			  // SoundEngine1->s
			 // SoundEngine1->play2D(SOUND_PATH, GL_TRUE);
			  //SoundEngine1->play2D(SHELL_PATH, GL_TRUE);
			  // SoundEngine1->stopAllSounds();
		  
		  
	  }
	
	  if (bullet->duration == 0) {
		  finishFire = true;
	  }
	  if (fired) {
		  
		  
	  }
	  
	 
	  //show bouding boxes
	  if (showBounding) {
		  glUseProgram(boundingShader);
		  bulletBounding->draw(boundingShader, projection, view);
	  }
	  //draw skybox
	  glUseProgram(shaderID);
	
	  if (left) {
	    // Render Skybox : remove view translation
	    skybox_l->draw(shaderID, projection, view);
	  }
	  else {
		skybox_r->draw(shaderID, projection, view);
	  }
	  cube->toWorld = camMt * glm::scale(glm::mat4(1.0f), glm::vec3(scalor));
	  cube->draw(shaderID, projection, view);
		
	
      //set lighting and model shaders
	  //for the model
	  setUpLight();
	  inverse = glm::translate(glm::mat4(1.0f), -headPos);
	 
	  T = glm::translate(glm::mat4(1.0f), headPos);
	  //for gun picking if we are gonna implement that
	  // T = glm::translate(glm::mat4(1.0f), glm::vec3(headPos.x+0.5f,headPos.y-2.0f,headPos.z));
	  scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 0.05f, 0.05f));
	  modelMatrix = T * scale*inverse;
	  uProjection = glGetUniformLocation(modelShader, "projection");
	  uModelview = glGetUniformLocation(modelShader, "view");
	  model = glGetUniformLocation(modelShader, "model");
    
  }

  void setUpLight() {
	  glUseProgram(modelShader);
	  glUniform3f(glGetUniformLocation(modelShader, "light.direction"),light.direction.x, light.direction.y, light.direction.z);
	  glUniform3f(glGetUniformLocation(modelShader, "light.ambient"), light.ambient.x, light.ambient.y, light.ambient.z);
	  glUniform3f(glGetUniformLocation(modelShader, "light.diffuse"), light.diffuse.x, light.diffuse.y, light.diffuse.z);
	  glUniform3f(glGetUniformLocation(modelShader, "light.specular"), light.specular.x, light.specular.y, light.specular.z);
	  glUniform3f(glGetUniformLocation(modelShader, "viewPos"), headPos.x,headPos.y,headPos.z);
	  glUniform1i(glGetUniformLocation(modelShader, "material.diffuse"), 0);
	  glUniform1i(glGetUniformLocation(modelShader, "material.specular"), 1);


	  
  }


  void checkcollision() {
	  std::vector<float> bound1 = bulletBounding->getBoundary();
	  //TODO
	  //model bounding hasnt been initialized since models are not done yet
	  std::vector<float> bound2 = modelBounding->getBoundary();

	  // check bound
	  if (bound1[0] > bound2[1] && bound1[2] > bound2[3] && bound1[4] > bound2[5] &&
		  bound1[1] < bound2[0] && bound1[3] < bound2[2] && bound1[5] < bound2[4]) {
		  bulletBounding->collisionflag = true;
		  modelBounding->collisionflag = true;
		  //TODO
		  //need a new model
		  //Model->dying=true;
	  }
	  else {
		  bulletBounding->collisionflag = false;
		  modelBounding->collisionflag = false;
	  }




  }

};

// An example application that renders a simple cube
class ExampleApp : public RiftApp
{
  std::shared_ptr<Scene> scene;


public:
  ExampleApp()
  {
  }

protected:
  void initGl() override
  {
    RiftApp::initGl();
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    ovr_RecenterTrackingOrigin(_session);
    scene = std::shared_ptr<Scene>(new Scene());
	std::cout << "Tracking lag: " << frameLag << " frames" << std::endl;
	std::cout << "Rendering delay : " << renderLag << " frames" << std::endl;
  }

  void shutdownGl() override
  {
    scene.reset();
  }
  void update() final override {
	  //TODO
	  //check collision
	 // scene->checkcollision();

	  //handle button
	  ovrInputState inputState;
	  if (OVR_SUCCESS(ovr_GetInputState(_session, ovrControllerType_Touch, &inputState))) {
		  if (inputState.Buttons & ovrButton_A) {
			  scene->buttonAPressed = true;
			  showBounding = true;
		  }
		  else if (scene->buttonAPressed) {
			  scene->buttonA = (scene->buttonA + 1) % 2; scene->buttonAPressed = false; showBounding = false;
		  }
		  if (inputState.Buttons & ovrButton_B) scene->buttonBPressed = true;
		  else if (scene->buttonBPressed) {
			  scene->buttonB = (scene->buttonB + 1) % 4; scene->buttonBPressed = false;
		  }
		  if (inputState.Buttons & ovrButton_X) scene->buttonXPressed = true;
		  else if (scene->buttonXPressed) {
			  scene->buttonX = (scene->buttonX + 1) % 4; scene->buttonXPressed = false;
		  }

		  if (inputState.Buttons & ovrButton_RThumb) iod = original_iod;
		  else {
			  if (inputState.Thumbstick[ovrHand_Right].x > 0.5f) iod =std::min(iod + 0.001f,0.3);
			  else if (inputState.Thumbstick[ovrHand_Right].x < -0.5f) iod=std::max(iod - 0.001f,-0.1);
		  }
		  if (inputState.Buttons & ovrButton_LThumb) scene->scalor = 0.03f;
		  else {
			  if (inputState.Thumbstick[ovrHand_Left].x > 0.5f) scene->scalor = std::min(scene->scalor + 0.001f, 0.5f);
			  else if (inputState.Thumbstick[ovrHand_Left].x < -0.5f) scene->scalor = std::max(scene->scalor - 0.001f, 0.01f);
		  }
		  //index
		  if (inputState.IndexTrigger[ovrHand_Right] > 0.5f) {
			  scene->RTPressed = true;
			  fire = true;
			  RT = true;
		  }
		 
		  else if(scene->RTPressed){
			  RT = false;
		

			 
			  scene->RTPressed = false;
			  //RT = false;
		  
		  }
		 
		  if (inputState.IndexTrigger[ovrHand_Left] > 0.5f) {
			  scene->LTPressed = true;
			  
		  }
		  else if (scene->LTPressed) {
		
			  scene->LTPressed = false;
			  
		  }

		  //hand
		  if (inputState.HandTrigger[ovrHand_Right] > 0.5f) scene->RHPressed = true;
		  else if (scene->RHPressed) {
			
			  
			  scene->RHPressed = false;
		  }
		  if (inputState.HandTrigger[ovrHand_Left] > 0.5f) scene->LHPressed = true;
		  else if (scene->LHPressed) {
			  
			  
			  std::cout << "Rendering delay : " << renderLag << " frames" << std::endl;
			  scene->LHPressed = false;
		  }

		  if (inputState.Buttons & ovrButton_Y) {
			  scene->buttonYPressed = true;
		  }
		  else if (scene->buttonYPressed) {
			  if (superRot) {
				  scene->buttonYPressed = false;
				  std::cout << "super rotation deactivated!\n";
				  superRot = false;
			  }

			  else {
				  scene->buttonYPressed = false;
				  std::cout << "super rotation activated!\n";
				  superRot = true;
			  }

		  }
	  }
  }

  mat3 rotation(float rotX, float rotY, float rotZ) {
	  mat3 X, Y, Z = mat3(1.0f);

	  X[1][1] = cosf(rotX);
	  X[2][1] = -sinf(rotX);
	  X[1][2] = sinf(rotX);
	  X[2][2] = cosf(rotX);

	  Y[0][0] = cosf(rotY);
	  Y[0][2] = -sinf(rotY);
	  Y[2][0] = sinf(rotY);
	  Y[2][2] = cosf(rotY);

	  Z[0][0] = cosf(rotZ);
	  Z[0][1] = sinf(rotZ);
	  Z[1][0] = -sinf(rotZ);
	  Z[1][1] = cosf(rotZ);

	  mat3 M = Z * Y * X;
	  return M;
  }


  void renderScene(const glm::mat4& projection, const glm::mat4& headPose,bool left) override
  {
	  curPose = headPose;
	  

	  camMt = headPose;
	 
	  ringBuf.push_back(camMt);
	  if (superRot) {
		  glm::mat3 temp = glm::mat3(curPose[0], curPose[1], curPose[2]);
		  float x = atan2f(temp[0][0], temp[0][2]);
		  float y = atan2f(-temp[1][1], temp[2][1]);
		  float z = atan2f(-temp[0][1], sqrt(pow(temp[1][1], 2) + pow(temp[2][1], 2)));
		  glm::mat3 temp2 = rotation(x, -y*2, z);
		  glm::mat4 T = glm::mat4(temp2);
		  T[3] = curPose[3];
		  scene->render(projection, glm::inverse(T), left);
	 }
	  else {
		  if (frameLag == 0) {
			  scene->render(projection, glm::inverse(headPose), left);

		  }
		  else {
			  if (isLeft) {
				  int tempIndex = std::max(0, (frameHead - frameLag) % 30);
				  if (tempIndex < 0) tempIndex = 0;
				  //or 30-framLag+frameHead%30
				  glm::mat4 prevMat = ringBuf.at(tempIndex);
				  scene->render(projection, glm::inverse(prevMat), true);
			  }
			  else {
				  int tempIndex = std::max(0, (frameHead - frameLag) % 30);
				  if (tempIndex < 0) tempIndex = 0;
				  //or 30-framLag+frameHead%30
				  glm::mat4 prevMat = ringBuf.at(tempIndex);
				  scene->render(projection, glm::inverse(prevMat), false);
			  }

		  }
	  }
	 // std::cout << "Tracking lag: " << frameLag << " frames" << std::endl;
	  ++frameHead;
  }
  int getAState() { return scene->buttonA; }
  int getBState() { return scene->buttonB; }
  int getXState() { return scene->buttonX; }
};




// Execute our example class
int main(int argc, char** argv)
{
  int result = -1;

  if (!OVR_SUCCESS(ovr_Initialize(nullptr)))
  {
    FAIL("Failed to initialize the Oculus SDK");
  }
  result = ExampleApp().run();

  ovr_Shutdown();
  return result;
}
