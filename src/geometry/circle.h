#pragma once

#include <cmath>
#include "math/vec3.h"
#include "geometry/ray.h"
#include "geometry/shape.h"

namespace raytracer {

    class Circle : public Shape {
    public:
        Vec3 center;
        Vec3 normal;
        double radius;
        Vec3 color;
        Vec3 emission;
        MaterialType material;

        Circle(const Vec3& center, const Vec3& normal, double radius, const Vec3& color, const Vec3& emission, MaterialType material)
            : center(center), normal(normal.Normalize()), radius(radius), color(color), emission(emission), material(material) {
        }

        bool Intersect(const Ray& ray, double t_min, double t_max, HitRecord& rec) const override {
            double denom = ray.dir.Dot(normal);
            if (std::fabs(denom) < 1e-6) return false;

            double t = (center - ray.origin).Dot(normal) / denom;
            if (t <= t_min || t >= t_max) return false;

            Vec3 point = ray.origin + ray.dir * t;
            Vec3 diff = point - center;
            if (diff.Dot(diff) > radius * radius) return false;

            rec.t = t;
            rec.point = point;
            rec.normal = denom < 0.0 ? normal : normal * -1.0;
            rec.color = color;
            rec.emission = emission;
            rec.material = material;
            return true;
        }
    };

} // namespace raytracer