// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace up {
    class Camera {
    public:
        void lookAt(glm::vec3 pos, glm::vec3 at, glm::vec3 up);

        void position(glm::vec3 pos);
        void rotateYaw(float radians);
        void rotatePitch(float radians);

        glm::vec3 position() const noexcept { return _position; }
        glm::vec3 view() const noexcept { return _view; }
        glm::vec3 up() const noexcept { return _up; }
        glm::vec3 right() const noexcept { return _right; }
        glm::mat4x4 matrix() const noexcept { return _matrix; }

    private:
        glm::vec3 _position = {0, 0, 0};
        glm::vec3 _view = {0, 0, -1};
        glm::vec3 _up = {0, 1, 0};
        glm::vec3 _right = {1, 0, 0};
        glm::mat4x4 _matrix = glm::identity<glm::mat4x4>();
    };
} // namespace up
