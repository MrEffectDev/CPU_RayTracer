#pragma once

#include <vector>
#include <memory>
#include "math/vec3.h"
#include "geometry/shape.h"

namespace raytracer {

	class Scene {
	public:
		virtual ~Scene() = default;
		virtual std::vector<std::unique_ptr<Shape>> Build() const = 0;
		virtual Vec3 CameraPosition() const = 0;
		virtual const char* Name() const = 0;
	};

} // namespace raytracer