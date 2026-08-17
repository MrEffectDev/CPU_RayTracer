#include "renderer/texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace raytracer {

    ImageTexture::ImageTexture(const std::string& filepath) {
        int channels_in_file = 0;
        unsigned char* pixels = stbi_load(filepath.c_str(), &width_, &height_, &channels_in_file, 3);

        if (!pixels) {
            std::cerr << "Failed to load texture: " << filepath << "\n";
            width_ = height_ = 0;
            return;
        }

        data_.assign(pixels, pixels + static_cast<size_t>(width_) * height_ * channels_);
        stbi_image_free(pixels);
    }

    Vec3 ImageTexture::Sample(double u, double v) const {
        if (width_ == 0 || height_ == 0) return { 1.0, 0.0, 1.0 }; // light-pink if not loaded

        u = u - std::floor(u);
        v = 1.0 - (v - std::floor(v));

        int x = std::min(static_cast<int>(u * width_), width_ - 1);
        int y = std::min(static_cast<int>(v * height_), height_ - 1);

        size_t idx = (static_cast<size_t>(y) * width_ + x) * channels_;
        return {
            data_[idx + 0] / 255.0,
            data_[idx + 1] / 255.0,
            data_[idx + 2] / 255.0
        };
    }

} // namespace raytracer