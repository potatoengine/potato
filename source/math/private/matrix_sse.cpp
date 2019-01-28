// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "vector.h"

#if defined(GM_ARCH_INTEL)

auto GM_VECTORCALL gm::transpose(Matrix4f mat) noexcept -> Matrix4f {
    auto t0 = _mm_unpacklo_ps(mat.c[0].v, mat.c[1].v);
    auto t1 = _mm_unpacklo_ps(mat.c[2].v, mat.c[3].v);
    auto t2 = _mm_unpackhi_ps(mat.c[0].v, mat.c[1].v);
    auto t3 = _mm_unpackhi_ps(mat.c[2].v, mat.c[3].v);
    return Matrix4f(
        _mm_movelh_ps(t0, t1),
        _mm_movehl_ps(t1, t0),
        _mm_movelh_ps(t2, t3),
        _mm_movehl_ps(t3, t2));
}

// from https://lxjk.github.io/2017/09/03/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained.html
auto GM_VECTORCALL gm::transformInverseUnscaled(Matrix4f mat) noexcept -> Matrix4f {
    Matrix4f res{noinit};

    // transpose 3x3, we know m03 = m13 = m23 = 0
    __m128 t0 = _mm_movelh_ps(mat.c[0].v, mat.c[1].v); // 00, 01, 10, 11
    __m128 t1 = _mm_movehl_ps(mat.c[0].v, mat.c[1].v); // 02, 03, 12, 13
    res.c[0].v = _mm_shuffle_ps(t0, mat.c[2].v, _MM_SHUFFLE(0, 2, 0, 3)); // 00, 10, 20, 23(=0)
    res.c[1].v = _mm_shuffle_ps(t0, mat.c[2].v, _MM_SHUFFLE(1, 3, 1, 3)); // 01, 11, 21, 23(=0)
    res.c[2].v = _mm_shuffle_ps(t1, mat.c[2].v, _MM_SHUFFLE(0, 2, 2, 3)); // 02, 12, 22, 23(=0)

    // last line
    res.c[3].v = _mm_mul_ps(res.c[0].v, _mm_shuffle_ps(mat.c[3].v, mat.c[3].v, _MM_SHUFFLE(0, 0, 0, 0)));
    res.c[3].v = _mm_add_ps(res.c[3].v, _mm_mul_ps(res.c[1].v, _mm_shuffle_ps(mat.c[3].v, mat.c[3].v, _MM_SHUFFLE(1, 2, 2, 2))));
    res.c[3].v = _mm_add_ps(res.c[3].v, _mm_mul_ps(res.c[2].v, _mm_shuffle_ps(mat.c[3].v, mat.c[3].v, _MM_SHUFFLE(2, 2, 2, 2))));
    res.c[3].v = _mm_sub_ps(_mm_setr_ps(0.f, 0.f, 0.f, 1.f), res.c[3].v);

    return res;
}

auto GM_VECTORCALL gm::transformInverse(Matrix4f mat) noexcept -> Matrix4f {
    Matrix4f res{noinit};

    // transpose 3x3, we know m03 = m13 = m23 = 0
    __m128 t0 = _mm_movelh_ps(mat.c[0].v, mat.c[1].v); // 00, 01, 10, 11
    __m128 t1 = _mm_movehl_ps(mat.c[0].v, mat.c[1].v); // 02, 03, 12, 13
    res.c[0].v = _mm_shuffle_ps(t0, mat.c[2].v, _MM_SHUFFLE(0, 2, 0, 3)); // 00, 10, 20, 23(=0)
    res.c[1].v = _mm_shuffle_ps(t0, mat.c[2].v, _MM_SHUFFLE(1, 3, 1, 3)); // 01, 11, 21, 23(=0)
    res.c[2].v = _mm_shuffle_ps(t1, mat.c[2].v, _MM_SHUFFLE(0, 2, 2, 3)); // 02, 12, 22, 23(=0)

    // (SizeSqr(c[0]), SizeSqr(c[1]), SizeSqr(c[2]), 0)
    __m128 sizeSqr;
    sizeSqr = _mm_mul_ps(res.c[0].v, res.c[0].v);
    sizeSqr = _mm_add_ps(sizeSqr, _mm_mul_ps(res.c[1].v, res.c[1].v));
    sizeSqr = _mm_add_ps(sizeSqr, _mm_mul_ps(res.c[2].v, res.c[2].v));

    // optional test to avoid divide by 0
    __m128 one = _mm_set1_ps(1.f);
    // for each component, if(sizeSqr < SMALL_NUMBER) sizeSqr = 1;
    __m128 rSizeSqr = _mm_blendv_ps(
        _mm_div_ps(one, sizeSqr),
        one,
        _mm_cmplt_ps(sizeSqr, _mm_set1_ps(std::numeric_limits<float>::min())));

    res.c[0].v = _mm_mul_ps(res.c[0].v, rSizeSqr);
    res.c[1].v = _mm_mul_ps(res.c[1].v, rSizeSqr);
    res.c[2].v = _mm_mul_ps(res.c[2].v, rSizeSqr);

    // last line
    res.c[3].v = _mm_mul_ps(res.c[0].v, _mm_shuffle_ps(mat.c[3].v, mat.c[3].v, _MM_SHUFFLE(0, 0, 0, 0)));
    res.c[3].v = _mm_add_ps(res.c[3].v, _mm_mul_ps(res.c[1].v, _mm_shuffle_ps(mat.c[3].v, mat.c[3].v, _MM_SHUFFLE(1, 2, 2, 2))));
    res.c[3].v = _mm_add_ps(res.c[3].v, _mm_mul_ps(res.c[2].v, _mm_shuffle_ps(mat.c[3].v, mat.c[3].v, _MM_SHUFFLE(2, 2, 2, 2))));
    res.c[3].v = _mm_sub_ps(_mm_setr_ps(0.f, 0.f, 0.f, 1.f), res.c[3].v);

    return res;
}

// https://gist.github.com/rygorous/4172889
static auto GM_VECTORCALL linearCombination(gm::Vector4f lhs, gm::Matrix4f rhs) noexcept -> gm::Vector4f {
    __m128 result;
    result = _mm_mul_ps(_mm_shuffle_ps(lhs.v, lhs.v, 0x00), rhs.c[0].v);
    result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(lhs.v, lhs.v, 0x55), rhs.c[1].v));
    result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(lhs.v, lhs.v, 0xaa), rhs.c[2].v));
    result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(lhs.v, lhs.v, 0xff), rhs.c[3].v));
    return result;
}

auto GM_VECTORCALL gm::operator*(Matrix4f lhs, Matrix4f rhs) noexcept -> Matrix4f {
    Matrix4f result{noinit};
    result.c[0] = linearCombination(lhs.c[0].v, rhs);
    result.c[1] = linearCombination(lhs.c[1].v, rhs);
    result.c[2] = linearCombination(lhs.c[2].v, rhs);
    result.c[3] = linearCombination(lhs.c[3].v, rhs);
    return result;
}

auto GM_VECTORCALL gm::Matrix4f::operator*=(Matrix4f rhs) noexcept -> Matrix4f& {
    c[0] = linearCombination(c[0].v, rhs);
    c[1] = linearCombination(c[1].v, rhs);
    c[2] = linearCombination(c[2].v, rhs);
    c[3] = linearCombination(c[3].v, rhs);
    return *this;
}

auto GM_VECTORCALL gm::operator*(Matrix4f lhs, Matrix4f::value_type rhs) noexcept -> Matrix4f::value_type {
    __m128 prod1 = _mm_dp_ps(lhs.c[0].v, rhs.v, 0xFF);
    __m128 prod2 = _mm_dp_ps(lhs.c[1].v, rhs.v, 0xFF);
    __m128 prod3 = _mm_dp_ps(lhs.c[2].v, rhs.v, 0xFF);
    __m128 prod4 = _mm_dp_ps(lhs.c[3].v, rhs.v, 0xFF);
    return _mm_shuffle_ps(_mm_movelh_ps(prod1, prod2), _mm_movelh_ps(prod3, prod4), _MM_SHUFFLE(2, 0, 2, 0));
}

#endif // defined(GM_ARCH_INTEL)
