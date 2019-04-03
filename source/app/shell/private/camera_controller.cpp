// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "camera_controller.h"
#include "camera.h"
#include <glm/common.hpp>
#include <glm/gtx/rotate_vector.hpp>

void up::FlyCameraController::apply(Camera& camera, glm::vec3 relativeMovement, glm::vec3 relativeMotion, float frameTime) {
    glm::vec3 movement =
        camera.right() * relativeMovement.x +
        camera.up() * relativeMovement.y +
        camera.view() * relativeMovement.z;

    glm::vec3 pos = camera.position() + movement * _moveSpeedPerSec * frameTime;

    _yaw = glm::mod(_yaw - relativeMotion.x, glm::two_pi<float>());
    _pitch = glm::clamp(_pitch - relativeMotion.y, -glm::half_pi<float>() + glm::epsilon<float>(), glm::half_pi<float>() - glm::epsilon<float>());

    glm::vec3 view{0, 0, -1};
    view = glm::rotate(view, _pitch, {1, 0, 0});
    view = glm::rotate(view, _yaw, {0, 1, 0});

    camera.lookAt(pos, pos + view, {0, 1, 0});
}

void up::ArcBallCameraController::apply(Camera& camera, glm::vec3 relativeMovement, glm::vec3 relativeMotion, float frameTime) {
    _target += relativeMovement * 10.f * frameTime;

    _yaw = glm::mod(_yaw + relativeMotion.x, glm::two_pi<float>());
    _pitch = glm::clamp(_pitch - relativeMotion.y, -glm::half_pi<float>() + glm::epsilon<float>(), glm::half_pi<float>() - glm::epsilon<float>());
    _boomLength = glm::clamp(_boomLength - relativeMotion.z, 1.f, 100.f);

    glm::vec3 pos{0, 0, _boomLength};
    pos = glm::rotate(pos, _pitch, {1, 0, 0});
    pos = glm::rotate(pos, _yaw, {0, 1, 0});

    camera.lookAt(pos + _target, _target, {0, 1, 0});
}
