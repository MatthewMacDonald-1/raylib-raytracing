#include <iostream>
#include <string>
#include <stdio.h>
#include <raylib.h>
#include <cmath>
#include <raymath.h>

#include "utility/utility-core.hpp"

#include "ray-tracing/cpu/rt_cpu.h"

bool flipImage = false;

Color convert_to_raylib_color(RAYTRACING::CPU::color color);
void draw_image_to_screen(int x, int y, RAYTRACING::CPU::color* pixel_data, int width, int height, int samples_per_pixel);
void draw_image_to_screen(int x, int y, RAYTRACING::CPU::PixelChunkData_t* pixel_data, int width, int height, int chunk_size);
void draw_chunks_not_complete_to_screen(int x, int y, bool* render_chunk, int width, int height, int chunk_size);
void draw_chunk_difference_to_screen(int x, int y, double* difference, double mutliplier, int width, int height, int chunk_size, bool print_stats = false);
void draw_chunk_sample_temp_screen(int x, int y, RAYTRACING::CPU::PixelChunkData_t* data, bool* render_chunk, int width, int height, int chunk_size, float sample_count);

int main(int argc, char* argv[]) {

    Tracelog::Debug("Hello World");


    using namespace RAYTRACING::CPU;

    // Render Settings
    float vFov = 20.0f;
    const int samplesPerPixel = 1;
    const int maxDepth = 10;

    // World
    hittable_list world = random_scene();
    point3 currentCameraPos = point3(13, 2, 3);

    double resScale = 2;

    // Window creation
    int renderWidth = 512 * resScale;
    int renderHeight = 256 * resScale;

    int screenWidth = renderWidth, screenHeight = renderHeight * 4;
    InitWindow(screenWidth, screenHeight, "Ray Tracing");
    SetWindowPosition(100, 100);


    // Render size
    //int renderWidth = screenWidth, renderHeight = screenHeight / 3;
    int maxRenderPixels = renderWidth * renderHeight;
    Tracelog::Debug("Max pixels: %d.", maxRenderPixels);

    color* renderOutputPrimay = (color*)malloc(sizeof(color) * maxRenderPixels);
    color* renderOutputSecondary = (color*)malloc(sizeof(color) * maxRenderPixels);

    for (int i = 0; i < maxRenderPixels; i++) {
        renderOutputPrimay[i] = color(0, 0, 0);
        renderOutputSecondary[i] = color(0, 0, 0);
    }

    long frameCount = 1;

    double differnceMult = 1;

    double acceptableNoiseThreshold = 0.01;
    int maxSamples = 250;

    int chunkSize = 8;
    int chunksWide = std::ceil(renderWidth / (float)chunkSize);
    int chunksTall = std::ceil(renderHeight / (float)chunkSize);
    int numberOfChunks = chunksWide * chunksTall;

    //PixelChunkData_t* pixelDataPrimary = (PixelChunkData_t*)malloc(sizeof(PixelChunkData_t) * numberOfChunks);
    //PixelChunkData_t* pixelDataSecondary = (PixelChunkData_t*)malloc(sizeof(PixelChunkData_t) * numberOfChunks);

    PixelChunkData_t* pixelDataPrimary = PixelChunkData_t::Build(renderWidth, renderHeight, chunkSize);
    PixelChunkData_t* pixelDataSecondary = PixelChunkData_t::Build(renderWidth, renderHeight, chunkSize);

    double* chunkDifference = (double*)malloc(sizeof(double) * numberOfChunks);
    bool* renderChunk = (bool*)malloc(sizeof(bool) * numberOfChunks);

    //numberOfChunks = chunksWide;

    for (int i = 0; i < numberOfChunks; i++) {
        chunkDifference[i] = 0;
        renderChunk[i] = true;
    }

    Tracelog::Debug("ChunkSize: %d,", chunkSize);
    Tracelog::Debug("RenderWidth: %d, Therefore there render is %f chunks wide.", renderWidth, chunksWide);
    Tracelog::Debug("renderHeight: %d, Therefore there render is %f chunks tall.", renderHeight, chunksTall);

    bool renderFinished = false;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        BeginDrawing();
        ClearBackground(BLACK);

        // Render scene using software ray tracing

        if (!renderFinished) {
            renderWorldImageMCRT_ChunkWise(pixelDataPrimary, renderWidth, renderHeight, renderChunk, chunkSize, world, maxDepth, currentCameraPos, point3(0, 0, 0), vFov);
        }

        // Compute Chunked difference
        if (frameCount > 1) {
            computeChunkNoise(chunkDifference, pixelDataPrimary, numberOfChunks);
            updateChunksToRender(renderChunk, pixelDataPrimary, maxSamples, acceptableNoiseThreshold, chunkDifference, numberOfChunks);
        }

        // Visualisation of current
        draw_image_to_screen(0, 0, pixelDataPrimary, renderWidth, renderHeight, chunkSize);
        draw_chunks_not_complete_to_screen(0, renderHeight * 1, renderChunk, renderWidth, renderHeight, chunkSize);
        draw_chunk_difference_to_screen(0, renderHeight * 2, chunkDifference, differnceMult, renderWidth, renderHeight, chunkSize, !renderFinished);

        draw_chunk_sample_temp_screen(0, renderHeight * 3, pixelDataPrimary, renderChunk, renderWidth, renderHeight, chunkSize, frameCount);

        if (!renderFinished) {
            int sum = 0;

            for (int i = 0; i < numberOfChunks; i++) {
                sum += pixelDataPrimary[i].number_of_samples;
            }

            Tracelog::Debug("Current sum of samples count: %d", sum);

            sum = 0;

            for (int i = 0; i < numberOfChunks; i++) {
                sum += pixelDataSecondary[i].number_of_samples;
            }

            Tracelog::Debug("Current sum of samples count: %d", sum);
        }
        
        DrawText(TextFormat("Sample #%d", frameCount), 4, 4, 20, RED);

        EndDrawing();

        renderFinished = true;
        for (int i = 0; i < numberOfChunks; i++) {
            if (renderChunk[i]) {
                renderFinished = false;
                break;
            }
        }

        if (!renderFinished) {
            frameCount++;
            //RAYTRACING::CPU::copyImage(renderOutputPrimay, renderOutputSecondary, maxRenderPixels);
            RAYTRACING::CPU::copyImage(pixelDataPrimary, pixelDataSecondary, numberOfChunks);
        }
    }

    free(renderOutputPrimay);
    free(renderOutputSecondary);

    PixelChunkData_t::Free(pixelDataPrimary, renderWidth, renderHeight, chunkSize);
    PixelChunkData_t::Free(pixelDataSecondary, renderWidth, renderHeight, chunkSize);
    free(chunkDifference);
    free(renderChunk);
    CloseWindow();

    return EXIT_SUCCESS;
}

Color convert_to_raylib_color(RAYTRACING::CPU::color color)
{
    Color color_v = {
        static_cast<unsigned char>(256 * std::clamp(color.x(), 0.0, 0.999)),
        static_cast<unsigned char>(256 * std::clamp(color.y(), 0.0, 0.999)),
        static_cast<unsigned char>(256 * std::clamp(color.z(), 0.0, 0.999)),
        static_cast<unsigned char>(255)
    };

    return color_v;
}

void draw_image_to_screen(int x, int y, RAYTRACING::CPU::color* pixel_data, int width, int height, int samples_per_pixel)
{
    using namespace RAYTRACING::CPU;

    int size = width * height;

    for (int index = 0; index < size; index++) {
        int px = index % width;
        int py = index / width;
        if (flipImage) py = height - py - 1; // Flip y coord
        //py = height - py - 1; // Flip y coord

        color correctedColor = correct_color_and_gamma(pixel_data[index], samples_per_pixel);

        DrawPixel(x + px, y + py, convert_to_raylib_color(correctedColor));
    }
}

void draw_image_to_screen(int x, int y, RAYTRACING::CPU::PixelChunkData_t* pixel_data, int width, int height, int chunk_size)
{
    using namespace RAYTRACING::CPU;

    const int chunks_wide = std::ceil(width / (float)chunk_size);
    const int chunks_tall = std::ceil(height / (float)chunk_size);
    const int numberOfChunks = chunks_wide * chunks_tall;

    for (int chunkIndex = 0; chunkIndex < numberOfChunks; chunkIndex++) {


        int cx = chunkIndex % chunks_wide;
        int cy = chunkIndex / chunks_wide;

        int start_y = cy * chunk_size;

        int start_x = cx * chunk_size;

        int end_x = start_x + pixel_data[chunkIndex].width;
        int end_y = start_y + pixel_data[chunkIndex].height;

        int index = 0;
        for (int py = start_y; py < end_y; py++) {
            int py_mod = py;
            if (flipImage) py_mod = height - py - 1; // Flip y coord
            for (int px = start_x; px < end_x; px++) {
                
                color correctedColor = correct_color_and_gamma(pixel_data[chunkIndex].pixel_data[index], pixel_data[chunkIndex].number_of_samples);

                

                DrawPixel(x + px, y + py_mod, convert_to_raylib_color(correctedColor));
                index++;
            }
        }
    }
}

void draw_chunks_not_complete_to_screen(int x, int y, bool* render_chunk, int width, int height, int chunk_size)
{
    int chunksWide = std::ceil(width / (float)chunk_size);
    int chunksTall = std::ceil(height / (float)chunk_size);
    int numberOfChunks = chunksWide * chunksTall;

    for (int index = 0; index < numberOfChunks; index++) {
        int start_x = (index % chunksWide) * chunk_size;
        int start_y = (index / chunksWide) * chunk_size;

        int end_y = std::min(height, start_y + chunk_size);

        int end_x = std::min(width, start_x + chunk_size);

        int chunkWidth = end_x - start_x;
        int chunkHeight = end_y - start_y;

        Color color = render_chunk[index] ? BLUE : GREEN;

        DrawRectangle(x + start_x, y + start_y, chunkWidth, chunkHeight, color);
    }
}

void draw_chunk_difference_to_screen(int x, int y, double* difference, double multiplier, int width, int height, int chunk_size, bool print_stats)
{
    int chunksWide = std::ceil(width / (float)chunk_size);
    int chunksTall = std::ceil(height / (float)chunk_size);
    int numberOfChunks = chunksWide * chunksTall;

    

    double sumOfDifference = 0;
    double minAverageDifference = 1;
    double maxAverageDifference = 0;

    for (int index = 0; index < numberOfChunks; index++) {
        int start_x = (index % chunksWide) * chunk_size;
        int start_y = (index / chunksWide) * chunk_size;

        int end_y = std::min(height, start_y + chunk_size);

        int end_x = std::min(width, start_x + chunk_size);

        int chunkWidth = end_x - start_x;
        int chunkHeight = end_y - start_y;

        double diff = difference[index];

        sumOfDifference += diff;

        if (diff > maxAverageDifference) maxAverageDifference = diff;
        if (diff < minAverageDifference) minAverageDifference = diff;

        DrawRectangle(x + start_x, y + start_y, chunkWidth, chunkHeight, convert_to_raylib_color(RAYTRACING::CPU::color(1, 1, 1) * diff * multiplier));
    }


    

    if (print_stats) {
        Tracelog::Debug("Number of chunks: %d", numberOfChunks);
        Tracelog::Debug("sumOfDifference: %lf", sumOfDifference);
        Tracelog::Debug("Average Pixel Difference: %lf", (sumOfDifference / numberOfChunks));
        Tracelog::Debug("Min Average Pixel Difference: %lf", minAverageDifference);
        Tracelog::Debug("Max Average Pixel Difference: %lf", maxAverageDifference);
    }
}

void draw_chunk_sample_temp_screen(int x, int y, RAYTRACING::CPU::PixelChunkData_t* data, bool* render_chunk, int width, int height, int chunk_size, float sample_count)
{

    int chunksWide = std::ceil(width / (float)chunk_size);
    int chunksTall = std::ceil(height / (float)chunk_size);
    int numberOfChunks = chunksWide * chunksTall;

    for (int index = 0; index < numberOfChunks; index++) {
        int start_x = (index % chunksWide) * chunk_size;
        int start_y = (index / chunksWide) * chunk_size;

        int end_y = std::min(height, start_y + chunk_size);

        int end_x = std::min(width, start_x + chunk_size);

        int chunkWidth = end_x - start_x;
        int chunkHeight = end_y - start_y;

        double scalar = render_chunk[index] ? 1 : 1;

        DrawRectangle(x + start_x, y + start_y, chunkWidth, chunkHeight, convert_to_raylib_color(RAYTRACING::CPU::color(Lerp(0, 1, data[index].number_of_samples / sample_count), 0, Lerp(1, 0, data[index].number_of_samples / sample_count)) * scalar));
        
    }
}
