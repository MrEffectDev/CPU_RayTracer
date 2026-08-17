#pragma once

#include <atomic>
#include <memory>
#include <vector>

#include "geometry/shape.h"
#include "math/vec3.h"

namespace raytracer {

    struct RenderContext {
        int width;
        int height;
        int samples_per_pixel;
        int max_depth;
        Vec3 camera_pos;
        const std::vector<std::shared_ptr<Shape>>* scene;
        std::shared_ptr<Shape> bvh_root;
        const std::vector<std::shared_ptr<Shape>>* unbounded_shapes;
        bool use_bvh;
        std::vector<uint8_t>* texture_buffer;
        std::atomic<int>* completed_tiles;
        int total_tiles;
    };

} // namespace raytracer