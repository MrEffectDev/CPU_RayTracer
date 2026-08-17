#pragma once

#include "renderer/scene.h"
#include "renderer/obj_loader.h"

namespace raytracer {

    class ObjScene : public Scene {
    public:
        std::vector<std::shared_ptr<Shape>> Build() const override {
            std::vector<std::shared_ptr<Shape>> scene;

            auto cat = LoadObjModel(
                "assets/cat_statue/model.obj",
                Vec3(0.8, 0.7, 0.9),
                Vec3(0.0, 0.0, 0.0),
                MaterialType::Diffuse
            );

            scene.insert(scene.end(), cat.begin(), cat.end());
            scene.push_back(std::make_shared<Plane>(Vec3{ 0.0, -0.5, 0.0 }, Vec3{ 0.0, 1.0, 0.0 }, Vec3{ 0.8, 0.8, 0.8 }, Vec3{ 0, 0, 0 }, MaterialType::Diffuse)); // Floor
            scene.push_back(std::make_shared<Sphere>(Vec3{ 0.0, 3.0, -1.0 }, 0.5, Vec3{ 0, 0, 0 }, Vec3{ 15, 15, 15 }, MaterialType::Diffuse)); // Light Source
            auto car = LoadObjModel(
                "assets/car/car.obj",
                Vec3(0.8, 0.7, 0.9),
                Vec3(0.0, 0.0, 0.0),
                MaterialType::Diffuse
            );
            scene.insert(scene.end(), car.begin(), car.end());

            return scene;
        }

        Vec3 CameraPosition() const override {
            return Vec3(0.2, 0.3, 1.9);
        }

        const char* Name() const override {
            return "OBJ Model Scene";
        }
    };

} // namespace raytracer