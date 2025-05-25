#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#include <vector>
#include "PerlinNoise.h"

class HeightMap {
public:
    HeightMap(int width, int depth);
    ~HeightMap();

    void generateRandomHeights(float minHeight = 0.0f, float maxHeight = 10.0f);
    void generatePerlinHeights(const PerlinNoise& pn, 
                               float scale = 20.0f, 
                               int octaves = 4, 
                               float persistence = 0.5f, 
                               float overallMinHeight = 0.0f, 
                               float overallMaxHeight = 10.0f,
                               float peakExponent = 1.0f); // Added peakExponent
    void smoothHeights(int iterations = 1, int kernelSize = 1);

    float getHeight(int x, int z) const;
    int getWidth() const;
    int getDepth() const;

private:
    int width_;
    int depth_;
    std::vector<std::vector<float>> heights_;
};

#endif // HEIGHTMAP_H