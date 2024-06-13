#include <EGL/egl.h>
#include <GL/glew.h>

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "prepare_ogl.h"

// clang-format off
static const EGLint configAttribs[] = {
  EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
  EGL_BLUE_SIZE, 8,
  EGL_GREEN_SIZE, 8,
  EGL_RED_SIZE, 8,
  EGL_DEPTH_SIZE, 8,
  EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
  EGL_NONE
};
// clang-format on

class MaskRenderer {
 public:
  std::vector<double> vertex_array;
  std::vector<uint32_t> index_array;

  void initialize(int width, int height, std::vector<std::string> collections,
                  std::string crs);

  bool is_initialized(int width, int height,
                      std::vector<std::string> &collections, std::string &crs) {
    return (this->width == width && this->height == height &&
            this->collections == collections && this->crs == crs);
  };

  std::vector<uint8_t> render(double bbox[4]);

  MaskRenderer();
  ~MaskRenderer() {
    // 6. Terminate EGL when finished
    eglTerminate(display);
  };

 private:
  int width;
  int height;
  std::vector<std::string> collections;
  std::string crs;

  EGLDisplay display;
  EGLint major, minor;
  EGLint numConfigs;
  EGLConfig eglCfg;
  EGLSurface eglSurf;
  EGLContext eglCtx;
  GLuint points_vbo = 0;
  GLuint index_ibo = 0;

  std::vector<uint8_t> image;
};

MaskRenderer::MaskRenderer() {}

void MaskRenderer::initialize(int width, int height,
                              std::vector<std::string> collections,
                              std::string crs) {
  this->width = width;
  this->height = height;
  this->collections = collections;
  this->crs = crs;

  image.resize(width * height * 4);

  // 0. Prepare vertices
  load_opengl(collections, vertex_array, index_array, crs);

  // 1. Initialize EGL
  display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  eglInitialize(display, &major, &minor);

  // 2. Select an appropriate configuration
  eglChooseConfig(display, configAttribs, &eglCfg, 1, &numConfigs);

  // 3. Create a surface
  static const EGLint pbufferAttribs[] = {
      EGL_WIDTH, width, EGL_HEIGHT, height, EGL_NONE,
  };

  eglSurf = eglCreatePbufferSurface(display, eglCfg, pbufferAttribs);

  // 4. Bind the API
  eglBindAPI(EGL_OPENGL_API);

  // 5. Create a context and make it current
  eglCtx = eglCreateContext(display, eglCfg, EGL_NO_CONTEXT, NULL);

  eglMakeCurrent(display, eglSurf, eglSurf, eglCtx);

  // From now on use your OpenGL context
  GLenum err = glewInit();
  if (GLEW_OK != err && err != GLEW_ERROR_NO_GLX_DISPLAY) {
    /* Problem: glewInit failed, something is seriously wrong. */
    fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
  }
  fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

  // Transmit Data
  glGenBuffers(1, &points_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
  glBufferData(GL_ARRAY_BUFFER, vertex_array.size() * sizeof(double),
               &vertex_array[0], GL_STATIC_DRAW);
  glVertexPointer(2, GL_DOUBLE, 0, NULL);

  glGenBuffers(1, &index_ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_array.size() * sizeof(uint32_t),
               index_array.data(), GL_STATIC_DRAW);

  // Set up view
  glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
  glDisable(GL_TEXTURE_2D);  // textures
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glViewport(0, 0, width, height);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_INDEX_ARRAY);
  glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_ibo);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
}

std::vector<uint8_t> MaskRenderer::render(double bbox[4]) {
  glLoadIdentity();
  gluOrtho2D(bbox[0], bbox[2], bbox[1], bbox[3]);
  glClear(GL_COLOR_BUFFER_BIT);
  glDrawElements(GL_TRIANGLES, index_array.size(), GL_UNSIGNED_INT, NULL);
  glFinish();
  eglSwapBuffers(display, eglSurf);
  // Here we should read back our buffer
  memset(image.data(), 0, width * height * 4);
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image.data());

  return image;
}

static MaskRenderer mr;

std::vector<uint8_t> get_mask(int width, int height, double bbox[4],
                              std::string bbox_crs,
                              std::vector<std::string> collections,
                              std::string crs) {
  if (!mr.is_initialized(width, height, collections, crs)) {
    std::cout << "Initialize renderer" << std::endl;
    mr.initialize(width, height, collections, crs);
  }

  return mr.render(bbox);
}