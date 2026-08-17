#pragma once

#include <cmath>
#include "math/vec3.h"
#include "geometry/ray.h"
#include "geometry/shape.h"

namespace raytracer {

    enum class Axis { X, Y, Z };

    class Rect : public Shape {
    public:
        Axis axis;       // the axis perpendicular to which the rectangle lies
        double k;         // a fixed coordinate along the axis
        double a0, a1;     // range along the first of the remaining axes
        double b0, b1;     // range along the second of the remaining axes
        Vec3 color;
        Vec3 emission;
        MaterialType material;

        Rect(Axis axis, double k, double a0, double a1, double b0, double b1, const Vec3& color, const Vec3& emission, MaterialType material)
            : axis(axis), k(k), a0(a0), a1(a1), b0(b0), b1(b1), color(color), emission(emission), material(material) {
        }

        bool Intersect(const Ray& ray, double t_min, double t_max, HitRecord& rec) const override {
            double origin_k, dir_k, origin_a, dir_a, origin_b, dir_b;
            Vec3 normal;

            switch (axis) {
            case Axis::X:
                origin_k = ray.origin.x; dir_k = ray.dir.x;
                origin_a = ray.origin.y; dir_a = ray.dir.y;
                origin_b = ray.origin.z; dir_b = ray.dir.z;
                normal = { 1.0, 0.0, 0.0 };
                break;
            case Axis::Y:
                origin_k = ray.origin.y; dir_k = ray.dir.y;
                origin_a = ray.origin.x; dir_a = ray.dir.x;
                origin_b = ray.origin.z; dir_b = ray.dir.z;
                normal = { 0.0, 1.0, 0.0 };
                break;
            default: // Axis::Z
                origin_k = ray.origin.z; dir_k = ray.dir.z;
                origin_a = ray.origin.x; dir_a = ray.dir.x;
                origin_b = ray.origin.y; dir_b = ray.dir.y;
                normal = { 0.0, 0.0, 1.0 };
                break;
            }

            if (std::fabs(dir_k) < 1e-8) return false;

            double t = (k - origin_k) / dir_k;
            if (t <= t_min || t >= t_max) return false;

            double a = origin_a + t * dir_a;
            double b = origin_b + t * dir_b;
            if (a < a0 || a > a1 || b < b0 || b > b1) return false;

            rec.t = t;
            rec.point = ray.origin + ray.dir * t;
            rec.normal = normal.Dot(ray.dir) < 0.0 ? normal : normal * -1.0;
            rec.color = color;
            rec.emission = emission;
            rec.material = material;
            return true;
        }
    };

} // namespace raytracer