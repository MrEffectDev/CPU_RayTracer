# TODO

Stuff I want to add. No particular order, just whatever I feel like next.

- [ ] scene preview — a separate OpenGL viewport with a rasterized render of the scene, so the camera can be positioned before running a full path trace
- [x] more primitives — cube, plane, triangle as base shapes through a shared Shape interface
- [ ] 3d models — loading models (obj/gltf) as sets of triangles
- [ ] BVH — acceleration structure, without it models with lots of triangles will take forever to render
- [ ] raytracing — switchable render mode (path tracing / classic whitted-style ray tracing)
- [ ] real materials — parameters instead of an enum (roughness, metallic, ior), textures or a flat color
- [ ] configurable render settings through the UI (SPP, max depth, resolution) without rebuilding

More to come.
