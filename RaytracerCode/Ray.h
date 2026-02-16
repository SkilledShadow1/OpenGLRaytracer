#ifndef __RAY_H__
#define __RAY_H__

#include <glm/glm.hpp>

class Ray
{
public:
  glm::vec4 start, direction;

  Ray()
  {
    start = glm::vec4(0.0f, 0.0f, 0.0f, 1);
    direction = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
  }

  Ray(glm::vec4 start, glm::vec4 direction)
  {
    this->start = start;
    this->direction = direction;
  }
};

#endif // RAY_H
