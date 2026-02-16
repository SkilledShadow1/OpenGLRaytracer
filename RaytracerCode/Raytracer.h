#ifndef _RAYTRACER_H_
#define _RAYTRACER_H_

#include <stack>
using namespace std;

#include "sgraph/IScenegraph.h"
#include "sgraph/RaycastRenderer.h"
#include "Ray.h"
#include "Light.h"

class Raytracer
{
public:
    Raytracer(const string &filename)
    {
        this->filename = filename;
    }
    void raytrace(sgraph::IScenegraph *scenegraph, stack<glm::mat4> &modelview, vector<util::Light> &lights, float FOVY, int width, int height)
    {
        this->scenegraph = scenegraph;
        int *image = new int[3 * width * height];
        int i, j;
        Ray rayView;

        sgraph::RaycastRenderer *renderer = new sgraph::RaycastRenderer(modelview);

        for (i = 0; i < width; i++)
        {
            for (j = 0; j < height; j++)
            {
                /*
                create ray in view coordinates
                start point: 0,0,0 always!
                going through near plane pixel (i,j)
                So 3D location of that pixel in view coordinates is
                x = i-width/2
                y = j-height/2
                z = -0.5*height/tan(FOVY)
                */
                rayView.direction = glm::vec4(i - 0.5f * width,
                                              j - 0.5f * height,
                                              -0.5f * height / (float)tan(glm::radians(0.5 * FOVY)),
                                              0.0f);

                HitRecord hitR;
                glm::vec3 color;

                renderer->setRay(rayView);

                scenegraph->getRoot()->accept(renderer);

                hitR = renderer->getNearestIntersection();

                color = getRaytracedColor(hitR, lights, scenegraph, renderer);

                // Get the surface's reflection coefficient
                float surfaceReflection = hitR.material.getReflection();

                // Set the object's color be to be weaker depending on its reflectivity
                color *= (1.0f - surfaceReflection);

                // Reflection coefficient for the current ray being calculated
                float reflectionAmt = surfaceReflection;

                // Direction the starting reflecting ray is moving in
                glm::vec4 rayDir = glm::normalize(rayView.direction);

                int reflectionCount = 0;

                // Keep going unless there was no intersection, reflection coeff is too small or too many reflections
                while (reflectionAmt > 0.01f && reflectionCount < maxReflections && hitR.intersected())
                {
                    glm::vec4 reflectDir = glm::reflect(rayDir, hitR.normal);
                    Ray reflectedRay(hitR.point + offsetCoeff * hitR.normal, reflectDir);
                    HitRecord reflectedHit = createNewRay(reflectedRay, scenegraph, renderer);

                    // If the reflected ray didn't hit anything, don't change the color
                    if (!reflectedHit.intersected())
                        break;

                    // Get the color of what it hit
                    glm::vec3 reflectedColor = getRaytracedColor(reflectedHit, lights, scenegraph, renderer);

                    // Get the material's reflectivity
                    float hitReflection = reflectedHit.material.getReflection();

                    // Using the object's reflectivity and reflection amount, change the color
                    color += reflectionAmt * (1.0f - hitReflection) * reflectedColor;

                    // Multiply the reflection amount by the reflection coeff so the next reflection makes equal or less impact
                    reflectionAmt *= hitReflection;

                    // Move variables to next iteration
                    rayDir = reflectDir;
                    hitR = reflectedHit;
                    reflectionCount++;
                }

                // Clamp color in case it goes outide of the valid colors
                color = glm::clamp(color, glm::vec3(0.0f), glm::vec3(1.0f));

                int r, g, b;

                r = (int)(color.r * 255);
                g = (int)(color.g * 255);
                b = (int)(color.b * 255);

                image[3 * (j * width + i)] = r;
                image[3 * (j * width + i) + 1] = g;
                image[3 * (j * width + i) + 2] = b;
            }
        }

        PPMImageWriter writer;
        writer.save(image, width, height, filename);
    }

private:
    glm::vec3 getRaytracedColor(HitRecord &hitRecord, vector<util::Light> &lights,
                                sgraph::IScenegraph *scenegraph, sgraph::RaycastRenderer *renderer)
    {
        glm::vec3 color;
        if (hitRecord.intersected())
        {
            color = shade(hitRecord.point, hitRecord.normal, hitRecord.material, lights, scenegraph, renderer);
        }
        else
        {

            color = glm::vec3(0.0f, 0.0f, 0.0f);
        }
        return color;
    }

    glm::vec3 shade(glm::vec4 &point, glm::vec4 &normal, util::Material &material, vector<util::Light> &lights,
                    sgraph::IScenegraph *scenegraph, sgraph::RaycastRenderer *renderer)
    {
        glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);

        for (int i = 0; i < lights.size(); i++)
        {
            glm::vec3 lightVec;
            glm::vec3 spotdirection = glm::vec3(
                lights[i].getSpotDirection().x,
                lights[i].getSpotDirection().y,
                lights[i].getSpotDirection().z);

            if (spotdirection.length() > 0)
                spotdirection = glm::normalize(spotdirection);

            if (lights[i].getPosition().w != 0)
            {
                lightVec = glm::vec3(
                               lights[i].getPosition().x,
                               lights[i].getPosition().y,
                               lights[i].getPosition().z) -
                           glm::vec3(point.x, point.y, point.z);
            }
            else
            {
                lightVec = glm::vec3(
                    -lights[i].getPosition().x,
                    -lights[i].getPosition().y,
                    -lights[i].getPosition().z);
            }
            lightVec = glm::normalize(lightVec);

            /* if point is not in the light cone of this light, move on to next light */
            if (glm::dot(-lightVec, spotdirection) <= cos(glm::radians(lights[i].getSpotCutoff())))
                continue;

            glm::vec3 normalView = glm::normalize(glm::vec3(normal.x, normal.y, normal.z));

            float nDotL = glm::dot(normalView, lightVec);

            glm::vec3 viewVec = -glm::vec3(point.x, point.y, point.z);
            viewVec = glm::normalize(viewVec);

            glm::vec3 reflectVec = glm::reflect(-lightVec, normalView);
            reflectVec = glm::normalize(reflectVec);

            float rDotV = glm::max(glm::dot(reflectVec, viewVec), 0.0f);

            glm::vec3 ambient = glm::vec3(material.getAmbient()) * lights[i].getAmbient();

            glm::vec3 diffuse = glm::vec3(material.getDiffuse()) * lights[i].getDiffuse() * glm::max(nDotL, 0.0f);
            glm::vec3 specular;
            if (nDotL > 0)
            {
                specular = glm::vec3(material.getSpecular()) * lights[i].getSpecular() * glm::pow(rDotV, material.getShininess());
            }
            else
            {
                specular = glm::vec3(0.0f, 0.0f, 0.0f);
            }

            handleShadows(color, lightVec, point, normal, ambient, diffuse, specular, lights[i], scenegraph, renderer);
        }
        color = glm::clamp(color, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));

        return color;
    }

    // Helper function that records a hit given a ray
    HitRecord createNewRay(Ray &ray, sgraph::IScenegraph *scenegraph, sgraph::RaycastRenderer *renderer)
    {
        renderer->setRay(ray);
        scenegraph->getRoot()->accept(renderer);
        return renderer->getNearestIntersection();
    }

    // This function calculates shadows
    void handleShadows(glm::vec3 &color, glm::vec3 &lightVec, glm::vec4 &point, glm::vec4 &normal, glm::vec3 &ambient,
                       glm::vec3 &diffuse, glm::vec3 &specular, util::Light &light, sgraph::IScenegraph *scenegraph,
                       sgraph::RaycastRenderer *renderer)
    {
        bool isShadow = false;

        // Create a ray pointing in the direction from where it came from
        glm::vec4 reversedDir = glm::vec4(-lightVec.x, -lightVec.y, -lightVec.z, 0.0f);
        Ray reverseRay = Ray(point + offsetCoeff * normal, glm::vec4(lightVec, 0.0f));

        // Get the hit data, and if there is a hit, there is something blocking the ray --> it is a shadow
        HitRecord shadowHit = createNewRay(reverseRay, scenegraph, renderer);

        // Calculate the distance between the start and end of the original ray to see if there is an obstruction
        float lightDist = glm::length(glm::vec3(light.getPosition() - point));

        // Shadow if obstructed
        if (shadowHit.intersected() and shadowHit.time < lightDist)
            isShadow = true;

        if (isShadow)
        {
            // Only calculate ambient colors if it is a shadow
            color = color + ambient;
        }
        else
        {
            // Calculate color like normal if it isn't a shadow
            color = color + ambient + diffuse + specular;
        }
    }

private:
    string filename;

    // Slight offset used when drawing a ray so it doesn't collide with its start point
    const float offsetCoeff = 1e-4f;

    // Maximum number of reflections being used
    const int maxReflections = 30;
    sgraph::IScenegraph *scenegraph;
};

#endif