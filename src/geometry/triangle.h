#pragma once

#include <cmath>
#include "math/vec3.h"
#include "geometry/ray.h"
#include "geometry/shape.h"

namespace raytracer {

    class Triangle : public Shape {
    public:
        Vec3 v0, v1, v2;
        Vec3 normal;
        Vec3 color;
        Vec3 emission;
        MaterialType material;

        Triangle(const Vec3& v0, const Vec3& v1, const Vec3& v2, const Vec3& color, const Vec3& emission, MaterialType material)
            : v0(v0), v1(v1), v2(v2), color(color), emission(emission), material(material) {
            normal = (v1 - v0).Cross(v2 - v0).Normalize();
        }

        bool Intersect(const Ray& ray, double t_min, double t_max, HitRecord& rec) const override {
            const double eps = 1e-8;
            Vec3 edge1 = v1 - v0;
            Vec3 edge2 = v2 - v0;
            Vec3 h = ray.dir.Cross(edge2);
            double a = edge1.Dot(h);
            if (std::fabs(a) < eps) return false; // The ray is almost parallel to the triangle

            double f = 1.0 / a;
            Vec3 s = ray.origin - v0;
            double u = f * s.Dot(h);
            if (u < 0.0 || u > 1.0) return false;

            Vec3 q = s.Cross(edge1);
            double v = f * ray.dir.Dot(q);
            if (v < 0.0 || u + v > 1.0) return false;

            double t = f * edge2.Dot(q);
            if (t <= t_min || t >= t_max) return false;

            rec.t = t;
            rec.point = ray.origin + ray.dir * t;
            rec.normal = normal.Dot(ray.dir) < 0.0 ? normal : normal * -1.0;
            rec.color = color;
            rec.emission = emission;
            rec.material = material;
            return true;
        }

        bool BoundingBox(AABB& out_box) const override {
            Vec3 lo{ std::min({v0.x, v1.x, v2.x}), std::min({v0.y, v1.y, v2.y}), std::min({v0.z, v1.z, v2.z}) };
            Vec3 hi{ std::max({v0.x, v1.x, v2.x}), std::max({v0.y, v1.y, v2.y}), std::max({v0.z, v1.z, v2.z}) };
            const double eps = 1e-4;
            out_box = { lo - Vec3{eps, eps, eps}, hi + Vec3{eps, eps, eps} };
            return true;
        }
    };

} // namespace raytracer