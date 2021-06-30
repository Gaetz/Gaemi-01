//
// Created by gaetz on 13/06/2021.
//

#ifndef MATH_TRANSFORMATIONS_H
#define MATH_TRANSFORMATIONS_H

#include "Types.h"

using engine::math::Mat4;
using engine::math::Vec3;

namespace engine::math {

Mat4 translate(const Mat4& transform, const Vec3& translation);
Mat4 rotate(const Mat4& transform, float radianAngle, const Vec3& axis);
Mat4 scale(const Mat4& transform, const Vec3& scale);

Mat4 perspective(float radianFovY, float aspect, float near, float far);

}

#endif //MATH_TRANSFORMATIONS_H
