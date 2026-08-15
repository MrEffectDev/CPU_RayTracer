#pragma once

#include "math/vec3.h"

namespace raytracer {

    struct Ray {
        Vec3 origin;
        Vec3 dir;
    };

} // namespace raytracer