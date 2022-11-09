// TexturedMesh.h
// -- triangular mesh with texture coordinates attached to vertices
// cs200 10/20

#ifndef CS200_TEXTUREDMESH_H
#define CS200_TEXTUREDMESH_H


#include "Mesh.h"


namespace cs200 {

  struct TexturedMesh : Mesh {
    virtual const glm::vec2* texcoordArray(void) const = 0;
    virtual const char *textureFileName(void) const = 0;
  };

}


#endif

