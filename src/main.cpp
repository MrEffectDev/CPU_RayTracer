#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <atomic>
#include <thread>
#include <iomanip>

// --- Dear ImGui and GLFW Headers ---
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

// --- Engine and RayTracer Headers ---
#include "jobs/job_system.h"
#include "math/vec3.h"
#include "geometry/ray.h"
#include "geometry/sphere.h"
#include "renderer/path_tracer.h"
#include "renderer/render_context.h"
#include "renderer/tile_job_data.h"


using namespace raytracer;

// --- GLFW Error Callback ---
static void GlfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << "\n";
}

// --- Main Entry ---
int main() {
    // 1. Setup GLFW
    glfwSetErrorCallback(GlfwErrorCallback);
    if (!glfwInit()) return 1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "CPU RayTracer - by MrEffect", nullptr, nullptr);
    if (!window) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // 2. Setup Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // 3. Render Settings and Complex Scene Setup (Cornell Box + Objects)
    constexpr int width = 800;
    constexpr int height = 600;
    constexpr int tile_size = 128;

    std::vector<Sphere> scene = {
        // --- Ground Plane ---
        { {0.0, -100.5, -1.0}, 100.0, {0.8, 0.8, 0.8}, {0, 0, 0}, MaterialType::Diffuse },

        // --- Simple Objects ---
        // Center sphere (diffuse)
        { {0.0, 0.0, -2.0}, 0.5, {0.9, 0.3, 0.2}, {0, 0, 0}, MaterialType::Diffuse },
        // Left sphere (mirror)
        { {-1.0, 0.0, -2.2}, 0.5, {1.0, 1.0, 1.0}, {0, 0, 0}, MaterialType::Mirror },
        // Right sphere (diffuse)
        { {1.0, 0.0, -1.8}, 0.5, {0.2, 0.4, 0.9}, {0, 0, 0}, MaterialType::Diffuse },

        // --- Light Source ---
        { {0.0, 3.0, -1.0}, 0.5, {0, 0, 0}, {15, 15, 15}, MaterialType::Diffuse }
    };

    std::vector<uint8_t> texture_buffer(width * height * 4, 0);
    std::atomic<int> completed_tiles{ 0 };

    RenderContext ctx{
        width, height, 300, 12, {0.0, 0.0, 1.5}, &scene,
        &texture_buffer, &completed_tiles, 0
    };

    // 4. Create OpenGL Texture
    GLuint render_texture;
    glGenTextures(1, &render_texture);
    glBindTexture(GL_TEXTURE_2D, render_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // 5. Threading Variables
    std::atomic<bool> is_rendering{ false };
    std::atomic<bool> render_finished{ false };
    std::thread render_thread;
    JobSystem job_system;

    // --- Main Rendering Loop ---
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- UI Window 1: Controls ---
        ImGui::Begin("RayTracer Control");
        ImGui::Text("CPU RayTracer");
        ImGui::Separator();
        ImGui::Text("Spheres in Scene: %zu", scene.size());
        ImGui::Text("Resolution: %dx%d | SPP: %d", width, height, ctx.samples_per_pixel);

        if (!is_rendering && ImGui::Button("Start Render")) {
            if (render_thread.joinable()) {
                render_thread.join();
            }

            is_rendering = true;
            render_finished = false;
            completed_tiles = 0;
            std::fill(texture_buffer.begin(), texture_buffer.end(), 0);

            render_thread = std::thread([&]() {
                std::vector<TileJobData> tiles;
                for (int y = 0; y < height; y += tile_size) {
                    for (int x = 0; x < width; x += tile_size) {
                        tiles.push_back({ &ctx, x, std::min(x + tile_size, width), y, std::min(y + tile_size, height) });
                    }
                }
                ctx.total_tiles = static_cast<int>(tiles.size());

                Job* root = job_system.CreateJob([](void*) {}, nullptr);
                for (auto& t : tiles) {
                    job_system.Dispatch(RenderTileJobFunction, &t, root);
                }
                job_system.Run(root);
                job_system.Wait(root);

                is_rendering = false;
                render_finished = true;
                });
        }

        // Draw Progress bar
        if (is_rendering || render_finished) {
            float progress = 0.0f;
            if (ctx.total_tiles > 0) {
                progress = static_cast<float>(completed_tiles.load()) / ctx.total_tiles;
            }
            ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f));
        }

        // Output saving mechanism
        if (render_finished) {
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Rendering Complete!");
            if (ImGui::Button("Save Image to Disk (.ppm)")) {
                std::ofstream file("Output_Complex.ppm");
                file << "P3\n" << width << " " << height << "\n255\n";
                for (int y = 0; y < height; ++y) {
                    for (int x = 0; x < width; ++x) {
                        int inverted_y = (height - 1 - y);
                        int idx = (inverted_y * width + x) * 4;
                        file << static_cast<int>(texture_buffer[idx]) << " "
                            << static_cast<int>(texture_buffer[idx + 1]) << " "
                            << static_cast<int>(texture_buffer[idx + 2]) << "\n";
                    }
                }
            }
        }
        ImGui::End();

        // --- UI Window 2: Image Viewport ---
        ImGui::Begin("Render Viewport", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        if (is_rendering || render_finished) {
            glBindTexture(GL_TEXTURE_2D, render_texture);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, texture_buffer.data());
        }

        ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(render_texture)), ImVec2(static_cast<float>(width), static_cast<float>(height)));
        ImGui::End();

        // --- Render ImGui onto OpenGL ---
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // 6. Clean Shutdown
    if (render_thread.joinable()) {
        render_thread.join();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}