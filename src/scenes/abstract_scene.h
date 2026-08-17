#pragma once

#include "renderer/scene.h"
#include "geometry/sphere.h"
#include "geometry/plane.h"

namespace raytracer {

	class AbstractScene : public Scene {
	public:
        std::vector<std::unique_ptr<Shape>> Build() const override {
            std::vector<std::unique_ptr<Shape>> scene;

            scene.push_back(std::make_unique<Plane>(
                Vec3{ 0.0, -1.0, 0.0 }, Vec3{ 0.0, 1.0, 0.0 },
                Vec3{ 0.75, 0.75, 0.75 }, Vec3{ 0, 0, 0 }, MaterialType::Diffuse));

            scene.push_back(std::make_unique<Plane>(
                Vec3{ 0.0, 5.0, 0.0 }, Vec3{ 0.0, -1.0, 0.0 },
                Vec3{ 0.85, 0.85, 0.85 }, Vec3{ 0, 0, 0 }, MaterialType::Diffuse));

            scene.push_back(std::make_unique<Rect>(
                Axis::X, -4.0, -4.0, 4.0, -1.0, 5.0,
                Vec3{ 0.68, 0.16, 0.15 }, Vec3{ 0, 0, 0 }, MaterialType::Diffuse));

            scene.push_back(std::make_unique<Rect>(
                Axis::X, 4.0, -4.0, 4.0, -1.0, 5.0,
                Vec3{ 0.15, 0.52, 0.28 }, Vec3{ 0, 0, 0 }, MaterialType::Diffuse));

            scene.push_back(std::make_unique<Rect>(
                Axis::Z, -5.0, -4.0, 4.0, -1.0, 5.0,
                Vec3{ 0.18, 0.18, 0.22 }, Vec3{ 0, 0, 0 }, MaterialType::Diffuse));

            scene.push_back(std::make_unique<Cube>(
                Vec3{ -1.6, -1.0, -3.3 }, Vec3{ -0.6, 0.6, -2.3 },
                Vec3{ 1.0, 1.0, 1.0 }, Vec3{ 0, 0, 0 }, MaterialType::Mirror));

            scene.push_back(std::make_unique<Sphere>(
                Vec3{ 0.9, -0.2, -2.4 }, 0.8,
                Vec3{ 0.92, 0.65, 0.15 }, Vec3{ 0, 0, 0 }, MaterialType::Diffuse));

            scene.push_back(std::make_unique<Sphere>(
                Vec3{ -0.2, 1.2, -2.8 }, 0.45,
                Vec3{ 0.15, 0.45, 0.88 }, Vec3{ 0, 0, 0 }, MaterialType::Diffuse));

            scene.push_back(std::make_unique<Triangle>(
                Vec3{ -0.9, -1.0, -1.5 }, Vec3{ -0.3, -1.0, -1.5 }, Vec3{ -0.6, 0.85, -1.6 },
                Vec3{ 0.25, 0.78, 0.55 }, Vec3{ 0, 0, 0 }, MaterialType::Diffuse));

            scene.push_back(std::make_unique<Rect>(
                Axis::Y, 4.98, -1.5, 1.5, 1.5, -1.5,
                Vec3{ 0, 0, 0 }, Vec3{ 24.0, 24.0, 24.0 }, MaterialType::Diffuse));

            scene.push_back(std::make_unique<Rect>(
                Axis::Z, -3.99, -2.0, 2.0, 1.0, 3.0,
                Vec3{ 0, 0, 0 }, Vec3{ 5.0, 3.2, 1.8 }, MaterialType::Diffuse));
            return scene;
        }

		Vec3 CameraPosition() const override { return { 0.0, 0.0, 1.5 }; }
		const char* Name() const override { return "Something abstract"; }
	};

} // namespace raytracer