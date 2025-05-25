#include "HeightMap.h"
#include <cstdlib>
#include <ctime>
#include <stdexcept>
#include <algorithm> 
#include <cmath> // For std::pow

HeightMap::HeightMap(int width, int depth) : width_(width), depth_(depth) {
    if (width <= 0 || depth <= 0) {
        throw std::invalid_argument("HeightMap dimensions must be positive.");
    }
    heights_.resize(width, std::vector<float>(depth, 0.0f));
    srand(static_cast<unsigned int>(time(nullptr)));
}

HeightMap::~HeightMap() {
}

void HeightMap::generateRandomHeights(float minHeight, float maxHeight) {
    if (minHeight >= maxHeight) {
        return; 
    }
    for (int x = 0; x < width_; ++x) {
        for (int z = 0; z < depth_; ++z) {
            float randomValue = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            heights_[x][z] = minHeight + randomValue * (maxHeight - minHeight);
        }
    }
}

void HeightMap::generatePerlinHeights(const PerlinNoise& pn, 
                                    float scale, 
                                    int octaves, 
                                    float persistence, 
                                    float overallMinHeight, 
                                    float overallMaxHeight,
                                    float peakExponent) { // Added peakExponent
    if (width_ <= 0 || depth_ <= 0) return;
    if (scale <= 0.0f) scale = 0.001f; 

    for (int x = 0; x < width_; ++x) {
        for (int z = 0; z < depth_; ++z) {
            double nx = static_cast<double>(x) / static_cast<double>(width_) * scale;
            double nz = static_cast<double>(z) / static_cast<double>(depth_) * scale;

            double perlinValue = pn.octaveNoise(nx, nz, octaves, persistence); // This is 0.0 to 1.0
            
            // Apply exponent to accentuate peaks
            if (peakExponent != 1.0f && peakExponent > 0.0f) { // Avoid pow(0, non-positive) or no change
                perlinValue = std::pow(perlinValue, static_cast<double>(peakExponent));
            }
            // Ensure perlinValue is still clamped if pow somehow pushes it out (unlikely for exponent > 0 and base in [0,1])
            perlinValue = std::max(0.0, std::min(1.0, perlinValue));


            heights_[x][z] = static_cast<float>(overallMinHeight + perlinValue * (overallMaxHeight - overallMinHeight));
        }
    }
}

void HeightMap::smoothHeights(int iterations, int kernelSize) {
    if (width_ <= 0 || depth_ <= 0 || kernelSize < 0) return;

    for (int iter = 0; iter < iterations; ++iter) {
        std::vector<std::vector<float>> smoothedHeights = heights_; 

        for (int r = 0; r < width_; ++r) { // Renamed x to r to avoid conflict
            for (int c = 0; c < depth_; ++c) { // Renamed z to c to avoid conflict
                float sum = 0.0f;
                int count = 0;
                for (int i = -kernelSize; i <= kernelSize; ++i) {
                    for (int j = -kernelSize; j <= kernelSize; ++j) {
                        int nx = r + i;
                        int nz = c + j;
                        if (nx >= 0 && nx < width_ && nz >= 0 && nz < depth_) {
                            sum += heights_[nx][nz]; 
                            count++;
                        }
                    }
                }
                if (count > 0) {
                    smoothedHeights[r][c] = sum / count;
                }
            }
        }
        heights_ = smoothedHeights; 
    }
}

float HeightMap::getHeight(int x, int z) const {
    if (x < 0 || x >= width_ || z < 0 || z >= depth_) {
        throw std::out_of_range("HeightMap coordinates out of bounds.");
    }
    return heights_[x][z];
}

int HeightMap::getWidth() const {
    return width_;
}

int HeightMap::getDepth() const {
    return depth_;
}