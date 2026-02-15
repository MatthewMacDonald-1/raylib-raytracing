#include <iostream>
#include <string>
#include <stdio.h>
#include <raylib.h>
#include <cmath>

#include "utility/utility-core.hpp"

#include "ray-tracing/cpu/rt_cpu.h"

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

    // Window creation
    int screenWidth = 516 * 1, screenHeight = 384 * 3;
    InitWindow(screenWidth, screenHeight, "Ray Tracing");
    SetWindowPosition(100, 100);


    // Render size
    int renderWidth = screenWidth, renderHeight = screenHeight / 4;
    int maxRenderPixels = renderWidth * renderHeight;
    Tracelog::Debug("Max pixels: %d.", maxRenderPixels);
    bool useSecondaryRenderOutput = false;
    color* renderOutputPrimay = (color*)malloc(sizeof(color) * maxRenderPixels);
    color* renderOutputSecondary = (color*)malloc(sizeof(color) * maxRenderPixels);
    double* renderDifference = (double*)malloc(sizeof(color) * maxRenderPixels);
    //color* renderDifference = (color*)malloc(sizeof(color) * maxRenderPixels);


    for (int i = 0; i < maxRenderPixels; i++) {
        renderOutputPrimay[i] = color(0, 0, 0);
        renderOutputSecondary[i] = color(0, 0, 0);
        //renderDifference[i] = color(0, 0, 0);
    }

    long frameCount = 1;

    bool progressive = true;

    double differnceMult = 10;

    bool chunkedAverageDifference = true;
    int chunkSize = 8;
    int chunksWide = std::ceil(renderWidth / (float)chunkSize);
    int chunksTall = std::ceil(renderHeight / (float)chunkSize);

    Tracelog::Debug("ChunkSize: %d,", chunkSize);
    Tracelog::Debug("RenderWidth: %d, Therefore there render is %f chunks wide.", renderWidth, chunksWide);
    Tracelog::Debug("renderHeight: %d, Therefore there render is %f chunks tall.", renderHeight, chunksTall);


    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        BeginDrawing();
        ClearBackground(BLACK);

        // Render scene using software ray tracing

        renderWorldImageMCRT(renderOutputPrimay, renderWidth, renderHeight, world, samplesPerPixel, maxDepth, currentCameraPos, point3(0, 0, 0), vFov, progressive);

        double renderDifferenceSum = 0;

        for (int index = 0; index < maxRenderPixels; index++) {
            int x = index % renderWidth;
            int y = index / renderWidth;
            y = renderHeight - y - 1; // Flip y coord

            color correctedColor = correct_color_and_gamma(renderOutputPrimay[index], samplesPerPixel * frameCount);
            color correctedColorPrevious = correct_color_and_gamma(renderOutputSecondary[index], samplesPerPixel * (frameCount - 1));
            color diff = (correctedColorPrevious - correctedColor);

            renderDifference[index] = diff.length();
            renderDifferenceSum += renderDifference[index];

            diff *= differnceMult;


            Color color_v = {
                static_cast<unsigned char>(256 * clamp(correctedColor.x(), 0.0, 0.999)),
                static_cast<unsigned char>(256 * clamp(correctedColor.y(), 0.0, 0.999)),
                static_cast<unsigned char>(256 * clamp(correctedColor.z(), 0.0, 0.999)),
                static_cast<unsigned char>(255)
            };

            DrawPixel(x, y, color_v);

            color_v = {
                static_cast<unsigned char>(256 * clamp(correctedColorPrevious.x(), 0.0, 0.999)),
                static_cast<unsigned char>(256 * clamp(correctedColorPrevious.y(), 0.0, 0.999)),
                static_cast<unsigned char>(256 * clamp(correctedColorPrevious.z(), 0.0, 0.999)),
                static_cast<unsigned char>(255)
            };

            DrawPixel(x, y + renderHeight, color_v);

            color_v = {
                static_cast<unsigned char>(256 * clamp(diff.x(), 0.0, 0.999)),
                static_cast<unsigned char>(256 * clamp(diff.y(), 0.0, 0.999)),
                static_cast<unsigned char>(256 * clamp(diff.z(), 0.0, 0.999)),
                static_cast<unsigned char>(255)
            };

            DrawPixel(x, y + renderHeight + renderHeight, color_v);

            renderOutputSecondary[index] = renderOutputPrimay[index];
        }

        if (frameCount > 1) {
            double averageDifference = renderDifferenceSum / (maxRenderPixels);

            Tracelog::Debug("Average Pixel Difference: %lf", averageDifference);

            double minAverageDifference = 1;
            double maxAverageDifference = 0;

            if (chunkedAverageDifference) {
                
                for (int cy = 0; cy < chunksTall; cy++) {
                    for (int cx = 0; cx < chunksWide; cx++) {

                        int start_y = cy * chunkSize;
                        int end_y = std::min(renderHeight, (cy + 1) * chunkSize);

                        int start_x = cx * chunkSize;
                        int end_x = std::min(renderWidth, (cx + 1) * chunkSize);

                        int chunkWidth = end_x - start_x;
                        int chunkHeight = end_y - start_y;
                        int chunkPixelCount = chunkHeight * chunkWidth;

                        renderDifferenceSum = 0;

                        for (int y = start_y; y < end_y; y++) {
                            for (int x = start_x; x < end_x; x++) {
                                int index = (renderHeight - y) * renderWidth + x;

                                renderDifferenceSum += renderDifference[index];
                            }
                        }

                        averageDifference = renderDifferenceSum / (chunkPixelCount);

                        if (averageDifference >= 0 && averageDifference <= 1) {
                            if (averageDifference > maxAverageDifference) maxAverageDifference = averageDifference;
                            if (averageDifference < minAverageDifference) minAverageDifference = averageDifference;

                            averageDifference *= differnceMult;

                            Color averagePixelDifference = {
                            static_cast<unsigned char>(256 * clamp(averageDifference, 0.0, 0.999)),
                            static_cast<unsigned char>(256 * clamp(averageDifference, 0.0, 0.999)),
                            static_cast<unsigned char>(256 * clamp(averageDifference, 0.0, 0.999)),
                            static_cast<unsigned char>(255)
                            };

                            DrawRectangle(0 + start_x, renderHeight * 3 + start_y, chunkWidth, chunkHeight, averagePixelDifference);
                        }
                        else {
                            //DrawRectangle(0 + start_x, renderHeight * 3 + start_y, chunkWidth, chunkHeight, RED);
                        }


                        
                    }
                }
            }
            else {
                Color averagePixelDifference = {
                    static_cast<unsigned char>(256 * clamp(averageDifference, 0.0, 0.999)),
                    static_cast<unsigned char>(256 * clamp(averageDifference, 0.0, 0.999)),
                    static_cast<unsigned char>(256 * clamp(averageDifference, 0.0, 0.999)),
                    static_cast<unsigned char>(255)
                };

                DrawRectangle(0, renderHeight * 3, renderWidth, renderHeight, averagePixelDifference);
            }


            Tracelog::Debug("Min Average Pixel Difference: %lf", minAverageDifference);
            Tracelog::Debug("Max Average Pixel Difference: %lf", maxAverageDifference);
        }

        DrawText(TextFormat("Sample #%d", frameCount), 4, 4, 20, LIME);

        EndDrawing();


        if (progressive) {
            frameCount++;
            useSecondaryRenderOutput = !useSecondaryRenderOutput;
        }
    }

    free(renderOutputPrimay);
    free(renderOutputSecondary);
    free(renderDifference);
    //free(renderDifference);
    CloseWindow();

    return EXIT_SUCCESS;
}