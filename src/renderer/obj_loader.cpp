#include "obj_loader.h"
#include "renderer/texture.h"

#include <filesystem>
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace raytracer {

    std::vector<std::shared_ptr<Shape>> LoadObjModel(
        const std::string& filepath,
        const Vec3& default_color,
        const Vec3& default_emission,
        MaterialType default_material
    ) {
        std::vector<std::shared_ptr<Shape>> shapes_list;

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string err;

        std::string base_dir = std::filesystem::path(filepath).parent_path().string();
        if (!base_dir.empty() && base_dir.back() != '/' && base_dir.back() != '\\') {
            base_dir += '/';
        }

        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filepath.c_str(), base_dir.c_str());

        if (!err.empty()) std::cerr << "OBJ ERROR: " << err << "\n";
        if (!ret) {
            std::cerr << "Failed to load/parse .obj file: " << filepath << "\n";
            return shapes_list;
        }

        std::unordered_map<std::string, std::shared_ptr<Texture>> texture_cache;
        auto get_texture = [&](const std::string& texname) -> std::shared_ptr<Texture> {
            if (texname.empty()) return nullptr;
            auto it = texture_cache.find(texname);
            if (it != texture_cache.end()) return it->second;

            std::filesystem::path tex_path(texname);
            std::string full_path = tex_path.is_absolute() ? texname : base_dir + texname;

            auto tex = std::make_shared<ImageTexture>(full_path);
            texture_cache[texname] = tex;
            return tex;
            };

        bool has_uvs = !attrib.texcoords.empty();

        for (const auto& shape : shapes) {
            size_t index_offset = 0;
            for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
                int fv = shape.mesh.num_face_vertices[f];

                if (fv == 3) {
                    tinyobj::index_t idx0 = shape.mesh.indices[index_offset + 0];
                    tinyobj::index_t idx1 = shape.mesh.indices[index_offset + 1];
                    tinyobj::index_t idx2 = shape.mesh.indices[index_offset + 2];

                    Vec3 v0(attrib.vertices[3 * idx0.vertex_index + 0], attrib.vertices[3 * idx0.vertex_index + 1], attrib.vertices[3 * idx0.vertex_index + 2]);
                    Vec3 v1(attrib.vertices[3 * idx1.vertex_index + 0], attrib.vertices[3 * idx1.vertex_index + 1], attrib.vertices[3 * idx1.vertex_index + 2]);
                    Vec3 v2(attrib.vertices[3 * idx2.vertex_index + 0], attrib.vertices[3 * idx2.vertex_index + 1], attrib.vertices[3 * idx2.vertex_index + 2]);

                    UV uv0{ 0.0, 0.0 }, uv1{ 1.0, 0.0 }, uv2{ 0.0, 1.0 };
                    if (has_uvs && idx0.texcoord_index >= 0 && idx1.texcoord_index >= 0 && idx2.texcoord_index >= 0) {
                        uv0 = { attrib.texcoords[2 * idx0.texcoord_index + 0], attrib.texcoords[2 * idx0.texcoord_index + 1] };
                        uv1 = { attrib.texcoords[2 * idx1.texcoord_index + 0], attrib.texcoords[2 * idx1.texcoord_index + 1] };
                        uv2 = { attrib.texcoords[2 * idx2.texcoord_index + 0], attrib.texcoords[2 * idx2.texcoord_index + 1] };
                    }

                    std::shared_ptr<Texture> face_texture;
                    if (f < shape.mesh.material_ids.size()) {
                        int mat_id = shape.mesh.material_ids[f];
                        if (mat_id >= 0 && !materials[mat_id].diffuse_texname.empty()) {
                            face_texture = get_texture(materials[mat_id].diffuse_texname);
                        }
                    }

                    shapes_list.push_back(std::make_shared<Triangle>(
                        v0, v1, v2, default_color, default_emission, default_material,
                        uv0, uv1, uv2, face_texture
                    ));
                }
                index_offset += fv;
            }
        }

        std::cout << "Successfully loaded " << shapes_list.size() << " triangles from OBJ model: " << filepath << "\n";
        return shapes_list;
    }

} // namespace raytracer