#ifndef _RAYCASTRENDERER_H_
#define _RAYCASTRENDERER_H_

#include "SGNodeVisitor.h"
#include "GroupNode.h"
#include "LeafNode.h"
#include "TransformNode.h"
#include "RotateTransform.h"
#include "ScaleTransform.h"
#include "TranslateTransform.h"
#include "../HitRecord.h"
#include "../Ray.h"
#include <stack>
#include <iostream>
using namespace std;

namespace sgraph {
    /**
     * This visitor implements casting a single ray into the scene graph
     * 
     */
    class RaycastRenderer: public SGNodeVisitor {
        public:
        /**
         * @brief Construct a new GLScenegraphRenderer object
         * 
         * @param mv a reference to modelview stack that will be used while rendering
         */
        RaycastRenderer(stack<glm::mat4>& mv) 
            : modelview(mv) {
        }

        void setRay(Ray& viewRay) {
            this->viewRay = viewRay;
            hitRecords.clear();
        }

        HitRecord getNearestIntersection() {
            HitRecord minH;
            for (int i=0;i<hitRecords.size();i+=1) {
                if ((!minH.intersected()) || ((hitRecords[i].time>0) && (hitRecords[i].time<minH.time))) {
                    minH = hitRecords[i];
                }
            }
            return minH;
        }

        /**
         * @brief Recur to the children for drawing
         * 
         * @param groupNode 
         */
        void visitGroupNode(GroupNode *groupNode) {
            for (int i=0;i<groupNode->getChildren().size();i=i+1) {
                groupNode->getChildren()[i]->accept(this);
            }
        }

        /**
         * @brief Draw the instance for the leaf, after passing the 
         * modelview and color to the shader
         * 
         * @param leafNode 
         */
        void visitLeafNode(LeafNode *leafNode) {

            Ray objectRay;
            HitRecord hitRecord;

            glm::mat4 leafToView(modelview.top());
            glm::mat4 viewToLeaf = glm::inverse(leafToView);
            objectRay.start = glm::vec4(viewRay.start);
            objectRay.direction = glm::vec4(viewRay.direction);

            objectRay.start = viewToLeaf * objectRay.start;
            objectRay.direction = viewToLeaf * objectRay.direction;

            if (leafNode->getInstanceOf().compare("sphere") == 0) {
                float a, b, c;

                a = glm::length(objectRay.direction) *
                    glm::length(objectRay.direction);
                b = 2 * glm::dot(objectRay.start, objectRay.direction);
                // the extra -1 because this is a vec4 with 1 in the w, so
                // length is one more than what we want
                c = glm::length(objectRay.start) * glm::length(objectRay.start) -
                    1 - 1;

                if ((b * b - 4 * a * c) >= 0) {
                    float t1 = (-b + (float)sqrt(b * b - 4 * a * c)) / (2 * a);
                    float t2 = (-b - (float)sqrt(b * b - 4 * a * c)) / (2 * a);

                    float t;
                    if (t1 >= 0) {
                        if (t2 >= 0) {
                            t = glm::min(t1, t2);
                        }
                        else {
                            t = t1;
                        }
                    }
                    else {
                        if (t2 >= 0)
                            t = t2;
                        else
                            return;
                    }

                    if (t < hitRecord.time) {
                        hitRecord.time = t;
                        hitRecord.point = viewRay.start + viewRay.direction * t;
                        hitRecord.normal =
                            objectRay.start + objectRay.direction * t;
                        hitRecord.normal.w = 0;
                        
                        hitRecord.normal =
                            glm::transpose(viewToLeaf) * hitRecord.normal;
                        hitRecord.normal =
                            glm::vec4(glm::normalize(glm::vec3(hitRecord.normal.x,
                                                            hitRecord.normal.y,
                                                            hitRecord.normal.z)),
                                    0);

                        hitRecord.material = leafNode->getMaterial();
                    }
                }
            }
            else if (leafNode->getInstanceOf().compare("box") == 0) {
                float tmaxX, tmaxY, tmaxZ;
                float tminX, tminY, tminZ;

                if (fabs(objectRay.direction.x) < 0.0001f) {
                    if ((objectRay.start.x > 0.5f) || (objectRay.start.x < -0.5f))
                        return;
                    else {
                        tminX = numeric_limits<float>::lowest();
                        tmaxX = numeric_limits<float>::max();
                    }
                }
                else {
                    float t1 = (-0.5f - objectRay.start.x) / objectRay.direction.x;
                    float t2 = (0.5f - objectRay.start.x) / objectRay.direction.x;
                    tminX = std::min(t1, t2);
                    tmaxX = std::max(t1, t2);
                }

                if (fabs(objectRay.direction.y) < 0.0001f) {
                    if ((objectRay.start.y > 0.5f) || (objectRay.start.y < -0.5f)) {
                        return;
                    }
                    else {
                        tminY = numeric_limits<float>::lowest();
                        tmaxY = numeric_limits<float>::max();
                    }
                }
                else {
                    float t1 = (-0.5f - objectRay.start.y) / objectRay.direction.y;
                    float t2 = (0.5f - objectRay.start.y) / objectRay.direction.y;
                    tminY = std::min(t1, t2);
                    tmaxY = std::max(t1, t2);
                }

                if (fabs(objectRay.direction.z) < 0.0001f) {
                    if ((objectRay.start.z > 0.5f) || (objectRay.start.z < -0.5f)) {
                        return;
                    }
                    else {
                        tminZ = numeric_limits<float>::lowest();
                        tmaxZ = numeric_limits<float>::max();
                    }
                }
                else {
                    float t1 = (-0.5f - objectRay.start.z) / objectRay.direction.z;
                    float t2 = (0.5f - objectRay.start.z) / objectRay.direction.z;
                    tminZ = std::min(t1, t2);
                    tmaxZ = std::max(t1, t2);
                }

                float tmin, tmax;

                tmin = std::max<float>(tminX, std::max<float>(tminY, tminZ));
                tmax = std::min<float>(tmaxX, std::min<float>(tmaxY, tmaxZ));

                if ((tmin < tmax) && (tmax > 0)) {
                    float t;
                    if (tmin > 0)
                        t = tmin;
                    else
                        t = tmax;

                    if (t < hitRecord.time) {
                        hitRecord.time = t;

                        hitRecord.point = viewRay.start + viewRay.direction * t;

                        glm::vec4 pointInLeaf =
                            objectRay.start + objectRay.direction * t;

                        if (fabs(pointInLeaf.x - 0.5f) < 0.001) {
                            hitRecord.normal.x = 1;
                        }
                        else if (fabs(pointInLeaf.x + 0.5f) < 0.001) {
                            hitRecord.normal.x = -1;
                        }
                        else
                            hitRecord.normal.x = 0;

                        if (fabs(pointInLeaf.y - 0.5f) < 0.001) {
                            hitRecord.normal.y = 1;
                        }
                        else if (fabs(pointInLeaf.y + 0.5f) < 0.001) {
                            hitRecord.normal.y = -1;
                        }
                        else
                            hitRecord.normal.y = 0;

                        if (fabs(pointInLeaf.z - 0.5f) < 0.001) {
                            hitRecord.normal.z = 1;
                        }
                        else if (fabs(pointInLeaf.z + 0.5f) < 0.001) {
                            hitRecord.normal.z = -1;
                        }
                        else
                            hitRecord.normal.z = 0;

                        hitRecord.normal.w = 0;

                        hitRecord.normal =
                            glm::vec4(glm::normalize(glm::vec3(hitRecord.normal.x,
                                                            hitRecord.normal.y,
                                                            hitRecord.normal.z)),
                                    0);

                        hitRecord.normal =
                            glm::transpose(viewToLeaf) * hitRecord.normal;
                        hitRecord.normal =
                            glm::vec4(glm::normalize(glm::vec3(hitRecord.normal.x,
                                                            hitRecord.normal.y,
                                                            hitRecord.normal.z)),
                                    0);

                        hitRecord.material = leafNode->getMaterial();
                    }
                }
            }

            if (hitRecord.intersected()) {
                hitRecords.push_back(hitRecord);
            }
            
        }

        /**
         * @brief Multiply the transform to the modelview and recur to child
         * 
         * @param transformNode 
         */
        void visitTransformNode(TransformNode * transformNode) {
            modelview.push(modelview.top());
            modelview.top() = modelview.top() * transformNode->getTransform();
            if (transformNode->getChildren().size()>0) {
                transformNode->getChildren()[0]->accept(this);
            }
            modelview.pop();
        }

        /**
         * @brief For this visitor, only the transformation matrix is required.
         * Thus there is nothing special to be done for each type of transformation.
         * We delegate to visitTransformNode above
         * 
         * @param scaleNode 
         */
        void visitScaleTransform(ScaleTransform *scaleNode) {
            visitTransformNode(scaleNode);
        }

        /**
         * @brief For this visitor, only the transformation matrix is required.
         * Thus there is nothing special to be done for each type of transformation.
         * We delegate to visitTransformNode above
         * 
         * @param translateNode 
         */
        void visitTranslateTransform(TranslateTransform *translateNode) {
            visitTransformNode(translateNode);
        }

        void visitRotateTransform(RotateTransform *rotateNode) {
            visitTransformNode(rotateNode);
        }

        private:
        Ray viewRay;
        stack<glm::mat4>& modelview;    
        vector<HitRecord> hitRecords;

   };
}


#endif