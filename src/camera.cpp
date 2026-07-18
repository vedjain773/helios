#include "camera.hpp"
#include <cmath>

Camera::Camera(CamConfig &config)
    :position(config.position), up(config.up), yaw(config.yaw), pitch(config.pitch), 
    fov(config.fov), near(config.near), far(config.far) 
{
    update();
}

void Camera::setView(Shader &shader) {
    glUniformMatrix4fv(
            glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE,
            glm::value_ptr(view));  
}

void Camera::setProj(Shader &shader) {
    glUniformMatrix4fv(
            glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE,
            glm::value_ptr(proj));
}

void Camera::setCamPos(Vec3 &cam_pos) {
    position = cam_pos;
}

void Camera::setCamDir(float yaw, float pitch) {
    this->yaw = yaw;
    this->pitch = pitch;
}

void Camera::setProjCfg(float fov, float near, float far) {
    this->fov = fov;
    this->near = near;
    this->far = far;
}

void Camera::update() {
    cameraPos = glm::vec3(position.x, position.y, position.z);
   
    cameraDir = glm::vec3(
                cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                sin(glm::radians(pitch)), 
                sin(glm::radians(yaw)) * cos(glm::radians(pitch))
                );

    cameraDir = glm::normalize(cameraDir);

    cameraUp = glm::vec3(up.x, up.y, up.z);

    view = glm::lookAt(cameraPos, cameraPos + cameraDir, cameraUp);
    proj = glm::perspective(glm::radians(fov), 800.0f / 600.0f, near, far);
}
