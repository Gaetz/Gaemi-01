//
// Created by gaetz on 13/06/2021.
//

#include "Transformations.h"
#include "glm/gtx/transform.hpp"

Mat4 engine::math::translate(const Mat4& transform, const Vec3& translation) {
    return glm::translate(transform, translation);
}

Mat4 engine::math::rotate(const Mat4& transform, float radianAngle, const Vec3& axis) {
    return glm::rotate(transform, radianAngle, axis);
}

Mat4 engine::math::scale(const Mat4& transform, const Vec3& scale) {
    return glm::scale(transform, scale);
}

Mat4 engine::math::perspective(float radianFovY, float aspect, float near, float far) {
    return glm::perspective(radianFovY, aspect, near, far);
}


