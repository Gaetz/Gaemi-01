//
// Created by gaetz on 12/06/2021.
//

#ifndef MATH_FUNCTIONS_H
#define MATH_FUNCTIONS_H

#include "../../../externals/glm/glm/trigonometric.hpp"
#include "../Defines.h"

namespace engine::math {

const f32 pi = 3.1415926535f;
const f32 twoPi = pi * 2.0f;
const f32 piOver2 = pi / 2.0f;
const f32 infinity = std::numeric_limits<f32>::infinity();
const f32 negInfinity = -std::numeric_limits<f32>::infinity();

inline f32 toRad(f32 degrees) {
    return glm::radians(degrees);
}

inline f32 toDeg(f32 radians) {
    return glm::degrees(radians);
}

inline bool nearZero(f32 val, f32 epsilon = 0.001f) {
    if (fabs(val) <= epsilon) {
        return true;
    } else {
        return false;
    }
}

template<typename T>
T max(const T& a, const T& b) {
    return (a < b ? b : a);
}

template<typename T>
T min(const T& a, const T& b) {
    return (a < b ? a : b);
}

template<typename T>
T clamp(const T& value, const T& lower, const T& upper) {
    return min(upper, max(lower, value));
}

inline f32 abs(f32 value) {
    return fabs(value);
}

inline f32 cos(f32 angle) {
    return cosf(angle);
}

inline f32 sin(f32 angle) {
    return sinf(angle);
}

inline f32 tan(f32 angle) {
    return tanf(angle);
}

inline f32 acos(f32 value) {
    return acosf(value);
}

inline f32 atan2(f32 y, f32 x) {
    return atan2f(y, x);
}

inline f32 cot(f32 angle) {
    return 1.0f / tan(angle);
}

inline f32 lerp(f32 a, f32 b, f32 f) {
    return a + f * (b - a);
}

inline f32 sqrt(f32 value) {
    return sqrtf(value);
}

inline f32 fmod(f32 numer, f32 denom) {
    return std::fmod(numer, denom);
}

inline i32 round(f32 num) {
    return static_cast<i32>(std::round(num));
}

}
#endif //MATH_FUNCTIONS_H
