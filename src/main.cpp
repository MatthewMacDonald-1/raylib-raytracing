#include <iostream>
#include <string>
#include <stdio.h>
#include <raylib.h>
#include <cmath>

#include "utility/utility-core.hpp"

#include "ray-tracing/cpu/rt_cpu.h"

Color convert_to_raylib_color(RAYTRACING::CPU::color color);

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

    double resScale = 1.5;

    // Window creation
    int screenWidth = 512 * 1 * resScale, screenHeight = 384 * 3 * resScale;
    InitWindow(screenWidth, screenHeight, "Ray Tracing");
    SetWindowPosition(100, 100);


    // Render size
    int renderWidth = screenWidth, renderHeight = screenHeight / 4;
    int maxRenderPixels = renderWidth * renderHeight;
    Tracelog::Debug("Max pixels: %d.", maxRenderPixels);
    bool useSecondaryRenderOutput = false;
    color* renderOutputPrimay = (color*)malloc(sizeof(color) * maxRenderPixels);
    color* renderOutputSecondary = (color*)malloc(sizeof(color) * maxRenderPixels);

    for (int i = 0; i < maxRenderPixels; i++) {
        renderOutputPrimay[i] = color(0, 0, 0);
        renderOutputSecondary[i] = color(0, 0, 0);
    }

    long frameCount = 1;

    double differnceMult = 1;

    double acceptableDifferenceThreshold = 0.0025;
    bool chunkedAverageDifference = true;
    int chunkSize = 16;
    int chunksWide = std::ceil(renderWidth / (float)chunkSize);
    int chunksTall = std::ceil(renderHeight / (float)chunkSize);
    int numberOfChunks = chunksWide * chunksTall;

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

        //renderWorldImageMCRT(renderOutputPrimay, renderWidth, renderHeight, world, 1, maxDepth, currentCameraPos, point3(0, 0, 0), vFov, true);

        if (!renderFinished) {
            renderWorldImageMCRT_ChunkWise(renderOutputPrimay, renderWidth, renderHeight, renderChunk, chunkSize, world, maxDepth, currentCameraPos, point3(0, 0, 0), vFov);
        }

        // Compute Chunked difference
        computeChunkedDifference(chunkDifference, renderOutputPrimay, renderOutputSecondary, renderWidth, renderHeight, frameCount, chunkSize, chunksWide, chunksTall);
        updateChunksToRender(renderChunk, acceptableDifferenceThreshold, chunkDifference, numberOfChunks);

        // Visualisation of current vs previous
        for (int index = 0; index < maxRenderPixels; index++) {
            int x = index % renderWidth;
            int y = index / renderWidth;
            y = renderHeight - y - 1; // Flip y coord

            color correctedColor = correct_color_and_gamma(renderOutputPrimay[index], samplesPerPixel * frameCount);
            color correctedColorPrevious = correct_color_and_gamma(renderOutputSecondary[index], samplesPerPixel * (frameCount - 1));

            DrawPixel(x, y + renderHeight * 0, convert_to_raylib_color(correctedColor));

            DrawPixel(x, y + renderHeight * 1, convert_to_raylib_color(correctedColorPrevious));
        }

        // Visulisation of chunked average difference
        double sumOfDifference = 0;
        double minAverageDifference = 1;
        double maxAverageDifference = 0;
        for (int cy = 0; cy < chunksTall; cy++) {
            for (int cx = 0; cx < chunksWide; cx++) {

                double difference = chunkDifference[cy * chunksWide + cx];
                bool renderIncomplete = renderChunk[cy * chunksWide + cx];

                int start_y = cy * chunkSize;
                int end_y = std::min(renderHeight, (cy + 1) * chunkSize);

                int start_x = cx * chunkSize;
                int end_x = std::min(renderWidth, (cx + 1) * chunkSize);

                int chunkWidth = end_x - start_x;
                int chunkHeight = end_y - start_y;

                sumOfDifference += difference;

                if (difference > maxAverageDifference) maxAverageDifference = difference;
                if (difference < minAverageDifference) minAverageDifference = difference;

                if (renderIncomplete) {
                    DrawRectangle(0 + start_x, renderHeight * 2 + start_y, chunkWidth, chunkHeight, ORANGE);
                }
                else {
                    DrawRectangle(0 + start_x, renderHeight * 2 + start_y, chunkWidth, chunkHeight, GREEN);
                }

                // Chunk average difference
                DrawRectangle(0 + start_x, renderHeight * 3 + start_y, chunkWidth, chunkHeight, convert_to_raylib_color(color(1, 1, 1) * difference * differnceMult));
            }
        }
        
        if (!renderFinished) {
            Tracelog::Debug("Average Pixel Difference: %lf", (sumOfDifference / numberOfChunks));
            Tracelog::Debug("Min Average Pixel Difference: %lf", minAverageDifference);
            Tracelog::Debug("Max Average Pixel Difference: %lf", maxAverageDifference);
        }
        // Visulisations End

        RAYTRACING::CPU::copyImage(renderOutputPrimay, renderOutputSecondary, maxRenderPixels);

        DrawText(TextFormat("Sample #%d", frameCount), 4, 4, 20, LIME);

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
        }
    }

    free(renderOutputPrimay);
    free(renderOutputSecondary);
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
