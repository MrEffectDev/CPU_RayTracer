#pragma once

#include <cmath>
#include "math/vec3.h"
#include "geometry/ray.h"

namespace raytracer {

    enum class MaterialType {
        Diffuse,
        Mirror
    };

    struct Sphere {
        Vec3 center;
        double radius;
        Vec3 color;
        Vec3 emission;
        MaterialType material;

        bool Intersect(const Ray& ray, double& t) const {
            Vec3 oc = ray.origin - center;
            double b = oc.Dot(ray.dir);
            double c = oc.Dot(oc) - radius * radius;
            double h = b * b - c;
            if (h < 0.0) return false;
            h = std::sqrt(h);
            t = -b - h;
            if (t > 0.001) return true;
            t = -b + h;
            return t > 0.001;
        }
    };

} // namespace raytracer