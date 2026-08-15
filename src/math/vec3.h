#pragma once

#include <cmath>

namespace raytracer {

    struct Vec3 {
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;

        constexpr Vec3() = default;
        constexpr Vec3(double x, double y, double z) : x(x), y(y), z(z) {}

        constexpr Vec3 operator+(const Vec3& v) const { return { x + v.x, y + v.y, z + v.z }; }
        constexpr Vec3 operator-(const Vec3& v) const { return { x - v.x, y - v.y, z - v.z }; }
        constexpr Vec3 operator*(const Vec3& v) const { return { x * v.x, y * v.y, z * v.z }; }
        constexpr Vec3 operator*(double s) const { return { x * s, y * s, z * s }; }
        friend constexpr Vec3 operator*(double s, const Vec3& v) { return v * s; }

        constexpr double Dot(const Vec3& v) const { return x * v.x + y * v.y + z * v.z; }

        inline double Length() const { return std::sqrt(Dot(*this)); }

        inline Vec3 Normalize() const {
            double len = Length();
            return len > 0.0 ? *this * (1.0 / len) : Vec3{};
        }
    };

} // namespace raytracer