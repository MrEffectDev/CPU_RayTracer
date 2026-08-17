#pragma once

#include <cmath>
#include <algorithm>
#include "math/vec3.h"
#include "geometry/ray.h"
#include "geometry/shape.h"

namespace raytracer {

    class Cube : public Shape {
    public:
        Vec3 min_corner;
        Vec3 max_corner;
        Vec3 color;
        Vec3 emission;
        MaterialType material;

        Cube(const Vec3& min_corner, const Vec3& max_corner, const Vec3& color, const Vec3& emission, MaterialType material)
            : min_corner(min_corner), max_corner(max_corner), color(color), emission(emission), material(material) {
        }

        bool Intersect(const Ray& ray, double t_min, double t_max, HitRecord& rec) const override {
            double t_enter = t_min;
            double t_exit = t_max;
            Vec3 hit_normal;

            double origin[3] = { ray.origin.x, ray.origin.y, ray.origin.z };
            double dir[3] = { ray.dir.x, ray.dir.y, ray.dir.z };
            double bmin[3] = { min_corner.x, min_corner.y, min_corner.z };
            double bmax[3] = { max_corner.x, max_corner.y, max_corner.z };
            Vec3 axis_normals[3] = { {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0} };

            for (int i = 0; i < 3; ++i) {
                if (std::fabs(dir[i]) < 1e-12) {
                    if (origin[i] < bmin[i] || origin[i] > bmax[i]) return false;
                    continue;
                }

                double inv_d = 1.0 / dir[i];
                double t0 = (bmin[i] - origin[i]) * inv_d;
                double t1 = (bmax[i] - origin[i]) * inv_d;
                Vec3 n = axis_normals[i];
                if (t0 > t1) { std::swap(t0, t1); n = n * -1.0; }

                if (t0 > t_enter) { t_enter = t0; hit_normal = n; }
                if (t1 < t_exit) t_exit = t1;
                if (t_enter > t_exit) return false;
            }

            if (t_enter <= t_min || t_enter >= t_max) return false;

            rec.t = t_enter;
            rec.point = ray.origin + ray.dir * t_enter;
            rec.normal = hit_normal.Dot(ray.dir) < 0.0 ? hit_normal : hit_normal * -1.0;
            rec.color = color;
            rec.emission = emission;
            rec.material = material;
            return true;
        }
    };

} // namespace raytracer