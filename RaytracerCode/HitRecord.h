#ifndef HITRECORD_H
#define HITRECORD_H

#include <glm/glm.hpp>
#include "Material.h"
#include <limits>
#include <string>
using namespace std;

class HitRecord
{
public:
  float time;
  glm::vec4 point, normal;
  util::Material material;

  HitRecord()
  {
    time = numeric_limits<float>::max();
    point = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    normal = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
  }

  bool intersected()
  {
    return time < numeric_limits<float>::max();
  }
};
#endif // HITRECORD_H
