#pragma once

#include <vector>
#include <atomic>
#include <cstdint>
#include "math/vec3.h"
#include "geometry/ray.h"
#include "renderer/render_context.h"
#include "renderer/tile_job_data.h"
#include "geometry/shape.h"
#include <memory>

namespace raytracer {

	double RandomDouble(double min = 0.0, double max = 1.0);
	Vec3 RandomHemisphereDirection(const Vec3& normal);
	Vec3 TracePath(const Ray& ray, const RenderContext& ctx, int depth = 0, int max_depth = 5);
	void RenderTileJobFunction(void* raw_data);

} // namespace raytracer