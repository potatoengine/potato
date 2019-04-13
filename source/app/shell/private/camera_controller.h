// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include <glm/vec3.hpp>
#include <glm/gtc/constants.hpp>

#pragma once

namespace up {
    class Camera;

    class CameraController {
    public:
        virtual ~CameraController() = default;
        virtual void apply(Camera& camera, glm::vec3 relativeMovement, glm::vec3 relativeMotion, float frameTime) noexcept = 0;
    };

    class FlyCameraController : public CameraController {
    public:
        FlyCameraController(Camera const& camera) noexcept;
        void apply(Camera& camera, glm::vec3 relativeMovement, glm::vec3 relativeMotion, float frameTime) noexcept override;

    private:
        float _moveSpeedPerSec = 10;
        float _rotateRadiansPerSec = 1;
        float _yaw = 0;
        float _pitch = 0;
    };

    class ArcBallCameraController : public CameraController {
    public:
        ArcBallCameraController(Camera const& camera) noexcept;
        void apply(Camera& camera, glm::vec3 relativeMovement, glm::vec3 relativeMotion, float frameTime) noexcept override;

    private:
        glm::vec3 _target = {0, 5, 0};
        float _boomLength = 10;
        float _yaw = 0;
        float _pitch = -glm::quarter_pi<float>();
    };
} // namespace up
