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

    Vec3 TracePath(const Ray& ray, const std::vector<std::unique_ptr<Shape>>& scene, int depth, int max_depth) {
        if (depth >= max_depth) return { 0.0, 0.0, 0.0 };

        HitRecord closest_rec;
        bool hit_anything = false;
        double closest_t = 1e9;

        for (const auto& shape : scene) {
            HitRecord temp_rec;
            if (shape->Intersect(ray, 0.001, closest_t, temp_rec)) {
                hit_anything = true;
                closest_t = temp_rec.t;
                closest_rec = temp_rec;
            }
        }

        if (!hit_anything) return { 0.02, 0.02, 0.03 };

        Vec3 emitted = closest_rec.emission;
        Vec3 attenuation = closest_rec.color;

        Ray next_ray;
        if (closest_rec.material == MaterialType::Mirror) {
            Vec3 reflected = ray.dir - 2.0 * ray.dir.Dot(closest_rec.normal) * closest_rec.normal;
            next_ray = { closest_rec.point, reflected.Normalize() };
        }
        else {
            Vec3 new_dir = RandomHemisphereDirection(closest_rec.normal);
            next_ray = { closest_rec.point, new_dir };
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