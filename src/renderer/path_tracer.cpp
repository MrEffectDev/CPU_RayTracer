#include "renderer/path_tracer.h"
#include <random>
#include <algorithm>

namespace raytracer {

    double RandomDouble(double min, double max) {
        thread_local std::mt19937 generator(std::random_device{}());
        std::uniform_real_distribution<double> dist(min, max);
        return dist(generator);
    }

    Vec3 RandomHemisphereDirection(const Vec3& normal) {
        while (true) {
            Vec3 p{ RandomDouble(-1.0, 1.0), RandomDouble(-1.0, 1.0), RandomDouble(-1.0, 1.0) };
            if (p.Dot(p) <= 1.0) {
                p = p.Normalize();
                return p.Dot(normal) > 0.0 ? p : p * -1.0;
            }
        }
    }

    Vec3 TracePath(const Ray& ray, const std::vector<Sphere>& scene, int depth, int max_depth) {
        if (depth >= max_depth) return { 0.0, 0.0, 0.0 };

        double closest_t = 1e9;
        const Sphere* hit_sphere = nullptr;

        for (const auto& sphere : scene) {
            double t;
            if (sphere.Intersect(ray, t) && t < closest_t) {
                closest_t = t;
                hit_sphere = &sphere;
            }
        }

        if (!hit_sphere) return { 0.02, 0.02, 0.03 };

        Vec3 hit_point = ray.origin + ray.dir * closest_t;
        Vec3 normal = (hit_point - hit_sphere->center).Normalize();
        Vec3 emitted = hit_sphere->emission;

        Ray next_ray;
        Vec3 attenuation = hit_sphere->color;

        if (hit_sphere->material == MaterialType::Mirror) {
            Vec3 reflected = ray.dir - 2.0 * ray.dir.Dot(normal) * normal;
            next_ray = { hit_point, reflected.Normalize() };
        }
        else {
            Vec3 new_dir = RandomHemisphereDirection(normal);
            next_ray = { hit_point, new_dir };
        }

        Vec3 incoming = TracePath(next_ray, scene, depth + 1, max_depth);
        return emitted + (attenuation * incoming);
    }

    void RenderTileJobFunction(void* raw_data) {
        auto* tile = static_cast<TileJobData*>(raw_data);
        const auto& ctx = *tile->ctx;

        for (int y = tile->y_start; y < tile->y_end; ++y) {
            for (int x = tile->x_start; x < tile->x_end; ++x) {
                Vec3 pixel_color{ 0.0, 0.0, 0.0 };

                for (int s = 0; s < ctx.samples_per_pixel; ++s) {
                    double u = (x + RandomDouble()) / ctx.width;
                    double v = (y + RandomDouble()) / ctx.height;

                    double dir_x = (u - 0.5) * (static_cast<double>(ctx.width) / ctx.height);
                    double dir_y = -(v - 0.5);
                    Vec3 ray_dir = Vec3(dir_x, dir_y, -1.0).Normalize();

                    Ray ray{ ctx.camera_pos, ray_dir };
                    pixel_color = pixel_color + TracePath(ray, *ctx.scene, 0, ctx.max_depth);
                }

                double scale = 1.0 / ctx.samples_per_pixel;
                double r = std::sqrt(std::max(0.0, pixel_color.x * scale));
                double g = std::sqrt(std::max(0.0, pixel_color.y * scale));
                double b = std::sqrt(std::max(0.0, pixel_color.z * scale));

                int inverted_y = (ctx.height - 1 - y);
                int index = (inverted_y * ctx.width + x) * 4;

                (*ctx.texture_buffer)[index + 0] = static_cast<uint8_t>(255.99 * std::clamp(r, 0.0, 0.999));
                (*ctx.texture_buffer)[index + 1] = static_cast<uint8_t>(255.99 * std::clamp(g, 0.0, 0.999));
                (*ctx.texture_buffer)[index + 2] = static_cast<uint8_t>(255.99 * std::clamp(b, 0.0, 0.999));
                (*ctx.texture_buffer)[index + 3] = 255;
            }
        }

        ctx.completed_tiles->fetch_add(1, std::memory_order_relaxed);
    }

} // namespace raytracer