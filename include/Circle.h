#ifndef __CIRCLE_H__
#define __CIRCLE_H__

#include "VertexAttrib.h"
#include <PolygonMesh.h>
#include <GLFW/glfw3.h>

const glm::vec4 colorRed = glm::vec4(1, 0, 0, 1);

class Circle : public util::PolygonMesh<VertexAttribWithColor>
{
public:
    Circle(int cx, int cy, int radius, glm::vec4 color);
    Circle(int cx, int cy, int radius);
    ~Circle() {}
};

Circle::Circle(int originX, int originY, int radius, glm::vec4 color)
{
    // create the vertex data
    vector<glm::vec4> positions;

    const int SLICES = 100;
    const float PI = 3.14159;
    const int renderType = GL_LINE_LOOP;
    const int polygonSize = 3;

    for (int i = 0; i < SLICES; ++i)
    {
        float theta = (float)i / (SLICES - 1) * 2 * PI;
        float x = originX + radius * cos(theta);
        float y = originY + radius * sin(theta);

        positions.push_back(glm::vec4(x, y, 0, 1));
    }

    vector<VertexAttrib> vertexData;
    for (int i = 0; i < positions.size(); i++)
    {
        vector<float> data;
        VertexAttrib v;
        for (int j = 0; j < 4; j++)
        {
            data.push_back(positions[i][j]);
        }
        v.setData("position", data);
        vertexData.push_back(v);
    }

    // create the indices
    vector<unsigned int> indices;
    for (int i = 0; i < positions.size(); ++i)
    {
        indices.push_back(i);
    }

    this->setVertexData(vertexData);
    // give it the index data that forms the polygons
    this->setPrimitives(indices);

    this->setPrimitiveType(renderType);
    this->setPrimitiveSize(polygonSize); // 3 vertices per polygon
}

Circle::Circle(int originX, int originY, int radius) : Circle::Circle(originX, originY, radius, (colorRed)) {}

#endif