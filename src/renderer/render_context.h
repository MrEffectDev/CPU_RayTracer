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
        const std::vector<std::unique_ptr<Shape>>* scene;
        std::vector<uint8_t>* texture_buffer;
        std::atomic<int>* completed_tiles;
        int total_tiles;
    };

} // namespace raytracer