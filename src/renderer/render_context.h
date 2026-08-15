#pragma once

#include <vector>
#include <atomic>
#include "math/vec3.h"
#include "geometry/sphere.h"

namespace raytracer {

    struct RenderContext {
        int width;
        int height;
        int samples_per_pixel;
        int max_depth;
        Vec3 camera_pos;
        const std::vector<Sphere>* scene;
        std::vector<uint8_t>* texture_buffer;
        std::atomic<int>* completed_tiles;
        int total_tiles;
    };

} // namespace raytracer