#include "rtweekend.h"

#include "color.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"

#include <iostream>
#include <thread>
#include <atomic>
#include <future>
#include <chrono>
#include <mutex>
#include <cmath>

#define TRUE 1
#define FALSE 0

namespace RAYTRACING {

	namespace CPU {

		color ray_color(const ray& r, const hittable& world, int depth) {
			hit_record rec;

			// If we've exceed the ray bounce limit, no more light is gathered
			if (depth <= 0) {
				return color(0, 0, 0);
			}

			if (world.hit(r, 0.001, infinity, rec)) {
				ray scattered;
				color attenuation;
				if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
					return attenuation * ray_color(scattered, world, depth - 1);
				return color(0, 0, 0);
			}
			vec3 unit_direction = unit_vector(r.direction());
			auto t = 0.5 * (unit_direction.y() + 1.0);
			return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
		}

		/**
		 * Scene with 3 spheres and ground sphere.
		 * Left: Hollow glass
		 * Center: blue Lambertain
		 * Right: Bronze metal
		*/
		hittable_list scene_a() {
			hittable_list world;

			auto material_ground = make_shared<lambertian>(color(0.8, 0.8, 0.0));
			auto material_center = make_shared<lambertian>(color(0.1, 0.2, 0.5));
			auto material_left = make_shared<dielectric>(1.5);
			auto material_right = make_shared<metal>(color(0.8, 0.6, 0.2), 0.0);

			world.add(make_shared<sphere>(point3(0.0, -100.5, -1.0), 100.0, material_ground));
			world.add(make_shared<sphere>(point3(0.0, 0.0, -1.0), 0.5, material_center));
			world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.5, material_left));
			world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), -0.45, material_left));
			world.add(make_shared<sphere>(point3(1.0, 0.0, -1.0), 0.5, material_right));

			return world;
		}

		/**
		 * A random scene with lots of small spheres and three distinct large spheres
		*/
		hittable_list random_scene() {
			hittable_list world;

			auto material_ground = make_shared<lambertian>(color(0.5, 0.5, 0.5));
			world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, material_ground));

			// Random spheres
			for (int a = -11; a < 11; a++) {
				for (int b = -11; b < 11; b++) {
					auto choose_mat = random_double();
					point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

					if ((center - point3(4, 0.2, 0)).length() > -0.9) {
						shared_ptr<material> sphere_material;

						if (choose_mat < 0.8) {
							// Diffuse
							auto albedo = color::random() * color::random();
							sphere_material = make_shared<lambertian>(albedo);
							world.add(make_shared<sphere>(center, 0.2, sphere_material));
						}
						else if (choose_mat < 0.95) {
							// metal 
							auto albedo = color::random(0.5, 1);
							auto fuzz = random_double(0, 0.5);
							sphere_material = make_shared<metal>(albedo, fuzz);
							world.add(make_shared<sphere>(center, 0.2, sphere_material));
						}
						else {
							sphere_material = make_shared<dielectric>(1.5);
							world.add(make_shared<sphere>(center, 0.2, sphere_material));
						}
					}
				}
			}


			// 3 distinct spheres

			auto mat_1 = make_shared<dielectric>(1.5);
			world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, mat_1));
			// world.add(make_shared<sphere>(point3(0, 1, 0), -0.5, mat_1));

			auto mat_2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
			world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, mat_2));

			auto mat_3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
			world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, mat_3));


			return world;
		}

		/**
		 *
		*/
		unsigned char* colors_to_byte_array(color* pixels, int length, int samples_per_pixel) {
			unsigned char* byte_array = (unsigned char*)malloc(3 * length);

			for (int i = 0; i < length; i++) {
				int byte_index = 3 * i;
				pixels[i] = correct_color_and_gamma(pixels[i], samples_per_pixel);
				byte_array[byte_index] = static_cast<unsigned char>(256 * clamp(pixels[i].x(), 0.0, 0.999));
				byte_array[byte_index + 1] = static_cast<unsigned char>(256 * clamp(pixels[i].y(), 0.0, 0.999));
				byte_array[byte_index + 2] = static_cast<unsigned char>(256 * clamp(pixels[i].z(), 0.0, 0.999));
			}

			return byte_array;
		}

		/**
		 * Multi core renderer
		*/
		void render_world_mt(hittable_list& world, camera cam, int image_width, int image_height, int samples_per_pixel, int max_depth, color* rawPixelColors, bool progressiveRender) {
			int cores = std::thread::hardware_concurrency();
			// volatile std::atomic<std::size_t> count(0);


			int count = 0;
			std::vector<std::future<void>> future_vector;
			int blockSize = 1;
			std::mutex checkoutIndexLock;

			int max = image_width * image_height;
			// printf("Max Pixels: %lld\n", max);
			while (cores--) {
				future_vector.emplace_back(
					std::async([=, &world, &count, &rawPixelColors, &image_width, &image_height, &progressiveRender, &checkoutIndexLock, &blockSize]()
						{
							while (true)
							{
								checkoutIndexLock.lock();
								if (count >= max) {

									checkoutIndexLock.unlock();
									break;
								}
								int indexStart = count;
								int blockToDo = (int)std::min(blockSize, max - indexStart);
								count += blockToDo;
								checkoutIndexLock.unlock();


								// std::size_t index = count++;
								// if (index >= max)
								// 	break;
								for (int i = 0; i < blockToDo; i++) {
									int index = indexStart + i;
									int x = index % image_width;
									int y = index / image_width;
									color pixel_color(0, 0, 0);
									for (int s = 0; s < samples_per_pixel; ++s) {
										auto u = (x + random_double()) / (image_width - 1);
										auto v = (y + random_double()) / (image_height - 1);
										ray r = cam.get_ray(u, v);
										pixel_color += ray_color(r, world, max_depth);
									}
									if (progressiveRender) {
										rawPixelColors[y * image_width + x] += pixel_color;
									}
									else {
										rawPixelColors[y * image_width + x] = pixel_color;
									}
								}
							}
						}
					)
				);
			}
			using namespace std::chrono;
			using namespace std::this_thread;

			while (!(count >= max)) {
				// Wait
				// sleep_for(nanoseconds(1));
				sleep_until(system_clock::now() + milliseconds(2));
			}

			//return rawPixelColors;
		}

		/**
		 * Render an image from a predefined world.
		*/
		void renderWorldImageMCRT(color* pixel_output, int image_width, int image_height, hittable_list& world, int samples_per_pixel, int max_depth, point3 camera_pos, point3 camera_looking_at, double vfov, bool progressiveRender) {
			const double aspect_ratio = (double)image_width / (double)image_height;
			// const double aspect_ratio = 16.0 / 9.0;

			// Camera
			point3 lookfrom = camera_pos;
			point3 lookat = camera_looking_at;
			vec3 vup(0, 1, 0);
			auto dist_to_focus = 10.0;
			auto aperture = 0.0;
			camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus);

			// printf("Starting render (%dx%d)\n", image_width, image_height);
			render_world_mt(world, cam, image_width, image_height, samples_per_pixel, max_depth, pixel_output, progressiveRender);
		}

		struct PixelChunkData_t
		{
			int width;
			int height;
			color* pixel_data;
			int number_of_samples;
			int number_of_pixels;

			static PixelChunkData_t* Build(int width, int height, int chunk_size) {
				int chunks_wide = std::ceil(width / (float)chunk_size);
				int chunks_tall = std::ceil(height / (float)chunk_size);
				int number_of_chunks = chunks_wide * chunks_tall;

				PixelChunkData_t* data = (PixelChunkData_t*)malloc(sizeof(PixelChunkData_t) * number_of_chunks);

				for (int i = 0; i < number_of_chunks; i++) {
					int cx = i % chunks_wide;
					int cy = i / chunks_wide;

					int start_y = cy * chunk_size;
					int end_y = std::min(height, (cy + 1) * chunk_size);

					int start_x = cx * chunk_size;
					int end_x = std::min(width, (cx + 1) * chunk_size);

					int chunk_width = end_x - start_x;
					int chunk_height = end_y - start_y;

					data[i].width = chunk_width;
					data[i].height = chunk_height;
					data[i].number_of_samples = 0;

					data[i].number_of_pixels = chunk_width * chunk_height;

					data[i].pixel_data = (color*)malloc(sizeof(color) * data[i].number_of_pixels);

					for (int j = 0; j < data[i].number_of_pixels; j++) {
						data[i].pixel_data[j] = color(0, 0, 0);
					}
				}

				return data;
			}

			static void Free(PixelChunkData_t* data, int width, int height, int chunk_size) {
				int chunks_wide = std::ceil(width / (float)chunk_size);
				int chunks_tall = std::ceil(height / (float)chunk_size);
				int number_of_chunks = chunks_wide * chunks_tall;

				for (int i = 0; i < number_of_chunks; i++) {
					free(data[i].pixel_data);
				}

				free(data);
			}
		};

		/**
		 * Multi core chunk based renderer. This is a progressive single sample renderer.
		*/
		void render_world_mt_chunk(hittable_list& world, camera cam, int image_width, int image_height, bool* render_chunk, int chunk_size, int max_depth, PixelChunkData_t* output, int thread_limit = -1) {
			const int chunks_wide = std::ceil(image_width / (float)chunk_size);
			const int chunks_tall = std::ceil(image_height / (float)chunk_size);
			const int numberOfChunks = chunks_wide * chunks_tall;
			const int samples_per_pixel = 1;

			std::vector<int> chunkRenderIndexes;
			for (int i = 0; i < numberOfChunks; i++) {
				if (render_chunk[i]) {
					chunkRenderIndexes.push_back(i);
				}
			}

			const int max = chunkRenderIndexes.size();

			int cores = (int)std::thread::hardware_concurrency();

			if (thread_limit != -1 && thread_limit > 0) {
				cores = std::min(cores, thread_limit);
			}

			const int cores_total = cores;
			int chunkRenderIndex = 0;
			std::vector<std::future<void>> future_vector;

			std::mutex checkoutIndexLock;

			std::atomic<int> exited = 0;

			while (cores-- > 0)
				future_vector.emplace_back(
					std::async(
						[=, &output, &chunkRenderIndex, &chunkRenderIndexes, &exited, &image_width, &image_height, &chunks_wide, &chunks_tall, &checkoutIndexLock, &chunk_size, &max]()

						{
							while (true)
							{
								checkoutIndexLock.lock();
								if (chunkRenderIndex >= max) {

									exited++;

									checkoutIndexLock.unlock();
									break;
								}
								int chunkIndex = chunkRenderIndexes[chunkRenderIndex];
								chunkRenderIndex++; // Checkout exactly one chunk to render
								checkoutIndexLock.unlock();

								// Do block render

								int cx = chunkIndex % chunks_wide;
								int cy = chunkIndex / chunks_wide;

								int start_y = cy * chunk_size;

								int start_x = cx * chunk_size;

								int end_x = start_x + output[chunkIndex].width;
								int end_y = start_y + output[chunkIndex].height;

								int index = 0;
								for (int y = start_y; y < end_y; y++) {
									for (int x = start_x; x < end_x; x++) {
										color pixel_color(0, 0, 0);

										for (int s = 0; s < samples_per_pixel; ++s) {
											auto u = (x + random_double()) / (image_width - 1);
											auto v = (y + random_double()) / (image_height - 1);
											ray r = cam.get_ray(u, v);
											pixel_color += ray_color(r, world, max_depth);
										}
										//output[y * image_width + x] += pixel_color;
										output[chunkIndex].pixel_data[index] += pixel_color;
										index++;
									}
								}
								output[chunkIndex].number_of_samples++;
							}
						}
					)
				);

			using namespace std::chrono;
			using namespace std::this_thread;

			// int prevCount, prevCount2 = 0;

			while (!(exited == cores_total)) {
				// Wait
				sleep_until(system_clock::now() + nanoseconds(75000000));
			}

		}

		/**
		* Progressively render an image in chunks from a predefined world.
		*/
		void renderWorldImageMCRT_ChunkWise(PixelChunkData_t* pixel_output, int image_width, int image_height, bool* render_chunk, int chunk_size, hittable_list& world, int max_depth, point3 camera_pos, point3 camera_looking_at, double vfov, int thread_limit = -1) {

			const double aspect_ratio = (double)image_width / (double)image_height;

			// Camera
			point3 lookfrom = camera_pos;
			point3 lookat = camera_looking_at;
			vec3 vup(0, 1, 0);
			auto dist_to_focus = 10.0;
			auto aperture = 0.0;
			camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus);

			render_world_mt_chunk(world, cam, image_width, image_height, render_chunk, chunk_size, max_depth, pixel_output, thread_limit);
		}

		void computeChunkedDifference(double* difference, color* curr, color* prev, int renderWidth, int renderHeight, int samplesPerPixel, int chunkSize, int chunksWide, int chunksTall) {

			for (int cy = 0; cy < chunksTall; cy++) {
				for (int cx = 0; cx < chunksWide; cx++) {

					int start_y = cy * chunkSize;
					int end_y = std::min(renderHeight, (cy + 1) * chunkSize);

					int start_x = cx * chunkSize;
					int end_x = std::min(renderWidth, (cx + 1) * chunkSize);

					int chunkWidth = end_x - start_x;
					int chunkHeight = end_y - start_y;
					int chunkPixelCount = chunkHeight * chunkWidth;

					double renderDifferenceSum = 0;

					for (int y = start_y; y < end_y; y++) {
						for (int x = start_x; x < end_x; x++) {
							int index = y * renderWidth + x;
							//int index = (renderHeight - 1 - y) * renderWidth + x;

							color diff = (correct_color_and_gamma(prev[index], samplesPerPixel - 1) - correct_color_and_gamma(curr[index], samplesPerPixel));

							renderDifferenceSum += diff.length();
						}
					}

					difference[cy * chunksWide + cx] = renderDifferenceSum / chunkPixelCount;
				}
			}
		}

		void computeChunkedDifference(double* difference, PixelChunkData_t* curr, PixelChunkData_t* prev, int size) {

			for (int i = 0; i < size; i++) {

				int width = curr[i].width;
				int height = curr[i].height;
				int number_of_pixels = width * height;

				double renderDifferenceSum = 0;

				for (int j = 0; j < number_of_pixels; j++) {
					color diff = (correct_color_and_gamma(prev[i].pixel_data[j], prev[i].number_of_samples) - correct_color_and_gamma(curr[i].pixel_data[j], curr[i].number_of_samples));

					renderDifferenceSum += diff.length();
				}

				difference[i] = renderDifferenceSum / number_of_pixels;
			}
		}

		void computeChunkNoise(double* noise, PixelChunkData_t* data, int size) {
			for (int i = 0; i < size; i++) {

				double noiseSum = 0;

				std::vector<color> pixel_colors(data[i].number_of_pixels);

				for (int j = 0; j < data[i].number_of_pixels; j++) {
					pixel_colors[j] = correct_color_and_gamma(data[i].pixel_data[j], data[i].number_of_samples);
				}

				for (int y = 0; y < data[i].height; y++) {
					for (int x = 0; x < data[i].width; x++) {

						color neighbours = color(0, 0, 0);
						double sum = 0;

						if (x > 1) {
							neighbours += pixel_colors[y * data[i].width + (x - 1)];
							sum++;
						}
						if (y > 1) {
							neighbours += pixel_colors[(y - 1) * data[i].width + x];
							sum++;
						}

						if (x < data[i].width - 1) { 
							neighbours += pixel_colors[y * data[i].width + (x + 1)];
							sum++;
						}
						if (y < data[i].height - 1) {
							neighbours += pixel_colors[(y + 1) * data[i].width + x];
							sum++;
						}

						noiseSum += (pixel_colors[y * data[i].width + x] - (neighbours / sum)).length();
					}
				}

				noise[i] = noiseSum / data[i].number_of_pixels;
			}
		}

		void updateChunksToRender(bool* renderChunk, PixelChunkData_t* data, int max_samples, double threshold, double* difference, int size) {
			for (int i = 0; i < size; i++) {
				renderChunk[i] = difference[i] > threshold && data[i].number_of_samples < max_samples;
			}
		}

		void copyImage(color* source, color* destination, int size) {
			for (int i = 0; i < size; i++) {
				destination[i] = source[i];
			}
		}

		void copyImage(PixelChunkData_t* source, PixelChunkData_t* destination, int size) {
			for (int i = 0; i < size; i++) {

				if (destination[i].number_of_pixels != source[i].number_of_pixels) {
					throw std::runtime_error("ERROR: pixel data must be of same length");
				}

				destination[i].number_of_samples = source[i].number_of_samples;

				for (int j = 0; j < destination[i].number_of_pixels; j++) {
					destination[i].pixel_data[j] = source[i].pixel_data[j];
				}
			}
		}
	}
}
