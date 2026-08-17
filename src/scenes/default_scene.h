#pragma once

#include "renderer/scene.h"
#include "geometry/sphere.h"
#include "geometry/plane.h"

namespace raytracer {

	class DefaultScene : public Scene {
	public:
		std::vector<std::shared_ptr<Shape>> Build() const override {
			std::vector<std::shared_ptr<Shape>> scene;

			scene.push_back(std::make_shared<Plane>(Vec3{ 0.0, -0.5, 0.0 }, Vec3{ 0.0, 1.0, 0.0 }, Vec3{ 0.8, 0.8, 0.8 }, Vec3{ 0, 0, 0 }, MaterialType::Diffuse)); // Floor
			scene.push_back(std::make_shared<Sphere>(Vec3{ 0.0, 0.0, -2.0 }, 0.5, Vec3{ 0.9, 0.3, 0.2 }, Vec3{ 0, 0, 0 }, MaterialType::Diffuse)); // Center Sphere
			scene.push_back(std::make_shared<Sphere>(Vec3{ -1.0, 0.0, -2.2 }, 0.5, Vec3{ 1.0, 1.0, 1.0 }, Vec3{ 0, 0, 0 }, MaterialType::Mirror)); // Left Mirror Sphere
			scene.push_back(std::make_shared<Sphere>(Vec3{ 1.0, 0.0, -1.8 }, 0.5, Vec3{ 0.2, 0.4, 0.9 }, Vec3{ 0, 0, 0 }, MaterialType::Diffuse)); // Right Sphere
			scene.push_back(std::make_shared<Sphere>(Vec3{ 0.0, 3.0, -1.0 }, 0.5, Vec3{ 0, 0, 0 }, Vec3{ 15, 15, 15 }, MaterialType::Diffuse)); // Light Source

			return scene;
		}

		Vec3 CameraPosition() const override { return { 0.0, 0.0, 1.5 }; }
		const char* Name() const override { return "Default (Spheres)"; }
	};

} // namespace raytracer