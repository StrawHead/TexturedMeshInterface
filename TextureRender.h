// TextureRender.h
// -- rendering textured 2D meshes
// cs200 10/20

#ifndef CS200_TEXTURERENDER_H
#define CS200_TEXTURERENDER_H


#include <GL/glew.h>
#include <GL/gl.h>
#include "TexturedMesh.h"


namespace cs200 {

  class TextureRender {
    public:
      TextureRender(void);
      ~TextureRender(void);
      static void clearFrame(const glm::vec4 &c);
      void loadTexture(unsigned char *rgbdata, int width, int height);
      void unloadTexture(void);
      void setTransform(const glm::mat4 &M);
      void loadMesh(const TexturedMesh &m);
      void unloadMesh(void);
      void displayFaces(void);
    private:
      GLuint program,
             texture_buffer,
             vertex_buffer,
             texcoord_buffer,
             face_buffer,
             vao;
      GLint utransform;
      int mesh_face_count;
  };

}

#endif

