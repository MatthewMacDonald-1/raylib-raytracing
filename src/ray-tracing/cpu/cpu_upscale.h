#include "color.h"
#include "math.h"
#include "rtweekend.h"

// static int modInverse(int A, int M)
// {
//     for (int X = 1; X < M; X++)
//         if (((A % M) * (X % M)) % M == 1)
//             return X;
// }

static int getIndexFromCoord(int x, int y, int width, int length) {
    // int index = clamp(y * width, 0, length);
    int index = clamp(x + (y * width), 0, length);
    return index;
}

/**
 * Calculates the new pixel color using linear interpolation
*/
static color calculatePixelColor(int destX, int destY, int sourceMaxPixels, color *source, int sourceWidth, int sourceHeight, int destWidth, int destHeight) {
    double relativeX = ((double)destX / (double)destWidth) * (double)sourceWidth;
    int sourceXLower = (int)floor(relativeX);
    int sourceXUpper = (int)ceil(relativeX);

    double relativeY = ((double)destY / (double)destHeight) * (double)sourceHeight;
    int sourceYLower = (int)floor(relativeY);
    int sourceYUpper = (int)ceil(relativeY);

    color avgColor = (source[getIndexFromCoord(sourceXLower, sourceYLower, sourceWidth, sourceMaxPixels)] + source[getIndexFromCoord(sourceXUpper, sourceYLower, sourceWidth, sourceMaxPixels)] + source[getIndexFromCoord(sourceXLower, sourceYUpper, sourceWidth, sourceMaxPixels)] + source[getIndexFromCoord(sourceXUpper, sourceYUpper, sourceWidth, sourceMaxPixels)]) / 4;

    return avgColor;
}

void upscaleImage(color *source, int sourceWidth, int sourceHeight, color *dest, int destWidth, int destHeight) {
    int maxPixels = destWidth * destHeight;
    int sourceMaxPixels = sourceWidth * sourceHeight;
    for (int index = 0; index < maxPixels; index++) {
            int x = index % destWidth;
            int y = index / destWidth;

            // Calculate pixel value from source
            
            dest[index] = calculatePixelColor(x, y, sourceMaxPixels, source, sourceWidth, sourceHeight, destWidth, destHeight);
        }
}