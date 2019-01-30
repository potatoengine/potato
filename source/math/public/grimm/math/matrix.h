// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/platform.h"
#include "grimm/math/common.h"
#include "grimm/math/vector.h"

namespace gm {

    class Mat4x4 {
    public:
        static constexpr int component_length = 4;

        using value_type = Vec4;
        using component_type = float;
        using const_array_type = value_type const[component_length];

        Mat4x4() noexcept : r{{1.f, 0.f, 0.f, 0.f}, {0.f, 1.f, 0.f, 0.f}, {0.f, 0.f, 1.f, 0.f}, {0.f, 0.f, 0.f, 1.f}} {}
        /*implicit*/ Mat4x4(noinit_t) noexcept {}
        /*implicit*/ Mat4x4(value_type c0, value_type c1, value_type c2, value_type c3) noexcept : r{c0, c1, c2, c3} {}

        GM_FORCEINLINE static auto GM_VECTORCALL unalignedLoad(const component_type* unaligned) noexcept -> Mat4x4 {
            return {
                Vec4::unalignedLoad(unaligned + 0),
                Vec4::unalignedLoad(unaligned + 4),
                Vec4::unalignedLoad(unaligned + 8),
                Vec4::unalignedLoad(unaligned + 12)};
        }
        GM_FORCEINLINE static auto GM_VECTORCALL alignedLoad(const component_type* aligned) noexcept -> Mat4x4 {
            return {
                Vec4::alignedLoad(aligned + 0),
                Vec4::alignedLoad(aligned + 4),
                Vec4::alignedLoad(aligned + 8),
                Vec4::alignedLoad(aligned + 12)};
        }

        GM_FORCEINLINE auto GM_VECTORCALL unalignedStore(component_type* unaligned) noexcept -> void {
            r[0].unalignedStore(unaligned + 0);
            r[1].unalignedStore(unaligned + 4);
            r[2].unalignedStore(unaligned + 8);
            r[3].unalignedStore(unaligned + 12);
        }
        GM_FORCEINLINE auto GM_VECTORCALL alignedStore(component_type* aligned) noexcept {
            r[0].alignedStore(aligned + 0);
            r[1].alignedStore(aligned + 4);
            r[2].alignedStore(aligned + 8);
            r[3].alignedStore(aligned + 12);
        }

    public:
        value_type r[4];
    };

    using Vector = Vec4;
    using Matrix = Mat4x4;

    static_assert(sizeof(Vec4) == sizeof(float) * 4);
    static_assert(alignof(Vec4) == 16);

    static_assert(sizeof(Mat4x4) == sizeof(float) * 16);
    static_assert(alignof(Mat4x4) == 16);

    auto GM_VECTORCALL operator*(Mat4x4 lhs, Mat4x4 rhs) noexcept -> Mat4x4;
    auto GM_VECTORCALL operator*=(Mat4x4& lhs, Mat4x4 rhs) noexcept -> Mat4x4&;

    auto GM_VECTORCALL operator*(Mat4x4 lhs, Vec4 rhs) noexcept -> Vec4;
    auto GM_VECTORCALL operator*(Vec4 lhs, Mat4x4 rhs) noexcept -> Vec4;

    auto GM_VECTORCALL transpose(Mat4x4 mat) noexcept -> Mat4x4;
    auto GM_VECTORCALL transformInverseUnscaled(Mat4x4 mat) noexcept -> Mat4x4;
    auto GM_VECTORCALL transformInverse(Mat4x4 mat) noexcept -> Mat4x4;

    auto GM_VECTORCALL rotationZ(float radians) noexcept -> Mat4x4;
    auto GM_VECTORCALL rotationY(float radians) noexcept -> Mat4x4;
    auto GM_VECTORCALL rotationX(float radians) noexcept -> Mat4x4;

    auto GM_VECTORCALL projection(float aspect, float fovY, float nearZ, float farZ) noexcept -> Mat4x4;
} // namespace gm
