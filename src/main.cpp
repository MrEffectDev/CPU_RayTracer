#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <atomic>
#include <thread>
#include <iomanip>
#include <memory>
#include <chrono>

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
#include "geometry/shape.h"
#include "geometry/plane.h"
#include "imgui_internal.h"
#include "geometry/cube.h"
#include "geometry/rect.h"
#include "geometry/circle.h"
#include "geometry/triangle.h"
#include "geometry/quad.h"
#include "renderer/scene.h"
#include "scenes/default_scene.h"
#include "scenes/abstract_scene.h"
#include "renderer/bvh.h"


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

	// 3. Render Settings & Configurable Variables
	int width = 800;
	int height = 600;
	int tile_size = 32;
	int samples_per_pixel = 600;
	int max_bounces = 12;

	std::vector<std::unique_ptr<Scene>> scenes;
	scenes.push_back(std::make_unique<DefaultScene>());
	scenes.push_back(std::make_unique<AbstractScene>());

	std::vector<const char*> scene_names;
	for (const auto& s : scenes) scene_names.push_back(s->Name());

	int selected_scene = 0;
	std::vector<std::shared_ptr<Shape>> scene = scenes[selected_scene]->Build();
	std::shared_ptr<Shape> bvh_root;
	std::vector<std::shared_ptr<Shape>> unbounded_shapes;
	BuildSceneAcceleration(scene, bvh_root, unbounded_shapes);

	Vec3 camera_pos = scenes[selected_scene]->CameraPosition();

	std::vector<uint8_t> texture_buffer(width * height * 4, 0);
	std::atomic<int> completed_tiles{ 0 };

	RenderContext ctx{
		width, height, samples_per_pixel, max_bounces, camera_pos, &scene,
		bvh_root, &unbounded_shapes, true,
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
	std::atomic<double> last_render_time{ 0.0 };
	std::thread render_thread;
	JobSystem job_system;

	// --- Main Rendering Loop ---
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		// Start the ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// --- UI Window 1: Controls & Parameters ---
		ImGui::Begin("RayTracer Control");
		ImGui::Text("CPU RayTracer Configuration");
		ImGui::Separator();
		ImGui::Text("Worker Threads: %d", job_system.GetWorkerCount());
		ImGui::Text("Shapes in Scene: %zu", scene.size());

		if (!is_rendering) {
			// Scene Selection
			if (ImGui::Combo("Scene", &selected_scene, scene_names.data(), static_cast<int>(scene_names.size()))) {
				scene = scenes[selected_scene]->Build();
				BuildSceneAcceleration(scene, bvh_root, unbounded_shapes);
				camera_pos = scenes[selected_scene]->CameraPosition();

				ctx.camera_pos = camera_pos;
				ctx.scene = &scene;
				ctx.bvh_root = bvh_root;
				ctx.unbounded_shapes = &unbounded_shapes;

				render_finished = false;
				std::fill(texture_buffer.begin(), texture_buffer.end(), 0);
			}

			ImGui::Separator();
			ImGui::Text("Render Parameters");

			// Resolution Presets / Custom
			int resolution_idx = (width == 640) ? 0 : (width == 800) ? 1 : (width == 1280) ? 2 : 3;
			const char* res_presets[] = { "640 x 480", "800 x 600", "1280 x 720", "Custom" };
			if (ImGui::Combo("Resolution Preset", &resolution_idx, res_presets, 4)) {
				if (resolution_idx == 0) { width = 640; height = 480; }
				else if (resolution_idx == 1) { width = 800; height = 600; }
				else if (resolution_idx == 2) { width = 1280; height = 720; }

				texture_buffer.resize(width * height * 4);
				std::fill(texture_buffer.begin(), texture_buffer.end(), 0);
				glBindTexture(GL_TEXTURE_2D, render_texture);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

				ctx.width = width;
				ctx.height = height;
			}

			if (resolution_idx == 3) {
				ImGui::DragInt("Width", &width, 10, 100, 3840);
				ImGui::DragInt("Height", &height, 10, 100, 2160);
				if (ImGui::Button("Apply Resolution")) {
					texture_buffer.resize(width * height * 4);
					std::fill(texture_buffer.begin(), texture_buffer.end(), 0);
					glBindTexture(GL_TEXTURE_2D, render_texture);
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
					ctx.width = width;
					ctx.height = height;
				}
			}

			// Quality & Performance Sliders
			ImGui::SliderInt("Samples Per Pixel (SPP)", &samples_per_pixel, 1, 4096);
			ImGui::SliderInt("Max Bounces", &max_bounces, 1, 64);
			ImGui::SliderInt("Tile Size", &tile_size, 8, 128);

			ctx.samples_per_pixel = samples_per_pixel;
			ctx.max_depth = max_bounces;

			// Camera Position
			ImGui::Separator();
			float cam_pos_arr[3] = {
				static_cast<float>(camera_pos.x),
				static_cast<float>(camera_pos.y),
				static_cast<float>(camera_pos.z)
			};

			if (ImGui::DragFloat3("Position", cam_pos_arr, 0.1f)) {
				camera_pos.x = static_cast<double>(cam_pos_arr[0]);
				camera_pos.y = static_cast<double>(cam_pos_arr[1]);
				camera_pos.z = static_cast<double>(cam_pos_arr[2]);
				ctx.camera_pos = camera_pos;
			}

			// BVH Toggle
			ImGui::Separator();
			ImGui::Checkbox("Use BVH Acceleration", &ctx.use_bvh);
		}
		else {
			ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Rendering in progress... Parameters locked.");
		}

		ImGui::Separator();

		// Render Button & Logic
		if (!is_rendering && ImGui::Button("Start Render", ImVec2(-1.0f, 30.0f))) {
			if (render_thread.joinable()) {
				render_thread.join();
			}

			is_rendering = true;
			render_finished = false;
			completed_tiles = 0;
			std::fill(texture_buffer.begin(), texture_buffer.end(), 0);

			render_thread = std::thread([&]() {
				auto start_time = std::chrono::high_resolution_clock::now();

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

				auto end_time = std::chrono::high_resolution_clock::now();
				last_render_time = std::chrono::duration<double>(end_time - start_time).count();

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
			ImGui::Text("Render Time: %.2f seconds", last_render_time.load());
			if (ImGui::Button("Save Image to Disk (.ppm)", ImVec2(-1.0f, 0.0f))) {
				std::ofstream file("Output.ppm");
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

		ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(render_texture)),
			ImVec2(static_cast<float>(width), static_cast<float>(height)),
			ImVec2(0, 1), ImVec2(1, 0));
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