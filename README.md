# CPU RayTracer

<img width="1265" height="712" alt="Preview1" src="https://github.com/user-attachments/assets/59ba6234-41e3-447d-8529-d389289fcc51" />


A pet project: a CPU ray tracer written from scratch in C++. No vcpkg, no Embree — just wanted to understand how this stuff actually works under the hood.

Currently supports:
- path tracing with diffuse and mirror materials
- multithreaded rendering via a custom job system (work-stealing)
- an ImGui/GLFW viewer with render progress
- saving the result to .ppm

More is coming — see TODO.md.

## Setup

You'll need CMake 3.20+ and a C++20-capable compiler (built and tested on Windows / MSVC).

```bash
git clone --recurse-submodules https://github.com/MrEffectDev/CPU_RayTracer.git
cd CPU_RayTracer
```

If you cloned without `--recurse-submodules`, pull the dependencies (GLFW and ImGui) separately:

```bash
git submodule update --init --recursive
```

Then a regular CMake build:

```bash
cmake -B build
cmake --build build --config Release
```

The built `CPU_RayTracer` binary will show up in `build/`.

## Project structure

```
src/
  jobs/       — job system for multithreaded rendering
  renderer/   — path tracer, render context
  math/       — vector math
  geometry/   — scene primitives
  scenes/     - showcase scenes
  main.cpp    — window, viewer, ImGui UI
```

## License

MIT!!! Do whatever you want! ^^
