#pragma once

#include <algorithm>
#include <memory>
#include <vector>
#include "geometry/shape.h"
#include "math/aabb.h"

namespace raytracer {

    class BVHNode : public Shape {
    public:
        std::shared_ptr<Shape> left;
        std::shared_ptr<Shape> right;
        AABB box;

        bool Intersect(const Ray& ray, double t_min, double t_max, HitRecord& rec) const override {
            if (!box.Hit(ray, t_min, t_max)) return false;

            HitRecord left_rec, right_rec;
            bool hit_left = left->Intersect(ray, t_min, t_max, left_rec);
            bool hit_right = right->Intersect(ray, t_min, hit_left ? left_rec.t : t_max, right_rec);

            if (hit_right) { rec = right_rec; return true; }
            if (hit_left) { rec = left_rec; return true; }
            return false;
        }

        bool BoundingBox(AABB& out_box) const override {
            out_box = box;
            return true;
        }
    };

    inline std::shared_ptr<Shape> BuildBVH(std::vector<std::shared_ptr<Shape>>& objects, size_t start, size_t end) {
        size_t count = end - start;
        if (count == 1) return objects[start];

        AABB total{};
        for (size_t i = start; i < end; ++i) {
            AABB box;
            objects[i]->BoundingBox(box);
            total = AABB::Union(total, box);
        }
        double extent_x = total.max_point.x - total.min_point.x;
        double extent_y = total.max_point.y - total.min_point.y;
        double extent_z = total.max_point.z - total.min_point.z;
        int axis = (extent_x > extent_y && extent_x > extent_z) ? 0 : (extent_y > extent_z ? 1 : 2);

        std::sort(objects.begin() + start, objects.begin() + end,
            [axis](const std::shared_ptr<Shape>& a, const std::shared_ptr<Shape>& b) {
                AABB box_a, box_b;
                a->BoundingBox(box_a);
                b->BoundingBox(box_b);
                return box_a.Centroid(axis) < box_b.Centroid(axis);
            });

        auto node = std::make_shared<BVHNode>();
        size_t mid = start + count / 2;
        node->left = BuildBVH(objects, start, mid);
        node->right = (count == 2) ? objects[mid] : BuildBVH(objects, mid, end);

        AABB box_left, box_right;
        node->left->BoundingBox(box_left);
        node->right->BoundingBox(box_right);
        node->box = AABB::Union(box_left, box_right);

        return node;
    }

    inline void BuildSceneAcceleration(const std::vector<std::shared_ptr<Shape>>& scene,
        std::shared_ptr<Shape>& out_bvh_root,
        std::vector<std::shared_ptr<Shape>>& out_unbounded) {
        std::vector<std::shared_ptr<Shape>> bounded;
        out_unbounded.clear();

        for (const auto& shape : scene) {
            AABB box;
            if (shape->BoundingBox(box)) bounded.push_back(shape);
            else out_unbounded.push_back(shape);
        }

        out_bvh_root = bounded.empty() ? nullptr : BuildBVH(bounded, 0, bounded.size());
    }

} // namespace raytracer