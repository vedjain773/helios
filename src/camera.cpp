#include "camera.hpp"
#include <cmath>
#include <iostream>

Camera::Camera(CamConfig &config)
    : position(config.position),
      up(config.up),
      yaw(config.yaw),
      pitch(config.pitch),
      fov(config.fov),
      width(config.width),
      height(config.height) {
    aspectRatio = width / height;
    update();
}

void Camera::setCamPos(Vec3 &cam_pos) {
    position = cam_pos;
}

void Camera::setCamDir(float yaw, float pitch) {
    this->yaw = yaw;
    this->pitch = pitch;
}

void Camera::setProjCfg(float fov) {
    this->fov = fov;
}

void Camera::update() {
    camPos = glm::vec3(position.x, position.y, position.z);
    glm::vec3 worldUp = glm::vec3(up.x, up.y, up.z);

    float focalLength = 1.0;
    float theta = glm::radians(fov);
    float h = std::tan(theta / 2.0f);
    float viewportHeight = 2 * h * focalLength;
    float viewportWidth = viewportHeight * aspectRatio;

    glm::vec3 forward =
        glm::vec3(cos(glm::radians(pitch)) * cos(glm::radians(yaw)), sin(glm::radians(pitch)),
                  cos(glm::radians(pitch)) * sin(glm::radians(yaw)));

    glm::vec3 w = -forward;
    glm::vec3 u = glm::normalize(glm::cross(worldUp, w));
    glm::vec3 v = glm::cross(w, u);

    glm::vec3 viewportU = viewportWidth * u;
    glm::vec3 viewportV = viewportHeight * v;

    delu = viewportU / width;
    delv = viewportV / height;

    glm::vec3 bottomLeft = camPos - w * focalLength - viewportU * 0.5f - viewportV * 0.5f;
    topLeftPix = bottomLeft + 0.5f * delu + 0.5f * delv;
}

void Camera::updateParams(unsigned int ID) {
    int loc;

    loc = glGetUniformLocation(ID, "center");
    glUniform3fv(loc, 1, glm::value_ptr(camPos));

    loc = glGetUniformLocation(ID, "pixel00Loc");
    glUniform3fv(loc, 1, glm::value_ptr(topLeftPix));

    loc = glGetUniformLocation(ID, "pixelDeltaU");
    glUniform3fv(loc, 1, glm::value_ptr(delu));

    loc = glGetUniformLocation(ID, "pixelDeltaV");
    glUniform3fv(loc, 1, glm::value_ptr(delv));
}
