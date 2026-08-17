#pragma once

#include <vector>
#include <memory>
#include <string>
#include <iostream>

#include "geometry/shape.h"
#include "geometry/triangle.h"
#include "math/vec3.h"


namespace raytracer
{
    std::vector<std::shared_ptr<Shape>> LoadObjModel(
        const std::string& filepath,
        const Vec3& default_color,
        const Vec3& default_emission,
        MaterialType default_material
    );
} // namespace raytracer