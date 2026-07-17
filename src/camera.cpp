#include "camera.hpp"

Camera::Camera(CamConfig &config)
    :position(config.position), target(config.target), up(config.up), 
    fov(config.fov), near(config.near), far(config.far) 
{
    cameraPos = glm::vec3(position.x, position.y, position.z);
    cameraTarget = glm::vec3(target.x, target.y, target.z); 
    cameraUp = glm::vec3(up.x, up.y, up.z);

    view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
    proj = glm::perspective(glm::radians(fov), 800.0f / 600.0f, near, far);
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

void Camera::setProjCfg(float fov, float near, float far) {
    this->fov = fov;
    this->near = near;
    this->far = far;
}

void Camera::update() {
    cameraPos = glm::vec3(position.x, position.y, position.z);
    cameraTarget = glm::vec3(target.x, target.y, target.z); 
    cameraUp = glm::vec3(up.x, up.y, up.z);

    view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
    proj = glm::perspective(glm::radians(fov), 800.0f / 600.0f, near, far);
}
