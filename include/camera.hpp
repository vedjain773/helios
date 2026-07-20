#ifndef CAMERA_H
#define CAMERA_H

#include "glad/glad.h"
#include "vector.hpp"
#include "shader.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

struct CamConfig {
    Vec3 position; 
    Vec3 up;
    float yaw;
    float pitch;
    float fov;
    float width;
    float height;
};

class Camera {
  private:
    Vec3 position;
    Vec3 up;
    
    float yaw;
    float pitch;
    float fov;
    
    float width;
    float height;
    float aspectRatio;
   
    glm::vec3 camPos;
    glm::vec3 topLeftPix;
    glm::vec3 delu;
    glm::vec3 delv;

  public:
    Camera(CamConfig &config);
    void setCamPos(Vec3 &cam_pos);
    void setCamDir(float yaw, float pitch);
    void setProjCfg(float fov);

    void update();
    void updateParams(unsigned int ID);
};

#endif
