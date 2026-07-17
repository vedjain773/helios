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
    Vec3 target;
    Vec3 up;
    float fov;
    float near;
    float far;
};

class Camera {
  private:
    Vec3 position;
    Vec3 target;
    Vec3 up;
    float fov;
    float near;
    float far;

    glm::vec3 cameraPos;
    glm::vec3 cameraTarget; 
    glm::vec3 cameraUp;

    glm::mat4 view;
    glm::mat4 proj;

  public:
    Camera(CamConfig &config);
    void setView(Shader &shader);
    void setProj(Shader &shader);
    void setCamPos(Vec3 &cam_pos);
    void setProjCfg(float fov, float near, float far);
    void update();
};

#endif
