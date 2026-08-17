#pragma once

#include <cmath>
#include "math/vec3.h"
#include "ray.h"
#include "shape.h"

namespace raytracer {

    class Sphere : public Shape{
    public: 
        Vec3 center;
        double radius;
        Vec3 color;
        Vec3 emission;
        MaterialType material;

        Sphere(const Vec3& center, double radius, const Vec3& color, const Vec3& emission, MaterialType material)
            : center(center), radius(radius), color(color), emission(emission), material(material) {
        }

        bool Intersect(const Ray& ray, double t_min, double t_max, HitRecord& rec) const override {
            Vec3 oc = ray.origin - center;
            double b = oc.Dot(ray.dir);
            double c = oc.Dot(oc) - radius * radius;
            double h = b * b - c;
            if (h < 0.0) return false;
            h = std::sqrt(h);

            double t = -b - h;
            if (t <= t_min || t >= t_max) {
                t = -b + h;
                if (t <= t_min || t >= t_max) return false;
            }

            rec.t = t;
            rec.point = ray.origin + ray.dir * t;
            rec.normal = (rec.point - center) / radius;
            rec.color = color;
            rec.emission = emission;
            rec.material = material;
            return true;
        }
    };

} // namespace raytracer