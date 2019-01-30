// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "matrix.h"

#if defined(GM_ARCH_INTEL)

auto GM_VECTORCALL gm::transpose(Mat4x4 mat) noexcept -> Mat4x4 {
    auto t0 = _mm_unpacklo_ps(mat.r[0].v, mat.r[1].v);
    auto t1 = _mm_unpacklo_ps(mat.r[2].v, mat.r[3].v);
    auto t2 = _mm_unpackhi_ps(mat.r[0].v, mat.r[1].v);
    auto t3 = _mm_unpackhi_ps(mat.r[2].v, mat.r[3].v);
    return Mat4x4(
        _mm_movelh_ps(t0, t1),
        _mm_movehl_ps(t1, t0),
        _mm_movelh_ps(t2, t3),
        _mm_movehl_ps(t3, t2));
}

// from https://lxjk.github.io/2017/09/03/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained.html
auto GM_VECTORCALL gm::transformInverseUnscaled(Mat4x4 mat) noexcept -> Mat4x4 {
    Mat4x4 res{noinit};

    // transpose 3x3, we know m03 = m13 = m23 = 0
    __m128 t0 = _mm_movelh_ps(mat.r[0].v, mat.r[1].v); // 00, 01, 10, 11
    __m128 t1 = _mm_movehl_ps(mat.r[0].v, mat.r[1].v); // 02, 03, 12, 13
    res.r[0].v = _mm_shuffle_ps(t0, mat.r[2].v, _MM_SHUFFLE(0, 2, 0, 3)); // 00, 10, 20, 23(=0)
    res.r[1].v = _mm_shuffle_ps(t0, mat.r[2].v, _MM_SHUFFLE(1, 3, 1, 3)); // 01, 11, 21, 23(=0)
    res.r[2].v = _mm_shuffle_ps(t1, mat.r[2].v, _MM_SHUFFLE(0, 2, 2, 3)); // 02, 12, 22, 23(=0)

    // last line
    res.r[3].v = _mm_mul_ps(res.r[0].v, _mm_shuffle_ps(mat.r[3].v, mat.r[3].v, _MM_SHUFFLE(0, 0, 0, 0)));
    res.r[3].v = _mm_add_ps(res.r[3].v, _mm_mul_ps(res.r[1].v, _mm_shuffle_ps(mat.r[3].v, mat.r[3].v, _MM_SHUFFLE(1, 2, 2, 2))));
    res.r[3].v = _mm_add_ps(res.r[3].v, _mm_mul_ps(res.r[2].v, _mm_shuffle_ps(mat.r[3].v, mat.r[3].v, _MM_SHUFFLE(2, 2, 2, 2))));
    res.r[3].v = _mm_sub_ps(_mm_setr_ps(0.f, 0.f, 0.f, 1.f), res.r[3].v);

    return res;
}

auto GM_VECTORCALL gm::transformInverse(Mat4x4 mat) noexcept -> Mat4x4 {
    Mat4x4 res{noinit};

    // transpose 3x3, we know m03 = m13 = m23 = 0
    __m128 t0 = _mm_movelh_ps(mat.r[0].v, mat.r[1].v); // 00, 01, 10, 11
    __m128 t1 = _mm_movehl_ps(mat.r[0].v, mat.r[1].v); // 02, 03, 12, 13
    res.r[0].v = _mm_shuffle_ps(t0, mat.r[2].v, _MM_SHUFFLE(0, 2, 0, 3)); // 00, 10, 20, 23(=0)
    res.r[1].v = _mm_shuffle_ps(t0, mat.r[2].v, _MM_SHUFFLE(1, 3, 1, 3)); // 01, 11, 21, 23(=0)
    res.r[2].v = _mm_shuffle_ps(t1, mat.r[2].v, _MM_SHUFFLE(0, 2, 2, 3)); // 02, 12, 22, 23(=0)

    // (SizeSqr(r[0]), SizeSqr(r[1]), SizeSqr(r[2]), 0)
    __m128 sizeSqr;
    sizeSqr = _mm_mul_ps(res.r[0].v, res.r[0].v);
    sizeSqr = _mm_add_ps(sizeSqr, _mm_mul_ps(res.r[1].v, res.r[1].v));
    sizeSqr = _mm_add_ps(sizeSqr, _mm_mul_ps(res.r[2].v, res.r[2].v));

    // optional test to avoid divide by 0
    __m128 one = _mm_set1_ps(1.f);
    // for each component, if(sizeSqr < SMALL_NUMBER) sizeSqr = 1;
    __m128 rSizeSqr = _mm_blendv_ps(
        _mm_div_ps(one, sizeSqr),
        one,
        _mm_cmplt_ps(sizeSqr, _mm_set1_ps(std::numeric_limits<float>::min())));

    res.r[0].v = _mm_mul_ps(res.r[0].v, rSizeSqr);
    res.r[1].v = _mm_mul_ps(res.r[1].v, rSizeSqr);
    res.r[2].v = _mm_mul_ps(res.r[2].v, rSizeSqr);

    // last line
    res.r[3].v = _mm_mul_ps(res.r[0].v, _mm_shuffle_ps(mat.r[3].v, mat.r[3].v, _MM_SHUFFLE(0, 0, 0, 0)));
    res.r[3].v = _mm_add_ps(res.r[3].v, _mm_mul_ps(res.r[1].v, _mm_shuffle_ps(mat.r[3].v, mat.r[3].v, _MM_SHUFFLE(1, 2, 2, 2))));
    res.r[3].v = _mm_add_ps(res.r[3].v, _mm_mul_ps(res.r[2].v, _mm_shuffle_ps(mat.r[3].v, mat.r[3].v, _MM_SHUFFLE(2, 2, 2, 2))));
    res.r[3].v = _mm_sub_ps(_mm_setr_ps(0.f, 0.f, 0.f, 1.f), res.r[3].v);

    return res;
}

// https://gist.github.com/rygorous/4172889
static auto GM_VECTORCALL linearCombination(gm::Vec4 lhs, gm::Mat4x4 rhs) noexcept -> gm::Vec4 {
    __m128 result;
    result = _mm_mul_ps(_mm_shuffle_ps(lhs.v, lhs.v, 0x00), rhs.r[0].v);
    result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(lhs.v, lhs.v, 0x55), rhs.r[1].v));
    result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(lhs.v, lhs.v, 0xaa), rhs.r[2].v));
    result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(lhs.v, lhs.v, 0xff), rhs.r[3].v));
    return result;
}

auto GM_VECTORCALL gm::operator*(Mat4x4 lhs, Mat4x4 rhs) noexcept -> Mat4x4 {
    Mat4x4 result{noinit};
    result.r[0] = linearCombination(lhs.r[0].v, rhs);
    result.r[1] = linearCombination(lhs.r[1].v, rhs);
    result.r[2] = linearCombination(lhs.r[2].v, rhs);
    result.r[3] = linearCombination(lhs.r[3].v, rhs);
    return result;
}

auto GM_VECTORCALL gm::operator*=(Mat4x4& lhs, Mat4x4 rhs) noexcept -> Mat4x4& {
    lhs.r[0] = linearCombination(lhs.r[0].v, rhs);
    lhs.r[1] = linearCombination(lhs.r[1].v, rhs);
    lhs.r[2] = linearCombination(lhs.r[2].v, rhs);
    lhs.r[3] = linearCombination(lhs.r[3].v, rhs);
    return lhs;
}

auto GM_VECTORCALL gm::operator*(Mat4x4 lhs, Mat4x4::value_type rhs) noexcept -> Vec4 {
    __m128 prod1 = _mm_dp_ps(lhs.r[0].v, rhs.v, 0xFF);
    __m128 prod2 = _mm_dp_ps(lhs.r[1].v, rhs.v, 0xFF);
    __m128 prod3 = _mm_dp_ps(lhs.r[2].v, rhs.v, 0xFF);
    __m128 prod4 = _mm_dp_ps(lhs.r[3].v, rhs.v, 0xFF);
    return _mm_shuffle_ps(_mm_movelh_ps(prod1, prod2), _mm_movelh_ps(prod3, prod4), _MM_SHUFFLE(2, 0, 2, 0));
}

auto GM_VECTORCALL gm::operator*(Vec4 lhs, Mat4x4 rhs) noexcept -> Vec4 {
    // Splat x,y,z and w
    __m128 xv = _mm_shuffle_ps(lhs.v, lhs.v, _MM_SHUFFLE(0, 0, 0, 0));
    __m128 yv = _mm_shuffle_ps(lhs.v, lhs.v, _MM_SHUFFLE(1, 1, 1, 1));
    __m128 zv = _mm_shuffle_ps(lhs.v, lhs.v, _MM_SHUFFLE(2, 2, 2, 2));
    __m128 wv = _mm_shuffle_ps(lhs.v, lhs.v, _MM_SHUFFLE(3, 3, 3, 3));
    // Mul by the matrix
    xv = _mm_mul_ps(xv, rhs.r[0].v);
    yv = _mm_mul_ps(yv, rhs.r[1].v);
    zv = _mm_mul_ps(zv, rhs.r[2].v);
    wv = _mm_mul_ps(wv, rhs.r[3].v);
    // Add them all together
    xv = _mm_add_ps(xv, yv);
    zv = _mm_add_ps(zv, wv);
    xv = _mm_add_ps(xv, zv);
    return xv;
}

#endif // defined(GM_ARCH_INTEL)
