#pragma once

#include <cmath>
#include "math/vec3.h"
#include "geometry/ray.h"
#include "geometry/shape.h"

namespace raytracer {

    class Quad : public Shape {
    public:
        Vec3 corner; // Q
        Vec3 u, v;   // edge vectors from corner
        Vec3 normal;
        double d;
        Vec3 w;
        Vec3 color;
        Vec3 emission;
        MaterialType material;

        Quad(const Vec3& corner, const Vec3& u, const Vec3& v, const Vec3& color, const Vec3& emission, MaterialType material)
            : corner(corner), u(u), v(v), color(color), emission(emission), material(material) {
            Vec3 n = u.Cross(v);
            normal = n.Normalize();
            d = normal.Dot(corner);
            w = n * (1.0 / n.Dot(n));
        }

        bool Intersect(const Ray& ray, double t_min, double t_max, HitRecord& rec) const override {
            double denom = normal.Dot(ray.dir);
            if (std::fabs(denom) < 1e-8) return false;

            double t = (d - normal.Dot(ray.origin)) / denom;
            if (t <= t_min || t >= t_max) return false;

            Vec3 intersection = ray.origin + ray.dir * t;
            Vec3 planar_hitpt = intersection - corner;
            double alpha = w.Dot(planar_hitpt.Cross(v));
            double beta = w.Dot(u.Cross(planar_hitpt));

            if (alpha < 0.0 || alpha > 1.0 || beta < 0.0 || beta > 1.0) return false;

            rec.t = t;
            rec.point = intersection;
            rec.normal = denom < 0.0 ? normal : normal * -1.0;
            rec.color = color;
            rec.emission = emission;
            rec.material = material;
            return true;
        }

        bool BoundingBox(AABB& out_box) const override {
            Vec3 p0 = corner, p1 = corner + u, p2 = corner + v, p3 = corner + u + v;
            Vec3 lo{ std::min({p0.x,p1.x,p2.x,p3.x}), std::min({p0.y,p1.y,p2.y,p3.y}), std::min({p0.z,p1.z,p2.z,p3.z}) };
            Vec3 hi{ std::max({p0.x,p1.x,p2.x,p3.x}), std::max({p0.y,p1.y,p2.y,p3.y}), std::max({p0.z,p1.z,p2.z,p3.z}) };
            const double eps = 1e-4;
            out_box = { lo - Vec3{eps,eps,eps}, hi + Vec3{eps,eps,eps} };
            return true;
        }
    };

} // namespace raytracer