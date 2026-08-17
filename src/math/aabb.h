#pragma once

#include <algorithm>
#include <cmath>
#include "vec3.h"
#include "geometry/ray.h"

namespace raytracer {

	struct AABB {
		Vec3 min_point{1e18, 1e18, 1e18};
		Vec3 max_point{-1e18, -1e18, -1e18};

        bool Hit(const Ray& ray, double t_min, double t_max) const {
            for (int axis = 0; axis < 3; ++axis) {
                double origin = (axis == 0) ? ray.origin.x : (axis == 1) ? ray.origin.y : ray.origin.z;
                double dir = (axis == 0) ? ray.dir.x : (axis == 1) ? ray.dir.y : ray.dir.z;
                double bmin = (axis == 0) ? min_point.x : (axis == 1) ? min_point.y : min_point.z;
                double bmax = (axis == 0) ? max_point.x : (axis == 1) ? max_point.y : max_point.z;

                if (std::fabs(dir) < 1e-12) {
                    if (origin < bmin || origin > bmax) return false;
                    continue;
                }

                double inv_d = 1.0 / dir;
                double t0 = (bmin - origin) * inv_d;
                double t1 = (bmax - origin) * inv_d;
                if (t0 > t1) std::swap(t0, t1);

                t_min = t0 > t_min ? t0 : t_min;
                t_max = t1 < t_max ? t1 : t_max;
                if (t_max <= t_min) return false;
            }
            return true;
        }

        static AABB Union(const AABB& a, const AABB& b) {
            return {
                { std::min(a.min_point.x, b.min_point.x), std::min(a.min_point.y, b.min_point.y), std::min(a.min_point.z, b.min_point.z) },
                { std::max(a.max_point.x, b.max_point.x), std::max(a.max_point.y, b.max_point.y), std::max(a.max_point.z, b.max_point.z) }
            };
        }

        double Centroid(int axis) const {
            double lo = (axis == 0) ? min_point.x : (axis == 1) ? min_point.y : min_point.z;
            double hi = (axis == 0) ? max_point.x : (axis == 1) ? max_point.y : max_point.z;
            return 0.5 * (lo + hi);
        }
	};

} // namespace raytracer