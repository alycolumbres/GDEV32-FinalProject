#include <glm/glm.hpp>

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace std;

struct Ray
{
    glm::vec3 origin; // Ray origin
    glm::vec3 direction; // Ray direction
};

struct Material
{
    glm::vec3 ambient; // Ambient
    glm::vec3 diffuse; // Diffuse
    glm::vec3 specular; // Specular
    float shininess; // Shininess
};

struct SceneObject
{
    Material material; // Material

    /**
     * Template function for calculating the intersection of this object with the provided ray.
     * @param[in]   incomingRay             Ray that will be checked for intersection with this object
     * @param[out]  outIntersectionPoint    Point of intersection (in case there is an intersection)
     * @param[out]  outIntersectionNormal   Normal vector at the point of intersection (in case there is an intersection)
     * @return If there is an intersection, returns the distance from the ray origin to the intersection point. Otherwise, returns a negative number.
     */
    virtual float Intersect(const Ray& incomingRay, glm::vec3& outIntersectionPoint, glm::vec3& outIntersectionNormal) = 0;
};

// Subclass of SceneObject representing a Sphere scene object
struct Sphere : public SceneObject
{
    glm::vec3 center; // center
    float radius; // radius

    /**
     * @brief Ray-sphere intersection
     * @param[in]   incomingRay             Ray that will be checked for intersection with this object
     * @param[out]  outIntersectionPoint    Point of intersection (in case there is an intersection)
     * @param[out]  outIntersectionNormal   Normal vector at the point of intersection (in case there is an intersection)
     * @return If there is an intersection, returns the distance from the ray origin to the intersection point. Otherwise, returns a negative number.
     */
    virtual float Intersect(const Ray& incomingRay, glm::vec3& outIntersectionPoint, glm::vec3& outIntersectionNormal)
    {
        // In case there is an intersection, place the intersection point and intersection normal
        // that you calculated to the outIntersectionPoint and outIntersectionNormal variables.
        //
        // When you use this function from the outside, you can pass in the variables by reference.
        //
        // Example:
        // Ray ray = ...;
        // glm::vec3 point, normal;
        // float t = sphere->Intersect(ray, point, normal);
        //
        // (At this point, point and normal will now contain the intersection point and intersection normal)
        
        // Step 5

        glm::vec3 P = incomingRay.origin;
        glm::vec3 m = P - center;
        glm::vec3 d = incomingRay.direction;
        float b = glm::dot(m, d);
        float c = glm::dot(m, m) - radius * radius;
        
        float checkIntersection = b * b - c;
        float t1 = -b + sqrt(checkIntersection);
        float t2 = -b - sqrt(checkIntersection);

        if (checkIntersection < 0) // no intersection
        {
            return -1;
        }
        else if (checkIntersection == 0) // only one intersection point
        {
            outIntersectionPoint = P + t1 * d;
            outIntersectionNormal = glm::normalize(outIntersectionPoint - center);
            return t1;
        }
        else
        {
            if (t1 < 0 && t2 < 0) // no intersection
            {
                return -1;
            }
            else if ((t1 < 0 && t2 > 0) || (t1 > 0 && t2 < 0)) // ray starts inside sphere
            {
                if (t1 > 0)
                {
                    outIntersectionPoint = P + t1 * d;
                    outIntersectionNormal = glm::normalize(outIntersectionPoint - center);
                    return t1;
                }
                else
                {
                    outIntersectionPoint = P + t2 * d;
                    outIntersectionNormal = glm::normalize(outIntersectionPoint - center);
                    return t2;
                }
            }
            else
            {
                if (t1 > t2)
                {
                    outIntersectionPoint = P + t2 * d;
                    outIntersectionNormal = glm::normalize(outIntersectionPoint - center);
                    return t2;
                }
                else
                {
                    outIntersectionPoint = P + t1 * d;
                    outIntersectionNormal = glm::normalize(outIntersectionPoint - center);
                    return t1;
                }
            }
        }
    }
};

// Subclass of SceneObject representing a Triangle scene object
struct Triangle : public SceneObject
{
    glm::vec3 A; // First point
    glm::vec3 B; // Second point
    glm::vec3 C; // Third point

    /**
     * @brief Ray-Triangle intersection
     * @param[in]   incomingRay             Ray that will be checked for intersection with this object
     * @param[out]  outIntersectionPoint    Point of intersection (in case there is an intersection)
     * @param[out]  outIntersectionNormal   Normal vector at the point of intersection (in case there is an intersection)
     * @return If there is an intersection, returns the distance from the ray origin to the intersection point. Otherwise, returns a negative number.
     */
    virtual float Intersect(const Ray& incomingRay, glm::vec3& outIntersectionPoint, glm::vec3& outIntersectionNormal)
    {
        // The same idea for the outIntersectionPoint and outIntersectionNormal applies here
        
        // Step 9

        glm::vec3 P = incomingRay.origin;
        glm::vec3 n = glm::cross((B - A), (C - A));
        glm::vec3 e = glm::cross(-incomingRay.direction, (P - A));
        float f = glm::dot(-incomingRay.direction, n);

        float t = (glm::dot((P - A), n)) / f;
        float u = (glm::dot((C - A), e)) / f;
        float v = (glm::dot(-(B - A), e)) / f;

        if (t <= 0) // no intersection
        {
            return -1; 
        }
        else
        {
            if (u + v <= 1 && u > 0 && v > 0 && f > 0) // if u and v are inside the triangle
            {
                //outIntersectionPoint = A + u * (B - A) + v * (C - A);
                outIntersectionPoint = P + t * incomingRay.direction;
                
                //glm::vec3 AB = glm::normalize(B - A);
                //glm::vec3 AC = glm::normalize(C - A);
                outIntersectionNormal = glm::normalize( glm::cross(B - A, C - A) );
                return t;
            }
            else
            {
                return -1;
            }
        }
    }
};

struct Camera
{
    glm::vec3 position; // Position
    glm::vec3 lookTarget; // Look target
    glm::vec3 globalUp; // Global up-vector
    float fovY; // Vertical field of view
    float focalLength; // Focal length

    float imageWidth; // image width
    float imageHeight; // image height
};

struct Light
{
    glm::vec4 position; // Light position (w = 1 if point light, w = 0 if directional light)

    glm::vec3 ambient; // Light's ambient intensity
    glm::vec3 diffuse; // Light's diffuse intensity
    glm::vec3 specular; // Light's specular intensity

    // --- Attenuation variables ---
    float constant; // Constant factor
    float linear; // Linear factor
    float quadratic; // Quadratic factor
};

struct IntersectionInfo
{
    Ray incomingRay; // Ray used to calculate the intersection
    float t; // Distance from the ray's origin to the point of intersection (if there was an intersection).
    SceneObject* obj; // Object that the ray intersected with. If this is equal to nullptr, then no intersection occured.
    glm::vec3 intersectionPoint; // Point where the intersection occured (if there was an intersection)
    glm::vec3 intersectionNormal; // Normal vector at the point of intersection (if there was an intersection)
};

struct Scene
{
    std::vector<SceneObject*> objects; // List of all objects in the scene
    std::vector<Light> lights; // List of all lights in the scene
};

struct Image
{
    std::vector<unsigned char> data; // Image data
    int width; // Image width
    int height; // Image height

    /**
     * @brief Constructor
     * @param[in] w Width
     * @param[in] h Height
     */
    Image(const int& w, const int& h)
        : width(w)
        , height(h)
    {
        data.resize(w * h * 3, 0);
    }

    /**
     * @brief Converts the provided color value from [0, 1] to [0, 255]
     * @param[in] c Color value in [0, 1] range
     * @return Color value in [0, 255] range
     */
    unsigned char ToChar(float c)
    {
        c = glm::clamp(c, 0.0f, 1.0f);
        return static_cast<unsigned char>(c * 255);
    }

    /**
     * @brief Sets the color at the specified pixel location
     * @param[in] x     X-coordinate of the pixel
     * @param[in] y     Y-coordinate of the pixel
     * @param[in] color Pixel color
     */
    void SetColor(const int& x, const int& y, const glm::vec3& color)
    {
        int index = (y * width + x) * 3;
        data[index] = ToChar(color.r);
        data[index + 1] = ToChar(color.g);
        data[index + 2] = ToChar(color.b);
    }
};

/**
 * @brief Gets the ray that goes from the camera's position to the specified pixel at (x, y)
 * @param[in] camera Camera data
 * @param[in] x X-coordinate of the pixel (upper-left corner of the pixel)
 * @param[in] y Y-coordinate of the pixel (upper-left corner of the pixel)
 * @return Ray that passes through the pixel at (x, y)
 */

Ray GetRayThruPixel(const Camera &camera, const int& pixelX, const int& pixelY)
{
    // Step 4
    
    Ray ray;
    ray.origin = camera.position; 

    float aspect = camera.imageWidth / camera.imageHeight;
    float viewportHeight = 2 * camera.focalLength * glm::tan(camera.fovY / 2);
    float viewportWidth = aspect * viewportHeight;

    glm::vec3 lookDirection = glm::normalize(camera.lookTarget - ray.origin);
    glm::vec3 upVector = camera.globalUp;

    glm::vec3 lcrossU = glm::cross(lookDirection, upVector);
    glm::vec3 u = glm::normalize(lcrossU);

    glm::vec3 ucrossl = glm::cross(u, lookDirection);
    glm::vec3 v = glm::normalize(ucrossl);

    glm::vec3 L = camera.position + lookDirection * camera.focalLength - u * (viewportWidth / 2) - v * (viewportHeight / 2);

    float pixelSizeW = viewportWidth / camera.imageWidth;
    float pixelSizeH = viewportHeight / camera.imageHeight;

    float s = ((pixelX + pixelSizeW) / camera.imageWidth) * viewportWidth;
    float t = ((pixelY + pixelSizeH) / camera.imageHeight) * viewportHeight;
    glm::vec3 P = L + u * s + v * t;
    glm::vec3 rayDirection = glm::normalize(P - ray.origin);

    ray.direction = rayDirection;

    return ray;
}

/**
 * @brief Cast a ray to the scene.
 * @param[in] ray   Ray to cast to the scene
 * @param[in] scene Scene object
 * @return Returns an IntersectionInfo object that will contain the results of the raycast
 */
IntersectionInfo Raycast(const Ray& ray, const Scene &scene)
{
    // Step 6

    /*
    // Fields that need to be populated:
    ret.intersectionPoint = glm::vec3(0.0f); // Intersection point
    ret.intersectionNormal = glm::vec3(0.0f); // Intersection normal
    ret.t = 0.0f; // Distance from ray origin to intersection point
    ret.obj = nullptr; // First object hit by the ray. Set to nullptr if the ray does not hit anything
    */

    std::vector<IntersectionInfo> intersections;
    IntersectionInfo ret;

    for (int i = 0; i < scene.objects.size(); i++)
    {
        IntersectionInfo info;
        info.incomingRay = ray;
        float intersection = scene.objects[i]->Intersect(info.incomingRay, info.intersectionPoint, info.intersectionNormal);

        if (intersection < 0)
        {
            info.obj = nullptr;
        }
        else
        {
            info.obj = scene.objects[i];
        }

        info.t = intersection;
        intersections.push_back(info);
    }

    // checks if all objects are nullptr
    bool noIntersection = true;
    for (int i = 0; i < intersections.size(); i++)
    {
        if (intersections[i].obj != nullptr)
        {
            noIntersection = false;
            break;
        }
    }

    float shortestDistance = intersections[0].t;
    int index = 0;
    if (noIntersection == true)
    {
        ret.obj = nullptr;
        return ret;
    }
    else // find IntersectionInfo with the smallest positive t
    {
        for (int i = 0; i < intersections.size(); i++)
        {
            if (shortestDistance < 0 && intersections[i].t > 0)
            {
                shortestDistance = intersections[i].t;
                index = i;
            }
            else if (shortestDistance > intersections[i].t && intersections[i].t > 0)
            {
                shortestDistance = intersections[i].t;
                index = i;
            }
        }
    }

    return intersections[index];
}


/**
 * @brief Perform a ray-trace to the scene
 * @param[in] ray       Ray to trace
 * @param[in] scene     Scene data
 * @param[in] camera    Camera data
 * @param[in] maxDepth  Maximum depth of the trace
 * @return Resulting color after the ray bounced around the scene
 */
glm::vec3 RayTrace(const Ray& ray, const Scene& scene, const Camera& camera, int maxDepth = 1)
{
    IntersectionInfo ret;
    ret.incomingRay = ray;

    vector<glm::vec3> ambientValues;
    vector<glm::vec3> diffuseValues;
    vector<glm::vec3> specularValues;
    
    Scene currentScene = scene;
    
    glm::vec3 color = glm::vec3(0.33f, 0.6f, 0.75f);
  
    glm::vec3 finalAmbient = glm::vec3(0.0);
    glm::vec3 finalDiffuse = glm::vec3(0.0);
    glm::vec3 finalSpecular = glm::vec3(0.0);
    glm::vec3 finalColor(0.0f, 0.0f, 0.0f);
    
    // Step 7
    
    ret = Raycast( ret.incomingRay, currentScene );
    
    if( ret.obj == nullptr )
    {
        return color;
    }
    else
    {
        // Steps 13, 15
        
        for (int i = 0; i < scene.lights.size(); i++)
        {
            if (scene.lights[i].position.w == 1) // if point light
            {
                Ray shadowRay;
                shadowRay.origin = ret.intersectionPoint + 0.001f * ret.intersectionNormal;
                shadowRay.direction = glm::normalize(glm::vec3(scene.lights[i].position) - shadowRay.origin);
                IntersectionInfo shadowInfo;
                shadowInfo = Raycast(shadowRay, currentScene);

                // Step 14 : Attenuation
                float pointDistance = glm::length(glm::vec3(scene.lights[i].position) - ret.intersectionPoint);
                float attenuation = 1.0f / (scene.lights[i].constant + (scene.lights[i].linear * pointDistance) + scene.lights[i].quadratic * (pointDistance * pointDistance));

                if (shadowInfo.obj == nullptr || pointDistance < shadowInfo.t)
                {
                    // Ambient
                    glm::vec3 ambient = ret.obj->material.ambient * scene.lights[i].ambient * attenuation;
                    ambientValues.push_back(ambient);

                    // Diffuse
                    glm::vec3 lightDir = glm::normalize(glm::vec3(scene.lights[i].position) - ret.intersectionPoint);
                    float d = max(glm::dot(ret.intersectionNormal, lightDir), 0.0f);
                    glm::vec3 diffuse = d * (ret.obj->material.diffuse * scene.lights[i].diffuse) * attenuation;
                    diffuseValues.push_back(diffuse);

                    // Specular
                    glm::vec3 e = glm::normalize(camera.position - ret.intersectionPoint);
                    glm::vec3 reflect = glm::reflect(lightDir, ret.intersectionNormal);
                    float s = pow(max(glm::dot(reflect, e), 0.0f), ret.obj->material.shininess);
                    glm::vec3 specular = s * (ret.obj->material.specular * scene.lights[i].specular) * attenuation;
                    specularValues.push_back(specular);
                    
                    // Step 16: Reflection
                    if (maxDepth >= 0)
                    {
                        Ray reflectionRay;
                        reflectionRay.origin = ret.intersectionPoint + 0.001f * ret.intersectionNormal;
                        reflectionRay.direction = glm::reflect(ret.incomingRay.direction, ret.intersectionNormal);
                        float Kr = ret.obj->material.shininess / 128.0f;
                        finalColor += Kr * RayTrace(reflectionRay, currentScene, camera, maxDepth - 1);
                    }
                }
                else
                {
                    // Ambient
                    glm::vec3 ambient = ret.obj->material.ambient * scene.lights[i].ambient * attenuation;
                    ambientValues.push_back(ambient);
                }
                
            }
            else if (scene.lights[i].position.w == 0) // if directional light
            {
                Ray shadowRay;
                shadowRay.origin = ret.intersectionPoint + 0.0001f * ret.intersectionNormal;
                shadowRay.direction = glm::normalize(-glm::vec3(scene.lights[i].position));
                IntersectionInfo shadowInfo;
                shadowInfo = Raycast(shadowRay, currentScene);
                
                if (shadowInfo.obj == nullptr)
                {
                    // Ambient
                    glm::vec3 ambient = ret.obj->material.ambient * scene.lights[i].ambient;
                    ambientValues.push_back(ambient);

                    // Diffuse
                    glm::vec3 lightDir = glm::normalize(glm::vec3(scene.lights[i].position));
                    float d = max(glm::dot(ret.intersectionNormal, -lightDir), 0.0f);
                    glm::vec3 diffuse = d * (ret.obj->material.diffuse * scene.lights[i].diffuse);
                    diffuseValues.push_back(diffuse);

                    // Specular
                    glm::vec3 e = glm::normalize(camera.position - ret.intersectionPoint);
                    glm::vec3 reflect = glm::reflect(lightDir, ret.intersectionNormal);
                    float s = pow(max(glm::dot(reflect, e), 0.0f), ret.obj->material.shininess);
                    glm::vec3 specular = s * (ret.obj->material.specular * scene.lights[i].specular);
                    specularValues.push_back(specular);
                    
                    // Step 16: Reflection
                    if (maxDepth >= 0)
                    {
                        Ray reflectionRay;
                        reflectionRay.origin = ret.intersectionPoint + 0.001f * ret.intersectionNormal;
                        reflectionRay.direction = glm::reflect(ret.incomingRay.direction, ret.intersectionNormal);
                        float Kr = ret.obj->material.shininess / 128.0f;
                        finalColor += Kr * RayTrace(reflectionRay, currentScene, camera, maxDepth - 1);
                    }
                    
                }
                else
                {
                    // Ambient
                    glm::vec3 ambient = ret.obj->material.ambient * scene.lights[i].ambient;
                    ambientValues.push_back(ambient);
                }
            }
            
            for (glm::vec3 ambient : ambientValues)
            {
                finalAmbient += ambient;
            }

            float ambientLights = ambientValues.size();
            finalAmbient = finalAmbient / ambientLights;

            for (glm::vec3 diffuse : diffuseValues)
            {
                finalDiffuse += diffuse;
            }

            for (glm::vec3 specular : specularValues)
            {
                finalSpecular += specular;
            }

            finalColor += finalAmbient + finalDiffuse + finalSpecular;
            
        }
        
        return finalColor;
    }
}

/**
 * Main function
 */
int main()
{
    Scene scene;
    int maxDepth = 5;

    Camera camera = { glm::vec3(-5.0f, 1.0f, 5.0f),  // position
                      glm::vec3(0.0f, 1.0f, 0.0f),  // look target
                      glm::vec3(0.0f, 1.0f, 0.0f),  // global up
                      glm::radians(60.0f), 1.0f,                  // fov and focal length
                      640, 480 };                   // image width and height;
    
    Light light = { glm::vec4(0.0f, -1.0f, -1.0f, 0.0f), // position
                    glm::vec3(0.2f, 0.2f, 0.2f),         // ambient
                    glm::vec3(1.0f, 1.0f, 1.0f),         // diffuse
                    glm::vec3(1.0f, 1.0f, 1.0f),         // specular
                    1, 0, 0 };                           // constant, linear, quadratic

    scene.lights.push_back(light);

    // FLOOR

    Triangle *floorT1 = new Triangle();
    floorT1->A = glm::vec3(-10, 0, -10);
    floorT1->B = glm::vec3(-10, 0, 10);
    floorT1->C = glm::vec3(10, 0, -10);
    floorT1->material.ambient = glm::vec3(0.0f, 0.05f, 0.05f);
    floorT1->material.diffuse = glm::vec3(0.24f, 0.76f, 0.0f);
    floorT1->material.specular = glm::vec3(0.04f, 0.7f, 0.7f);
    floorT1->material.shininess = 1;
    scene.objects.push_back(floorT1);

    Triangle* floorT2 = new Triangle();
    floorT2->A = glm::vec3(-10, 0, 10);
    floorT2->B = glm::vec3(10, 0, 10);
    floorT2->C = glm::vec3(10, 0, -10);
    floorT2->material.ambient = glm::vec3(0.0f, 0.05f, 0.05f);
    floorT2->material.diffuse = glm::vec3(0.24f, 0.76f, 0.0f);
    floorT2->material.specular = glm::vec3(0.04f, 0.7f, 0.7f);
    floorT2->material.shininess = 1;
    scene.objects.push_back(floorT2);

    // ROOF 1

    Triangle* roofFront = new Triangle();
    roofFront->A = glm::vec3(-3, 1, 1);
    roofFront->B = glm::vec3(-2, 1, 1);
    roofFront->C = glm::vec3(-2.5, 1.5, 0.5);
    roofFront->material.ambient = glm::vec3(0.2125f, 0.1275f, 0.054f);
    roofFront->material.diffuse = glm::vec3(0.9, 0.22, 0.27);
    roofFront->material.specular = glm::vec3(0.393548f, 0.271906f, 0.166721f);
    roofFront->material.shininess = 0.5;
    scene.objects.push_back(roofFront);

    Triangle* roofLeft = new Triangle();
    roofLeft->A = glm::vec3(-3, 1, 0);
    roofLeft->B = glm::vec3(-3, 1, 1);
    roofLeft->C = glm::vec3(-2.5, 1.5, 0.5);
    roofLeft->material.ambient = glm::vec3(0.2125f, 0.1275f, 0.054f);
    roofLeft->material.diffuse = glm::vec3(0.9, 0.22, 0.27);
    roofLeft->material.specular = glm::vec3(0.393548f, 0.271906f, 0.166721f);
    roofLeft->material.shininess = 0.5;
    scene.objects.push_back(roofLeft);

    Triangle* roofRight = new Triangle();
    roofRight->A = glm::vec3(-2, 1, 1);
    roofRight->B = glm::vec3(-2, 1, 0);
    roofRight->C = glm::vec3(-2.5, 1.5, 0.5);
    roofRight->material.ambient = glm::vec3(0.2125f, 0.1275f, 0.054f);
    roofRight->material.diffuse = glm::vec3(0.9, 0.22, 0.27);
    roofRight->material.specular = glm::vec3(0.393548f, 0.271906f, 0.166721f);
    roofRight->material.shininess = 0.5;
    scene.objects.push_back(roofRight);

    Triangle* roofBack = new Triangle();
    roofBack->A = glm::vec3(-2, 1, 0);
    roofBack->B = glm::vec3(-3, 1, 0);
    roofBack->C = glm::vec3(-2.5, 1.5, 0.5);
    roofBack->material.ambient = glm::vec3(0.2125f, 0.1275f, 0.054f);
    roofBack->material.diffuse = glm::vec3(0.9, 0.22, 0.27);
    roofBack->material.specular = glm::vec3(0.393548f, 0.271906f, 0.166721f);
    roofBack->material.shininess = 0.5;
    scene.objects.push_back(roofBack);

    // HOUSE 1 (CUBE)

    Triangle* frontT1 = new Triangle();
    frontT1->A = glm::vec3(-3, 0, 1);
    frontT1->B = glm::vec3(-2, 0, 1);
    frontT1->C = glm::vec3(-2, 1, 1);
    frontT1->material.ambient = glm::vec3(0.2125f, 0.1275f, 0.054f);
    frontT1->material.diffuse = glm::vec3(0.04, 0.2, 0.41);
    frontT1->material.specular = glm::vec3(0.393548f, 0.271906f, 0.166721f);
    frontT1->material.shininess = 0.5;
    scene.objects.push_back(frontT1);

    Triangle* frontT2 = new Triangle();
    frontT2->A = glm::vec3(-2, 1, 1);
    frontT2->B = glm::vec3(-3, 1, 1);
    frontT2->C = glm::vec3(-3, 0, 1);
    frontT2->material.ambient = glm::vec3(0.2125f, 0.1275f, 0.054f);
    frontT2->material.diffuse = glm::vec3(0.04, 0.2, 0.41);
    frontT2->material.specular = glm::vec3(0.393548f, 0.271906f, 0.166721f);
    frontT2->material.shininess = 0.5;
    scene.objects.push_back(frontT2);

    Triangle* backT1 = new Triangle();
    backT1->A = glm::vec3(-2, 0, 0);
    backT1->B = glm::vec3(-3, 0, 0);
    backT1->C = glm::vec3(-3, 1, 0);
    backT1->material.ambient = glm::vec3(0.2125f, 0.1275f, 0.054f);
    backT1->material.diffuse = glm::vec3(0.04, 0.2, 0.41);
    backT1->material.specular = glm::vec3(0.393548f, 0.271906f, 0.166721f);
    backT1->material.shininess = 0.5;
    scene.objects.push_back(backT1);

    Triangle* backT2 = new Triangle();
    backT2->A = glm::vec3(-3, 1, 0);
    backT2->B = glm::vec3(-2, 1, 0);
    backT2->C = glm::vec3(-2, 0, 0);
    backT2->material.ambient = glm::vec3(0.2125f, 0.1275f, 0.054f);
    backT2->material.diffuse = glm::vec3(0.04, 0.2, 0.41);
    backT2->material.specular = glm::vec3(0.393548f, 0.271906f, 0.166721f);
    backT2->material.shininess = 0.5;
    scene.objects.push_back(backT2);

    Triangle* leftT1 = new Triangle();
    leftT1->A = glm::vec3(-3, 0, 0);
    leftT1->B = glm::vec3(-3, 0, 1);
    leftT1->C = glm::vec3(-3, 1, 1);
    leftT1->material.ambient = glm::vec3(0.2125f, 0.1275f, 0.054f);
    leftT1->material.diffuse = glm::vec3(0.04, 0.2, 0.41);
    leftT1->material.specular = glm::vec3(0.393548f, 0.271906f, 0.166721f);
    leftT1->material.shininess = 0.5;
    scene.objects.push_back(leftT1);

    Triangle* leftT2 = new Triangle();
    leftT2->A = glm::vec3(-3, 1, 1);
    leftT2->B = glm::vec3(-3, 1, 0);
    leftT2->C = glm::vec3(-3, 0, 0);
    leftT2->material.ambient = glm::vec3(0.2125f, 0.1275f, 0.054f);
    leftT2->material.diffuse = glm::vec3(0.04, 0.2, 0.41);
    leftT2->material.specular = glm::vec3(0.393548f, 0.271906f, 0.166721f);
    leftT2->material.shininess = 0.5;
    scene.objects.push_back(leftT2);

    Triangle* rightT1 = new Triangle();
    rightT1->A = glm::vec3(-2, 0, 1);
    rightT1->B = glm::vec3(-2, 0, 0);
    rightT1->C = glm::vec3(-2, 1, 0);
    rightT1->material.ambient = glm::vec3(0.2125f, 0.1275f, 0.054f);
    rightT1->material.diffuse = glm::vec3(0.04, 0.2, 0.41);
    rightT1->material.specular = glm::vec3(0.393548f, 0.271906f, 0.166721f);
    rightT1->material.shininess = 0.5;
    scene.objects.push_back(rightT1);

    Triangle* rightT2 = new Triangle();
    rightT2->A = glm::vec3(-2, 1, 0);
    rightT2->B = glm::vec3(-2, 1, 1);
    rightT2->C = glm::vec3(-2, 0, 1);
    rightT2->material.ambient = glm::vec3(0.2125f, 0.1275f, 0.054f);
    rightT2->material.diffuse = glm::vec3(0.04, 0.2, 0.41);
    rightT2->material.specular = glm::vec3(0.393548f, 0.271906f, 0.166721f);
    rightT2->material.shininess = 0.5;
    scene.objects.push_back(rightT2);

    // ROOF 2

    Triangle* roof2Front = new Triangle();
    roof2Front->A = glm::vec3(2, 1, 1);
    roof2Front->B = glm::vec3(3, 1, 1);
    roof2Front->C = glm::vec3(2.5, 1.5, 0.5);
    roof2Front->material.ambient = glm::vec3(0.2125f, 0.1275f, 0.054f);
    roof2Front->material.diffuse = glm::vec3(1.0f, 0.8, 0.27);
    roof2Front->material.specular = glm::vec3(0.393548f, 0.271906f, 0.166721f);
    roof2Front->material.shininess = 0.5;
    scene.objects.push_back(roof2Front);

    Triangle* roof2Right = new Triangle();
    roof2Right->A = glm::vec3(3, 1, 1);
    roof2Right->B = glm::vec3(3, 1, 0);
    roof2Right->C = glm::vec3(2.5, 1.5, 0.5);
    roof2Right->material.ambient = glm::vec3(0.2125f, 0.1275f, 0.054f);
    roof2Right->material.diffuse = glm::vec3(1.0f, 0.8, 0.27);
    roof2Right->material.specular = glm::vec3(0.393548f, 0.271906f, 0.166721f);
    roof2Right->material.shininess = 0.5;
    scene.objects.push_back(roof2Right);

    Triangle* roof2Left = new Triangle();
    roof2Left->A = glm::vec3(2, 1, 0);
    roof2Left->B = glm::vec3(2, 1, 1);
    roof2Left->C = glm::vec3(2.5, 1.5, 0.5);
    roof2Left->material.ambient = glm::vec3(0.2125f, 0.1275f, 0.054f);
    roof2Left->material.diffuse = glm::vec3(1.0f, 0.8, 0.27);
    roof2Left->material.specular = glm::vec3(0.393548f, 0.271906f, 0.166721f);
    roof2Left->material.shininess = 0.5;
    scene.objects.push_back(roof2Left);

    Triangle* roof2Back = new Triangle();
    roof2Back->A = glm::vec3(3, 1, 0);
    roof2Back->B = glm::vec3(2, 1, 0);
    roof2Back->C = glm::vec3(2.5, 1.5, 0.5);
    roof2Back->material.ambient = glm::vec3(0.2125f, 0.1275f, 0.054f);
    roof2Back->material.diffuse = glm::vec3(1.0f, 0.8, 0.27);
    roof2Back->material.specular = glm::vec3(0.393548f, 0.271906f, 0.166721f);
    roof2Back->material.shininess = 0.5;
    scene.objects.push_back(roof2Back);

    // HOUSE 2 (CUBE)

    Triangle* front2T1 = new Triangle();
    front2T1->A = glm::vec3(2, 0, 1);
    front2T1->B = glm::vec3(3, 0, 1);
    front2T1->C = glm::vec3(2, 1, 1);
    front2T1->material.ambient = glm::vec3(0.2125f, 0.1275f, 0.054f);
    front2T1->material.diffuse = glm::vec3(0.04, 0.2, 0.41);
    front2T1->material.specular = glm::vec3(0.393548f, 0.271906f, 0.166721f);
    front2T1->material.shininess = 0.5;
    scene.objects.push_back(front2T1);

    Triangle* front2T2 = new Triangle();
    front2T2->A = glm::vec3(3, 1, 1);
    front2T2->B = glm::vec3(2, 1, 1);
    front2T2->C = glm::vec3(3, 0, 1);
    front2T2->material.ambient = glm::vec3(0.2125f, 0.1275f, 0.054f);
    front2T2->material.diffuse = glm::vec3(0.04, 0.2, 0.41);
    front2T2->material.specular = glm::vec3(0.393548f, 0.271906f, 0.166721f);
    front2T2->material.shininess = 0.5;
    scene.objects.push_back(front2T2);

    Triangle* back2T1 = new Triangle();
    back2T1->A = glm::vec3(3, 0, 0);
    back2T1->B = glm::vec3(2, 0, 0);
    back2T1->C = glm::vec3(3, 1, 0);
    back2T1->material.ambient = glm::vec3(0.2125f, 0.1275f, 0.054f);
    back2T1->material.diffuse = glm::vec3(0.04, 0.2, 0.41);
    back2T1->material.specular = glm::vec3(0.393548f, 0.271906f, 0.166721f);
    back2T1->material.shininess = 0.5;
    scene.objects.push_back(back2T1);

    Triangle* back2T2 = new Triangle();
    back2T2->A = glm::vec3(2, 1, 0);
    back2T2->B = glm::vec3(3, 1, 0);
    back2T2->C = glm::vec3(2, 0, 0);
    back2T2->material.ambient = glm::vec3(0.2125f, 0.1275f, 0.054f);
    back2T2->material.diffuse = glm::vec3(0.04, 0.2, 0.41);
    back2T2->material.specular = glm::vec3(0.393548f, 0.271906f, 0.166721f);
    back2T2->material.shininess = 0.5;
    scene.objects.push_back(back2T2);

    Triangle* right2T1 = new Triangle();
    right2T1->A = glm::vec3(3, 0, 1);
    right2T1->B = glm::vec3(3, 0, 0);
    right2T1->C = glm::vec3(3, 1, 1);
    right2T1->material.ambient = glm::vec3(0.2125f, 0.1275f, 0.054f);
    right2T1->material.diffuse = glm::vec3(0.04, 0.2, 0.41);
    right2T1->material.specular = glm::vec3(0.393548f, 0.271906f, 0.166721f);
    right2T1->material.shininess = 0.5;
    scene.objects.push_back(right2T1);

    Triangle* right2T2 = new Triangle();
    right2T2->A = glm::vec3(3, 1, 0);
    right2T2->B = glm::vec3(3, 1, 1);
    right2T2->C = glm::vec3(3, 0, 0);
    right2T2->material.ambient = glm::vec3(0.2125f, 0.1275f, 0.054f);
    right2T2->material.diffuse = glm::vec3(0.04, 0.2, 0.41);
    right2T2->material.specular = glm::vec3(0.393548f, 0.271906f, 0.166721f);
    right2T2->material.shininess = 0.5;
    scene.objects.push_back(right2T2);

    Triangle* left2T1 = new Triangle();
    left2T1->A = glm::vec3(2, 0, 0);
    left2T1->B = glm::vec3(2, 0, 1);
    left2T1->C = glm::vec3(2, 1, 0);
    left2T1->material.ambient = glm::vec3(0.2125f, 0.1275f, 0.054f);
    left2T1->material.diffuse = glm::vec3(0.04, 0.2, 0.41);
    left2T1->material.specular = glm::vec3(0.393548f, 0.271906f, 0.166721f);
    left2T1->material.shininess = 0.5;
    scene.objects.push_back(left2T1);

    Triangle* left2T2 = new Triangle();
    left2T2->A = glm::vec3(2, 1, 1);
    left2T2->B = glm::vec3(2, 1, 0);
    left2T2->C = glm::vec3(2, 0, 1);
    left2T2->material.ambient = glm::vec3(0.2125f, 0.1275f, 0.054f);
    left2T2->material.diffuse = glm::vec3(0.04, 0.2, 0.41);
    left2T2->material.specular = glm::vec3(0.393548f, 0.271906f, 0.166721f);
    left2T2->material.shininess = 0.5;
    scene.objects.push_back(left2T2);

    // RIVER

    Triangle* riverT1 = new Triangle();
    riverT1->A = glm::vec3(-10, 0.1, 2);
    riverT1->B = glm::vec3(-10, 0.1, 4);
    riverT1->C = glm::vec3(10, 0.1, 2);
    riverT1->material.ambient = glm::vec3(0.0f, 0.05f, 0.05f);
    riverT1->material.diffuse = glm::vec3(0.04, 0.2, 0.41);
    riverT1->material.specular = glm::vec3(0.04f, 0.7f, 0.7f);
    riverT1->material.shininess = 60;
    scene.objects.push_back(riverT1);
    
    Triangle* riverT2 = new Triangle();
    riverT2->A = glm::vec3(10, 0.1, 4);
    riverT2->B = glm::vec3(10, 0.1, 2);
    riverT2->C = glm::vec3(-10, 0.1, 4);
    riverT2->material.ambient = glm::vec3(0.0f, 0.05f, 0.05f);
    riverT2->material.diffuse = glm::vec3(0.04, 0.2, 0.41);
    riverT2->material.specular = glm::vec3(0.04f, 0.7f, 0.7f);
    riverT2->material.shininess = 60;
    scene.objects.push_back(riverT2);
    

    //*/
    /**
    // Step 11
    string filename;
    float imageWidth, imageHeight;
    float px, py, pz, lx, ly, lz, ux, uy, uz, fov, f;
    int maxDepth = 1;
    int numberOfObjects = 0;
    string shape;
    float sx, sy, sz, radius;
    float ax, ay, az, bx, by, bz, cx, cy, cz;
    float ar, ag, ab, dr, dg, db, sr, sg, sb, s;
    int numberOfLights = 0;
    float lpx, lpy, lpz, lpw, lar, lag, lab, ldr, ldg, ldb, lsr, lsg, lsb, c, l, q;

    ifstream readFile; 
    std::cout << "Enter filename: ";
    std::cin >> filename;
    readFile.open(filename); 
    if (!readFile) 
    { 
        cerr << "Error: file could not be opened" << endl;
        exit(1);
    }

    readFile >> imageWidth >> imageHeight;

    camera.imageWidth = imageWidth;
    camera.imageHeight = imageHeight;
    cout << "Image width and height: " << camera.imageWidth << " " << camera.imageHeight << endl;

    readFile >> px >> py >> pz >> lx >> ly >> lz >> ux >> uy >> uz >> fov >> f;

    camera.position = glm::vec3(px, py, pz);
    camera.lookTarget = glm::vec3(lx, ly, lz);
    camera.globalUp = glm::vec3(ux, uy, uz);
    camera.fovY = glm::radians(fov);
    camera.focalLength = f;

    cout << "Camera position: " << camera.position.x << " " << camera.position.y << " " << camera.position.z << endl;
    cout << "Camera look target: " << camera.lookTarget.x << " " << camera.lookTarget.y << " " << camera.lookTarget.z << endl;
    cout << "Camera global up: " << camera.globalUp.x << " " << camera.globalUp.y << " " << camera.globalUp.z << endl;
    cout << "Camera fov and focal length: " << camera.fovY << " " << camera.focalLength << endl;

    readFile >> maxDepth;
    readFile >> numberOfObjects;

    cout << "Max Depth: " << maxDepth << endl;
    cout << "Number of objects: " << numberOfObjects << endl;

    int counter = 0;
    while (counter < numberOfObjects)
    {
        readFile >> shape;
        if (shape == "sphere")
        {
            readFile >> sx >> sy >> sz >> radius;
            readFile >> ar >> ag >> ab >> dr >> dg >> db >> sr >> sg >> sb >> s;

            Sphere* sphere = new Sphere();
            sphere->center = glm::vec3(sx, sy, sz);
            sphere->radius = radius;

            sphere->material.ambient = glm::vec3(ar, ag, ab);
            sphere->material.diffuse = glm::vec3(dr, dg, db);
            sphere->material.specular = glm::vec3(sr, sg, sb);
            sphere->material.shininess = s;

            scene.objects.push_back(sphere);

            cout << "Sphere center: " << sphere->center.x << " " << sphere->center.y << " " << sphere->center.z << endl;
            cout << "Sphere radius: " << sphere->radius << endl;
            cout << "-------------" << endl;
        }
        else if (shape == "tri")
        {
            readFile >> ax >> ay >> az >> bx >> by >> bz >> cx >> cy >> cz;
            readFile >> ar >> ag >> ab >> dr >> dg >> db >> sr >> sg >> sb >> s;

            Triangle* triangle = new Triangle();
            triangle->A = glm::vec3(ax, ay, az);
            triangle->B = glm::vec3(bx, by, bz);
            triangle->C = glm::vec3(cx, cy, cz);

            triangle->material.ambient = glm::vec3(ar, ag, ab);
            triangle->material.diffuse = glm::vec3(dr, dg, db);
            triangle->material.specular = glm::vec3(sr, sg, sb);
            triangle->material.shininess = s;

            scene.objects.push_back(triangle);

            cout << "Triangle A: " << triangle->A.x << " " << triangle->A.y << " " << triangle->A.z << endl;
            cout << "Triangle B: " << triangle->B.x << " " << triangle->B.y << " " << triangle->B.z << endl;
            cout << "Triangle C: " << triangle->C.x << " " << triangle->C.y << " " << triangle->C.z << endl;
            cout << "-------------" << endl;

        }
        counter++;
    }
    readFile >> numberOfLights;
    int counter2 = 0;
    while (counter2 < numberOfLights)
    {
        readFile >> lpx >> lpy >> lpz >> lpw >> lar >> lag >> lab >> ldr >> ldg >> ldb >> lsr >> lsg >> lsb >> c >> l >> q;

        Light light;
        light.position = glm::vec4(lpx, lpy, lpz, lpw);
        light.ambient = glm::vec3(lar, lag, lab);
        light.diffuse = glm::vec3(ldr, ldg, ldb);
        light.specular = glm::vec3(lsr, lsg, lsb);
        light.constant = c;
        light.linear = l;
        light.quadratic = q;

        scene.lights.push_back(light);
        counter2++;
    }
    readFile.close();
    cout << "End of File Reached" << endl;
    */

    Image image(camera.imageWidth, camera.imageHeight);
    for (int y = 0; y < image.height; ++y)
    {
        for (int x = 0; x < image.width; ++x)
        {
            Ray ray = GetRayThruPixel(camera, x, image.height - y - 1);
            glm::vec3 color = RayTrace(ray, scene, camera, maxDepth);
            image.SetColor(x, y, color);
        }

        std::cout << "Row: " << std::setfill(' ') << std::setw(4) << (y + 1) << " / " << std::setfill(' ') << std::setw(4) << image.height << "\r" << std::flush;
    }
    std::cout << std::endl;
    
    // For Windows
    std::string imageFileName = "scene.png"; // You might need to make this a full path if you are on Mac
    // For Mac
    //std::string imageFileName = "/Users/alycolumbres/Desktop/admu/Y4 S2/GDEV 32/OpenGL/Projects/ProgEx 4 - Ray Tracing/scene.png";
    stbi_write_png(imageFileName.c_str(), image.width, image.height, 3, image.data.data(), 0);
    
    for (size_t i = 0; i < scene.objects.size(); ++i)
    {
        delete scene.objects[i];
    }

    return 0;
}
