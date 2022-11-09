// TextureRenderTest.cpp
// -- simple test of 'TextureRender' class
// cs200 10/20
//
// Visual Studio command prompt:
//   cl /EHsc TextureRenderTest.cpp Affine.lib SquareMesh.cpp\
//      Texture.lib TextureRender.cpp opengl32.lib glew32.lib\
//      SLD2.lib SDL2main.lib /link /subsystem:console
//
// Linux command line:
//   g++ TextureRenderTest.cpp Affine.cpp SquareMesh.cpp
//       Texture.cpp TextureRender.cpp -lGL -lGLEW -lSDL2
//
// Note: the file 'texture.bmp' is assumed to exist.

#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cmath>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "TextureRender.h"
#include "SquareMesh.h"
#include "Affine.h"
#include "Texture.h"
using namespace std;


const glm::vec4 O = cs200::point(0,0),
                EX = cs200::vector(1,0),
                EY = cs200::vector(0,1),
                BLUISH(0.4f,0.3f,0.7f,1);


/////////////////////////////////////////////////////////////////
class TexturedSquare : public cs200::TexturedMesh {
  public:
    TexturedSquare(const glm::mat4 &M);
    int vertexCount(void) const              { return square.vertexCount(); }
    const glm::vec4* vertexArray(void) const { return square.vertexArray(); }
    glm::vec4 dimensions(void) const         { return square.dimensions(); }
    glm::vec4 center(void) const             { return square.center(); }
    int faceCount(void) const                { return square.faceCount(); }
    const Face* faceArray(void) const        { return square.faceArray(); }
    int edgeCount(void) const                { return square.edgeCount(); }
    const Edge* edgeArray(void) const        { return square.edgeArray(); }
    const glm::vec2* texcoordArray(void) const;
    const char *textureFileName(void) const { return "texture.bmp"; }
  private:
    static cs200::SquareMesh square;
    glm::vec2 texcoords[4];
};


cs200::SquareMesh TexturedSquare::square;


TexturedSquare::TexturedSquare(const glm::mat4 &M = cs200::scale(1)) {
  for (int i=0; i < 4; ++i)
    texcoords[i] = glm::vec2(M * vertexArray()[i]);
}


const glm::vec2* TexturedSquare::texcoordArray(void) const {
  return texcoords;
}


/////////////////////////////////////////////////////////////////
class Client {
  public:
    Client(void);
    ~Client(void);
    void draw(double dt);
    void keypress(SDL_Keycode kc);
    void resize(int W, int H);
  private:
    double time;
    bool rotating;
    float rate,
          rscale;
    glm::vec4 center;
    TexturedSquare sq_mesh1,
                   sq_mesh2;
    vector<glm::mat4> object_maps;
    cs200::TextureRender trender;
};


Client::Client(void) {

  // load bitmap file
  cs200::Bitmap *texture = 0;
  const char *file = sq_mesh1.textureFileName();
  try {
    texture = new cs200::Bitmap(file);
    cs200::reverseRGB(*texture);
  }
  catch (exception &e) {
    cout << "failed to load '" << file << "'" << endl;
    cout << e.what() << endl;
    texture = new cs200::Bitmap(1024,1024);
  }
  trender.loadTexture(texture->data(),texture->width(),texture->height());
  delete texture;

  // textured square mesh for static objects
  glm::mat4 Mt = cs200::translate(0.5f*EX+0.5f*EY)
                 * cs200::scale(0.5f);
  sq_mesh1 = TexturedSquare(Mt);

  // textured square mesh for rotating object
  Mt = cs200::rotate(45.0f)
       * cs200::scale(1.5f);
  sq_mesh2 = TexturedSquare(Mt);

  // rotating object parameters
  rate = -360.0f/8.0f;
  rscale = 1.25f/sqrt(2.0f);
  center = O;

  // modeling transforms for static objects
  const int COUNT = 12;
  for (int i=0; i < COUNT; ++i) {
    glm::mat4 Mo = cs200::rotate(360.0f*i/COUNT)
                   * cs200::translate(0.8f*EX)
                   * cs200::scale(0.15f);
    object_maps.push_back(Mo);
  }

  time = 0;
  rotating = true;
}


Client::~Client(void) {
  trender.unloadTexture();
}


void Client::draw(double dt) {
  trender.clearFrame(BLUISH);

  // render static objects
  trender.loadMesh(sq_mesh1);
  for (unsigned i=0; i < object_maps.size(); ++i) {
    trender.setTransform(object_maps[i]);
    trender.displayFaces();
  }
  trender.unloadMesh();

  // rotating object modeling transform
  glm::mat4 M = cs200::translate(center-O)
                * cs200::rotate(rate*time)
                * cs200::scale(0.5f*rscale);

  // render rotating object
  trender.loadMesh(sq_mesh2);
  trender.setTransform(M);
  trender.displayFaces();
  trender.unloadMesh();

  if (rotating)
    time += dt;
}


void Client::keypress(SDL_Keycode kc) {
  if (kc == SDLK_SPACE)
    rotating = !rotating;
}


void Client::resize(int W, int H) {
  int D = min(W,H);
  glViewport(0,0,D,D);
}


/////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

  // SDL: initialize and create a window
  SDL_Init(SDL_INIT_VIDEO);
  const char *title = "CS 200: TextureRender Test";
  int width = 600,
      height = 600;
  SDL_Window *window = SDL_CreateWindow(title,SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,width,height,
                                        SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
  SDL_GLContext context = SDL_GL_CreateContext(window);

  // GLEW: get function bindings (if possible)
  glewInit();
  if (!GLEW_VERSION_2_0) {
    cout << "needs OpenGL version 3.0 or better" << endl;
    return -1;
  }

  // animation loop
  try {
    bool done = false;
    Client *client = new Client();
    Uint32 ticks_last = SDL_GetTicks();
    while (!done) {
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        switch (event.type) {
          case SDL_QUIT:
            done = true;
            break;
          case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
              done = true;
            else
              client->keypress(event.key.keysym.sym);
            break;
          case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED)
              client->resize(event.window.data1,event.window.data2);
            break;
        }
      }
      Uint32 ticks = SDL_GetTicks();
      double dt = 0.001*(ticks - ticks_last);
      ticks_last = ticks;
      client->draw(dt);
      SDL_GL_SwapWindow(window);
    }

    delete client;
  }

  catch (exception &e) {
    cout << e.what() << endl;
  }

  SDL_GL_DeleteContext(context);
  SDL_Quit();
  return 0;
}

