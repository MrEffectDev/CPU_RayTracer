#pragma once

#include <string>
#include <vector>
#include "math/vec3.h"

namespace raytracer {

    class Texture {
    public:
        virtual ~Texture() = default;
        virtual Vec3 Sample(double u, double v) const = 0;
    };

    class ImageTexture : public Texture {
    public:
        explicit ImageTexture(const std::string& filepath);
        Vec3 Sample(double u, double v) const override;

    private:
        std::vector<unsigned char> data_;
        int width_ = 0;
        int height_ = 0;
        int channels_ = 3;
    };

} //