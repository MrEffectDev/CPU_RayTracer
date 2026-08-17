#pragma once

#include <cmath>
#include "math/vec3.h"
#include "ray.h"
#include "shape.h"

namespace raytracer {

    class Plane : public Shape {
    public:
        Vec3 point;
        Vec3 normal;
        Vec3 color;
        Vec3 emission;
        MaterialType material;

        Plane(const Vec3& point, const Vec3& normal, const Vec3& color, const Vec3& emission, MaterialType material)
            : point(point), normal(normal.Normalize()), color(color), emission(emission), material(material) {
        }

        bool Intersect(const Ray& ray, double t_min, double t_max, HitRecord& rec) const override {
            double denom = ray.dir.Dot(normal);
            if (std::fabs(denom) < 1e-6) return false; // the ray is almost parallel to the plane

            double t = (point - ray.origin).Dot(normal) / denom;
            if (t <= t_min || t >= t_max) return false;

            rec.t = t;
            rec.point = ray.origin + ray.dir * t;
            rec.normal = denom < 0.0 ? normal : normal * -1.0;
            rec.color = color;
            rec.emission = emission;
            rec.material = material;
            return true;
        }

        bool BoundingBox(AABB& out_box) const override {
            // A plane is infinite, so we cannot create a bounding box for it.
            return false;
		}
    };

} // namespace raytracer