#pragma once

#include "geometry/ray.h"
#include "math/vec3.h"
#include "math/aabb.h"

namespace raytracer {

	enum class MaterialType {
		Diffuse,
		Mirror
	};

	struct HitRecord {
		double t;
		Vec3 point;
		Vec3 normal;
		Vec3 color;
		Vec3 emission;
		MaterialType material;
	};

	class Shape {
	public:
		virtual ~Shape() = default;
		virtual bool Intersect(const Ray& ray, double t_min, double t_max, HitRecord& rec) const = 0;
		virtual bool BoundingBox(AABB& out_box) const = 0;
	};

} // namespace raytracer