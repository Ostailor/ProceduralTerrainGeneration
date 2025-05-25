#include "PerlinNoise.h"
#include <cmath>       // For floor, fmod
#include <numeric>     // For std::iota
#include <algorithm>   // For std::shuffle, std::min, std::max
#include <stdexcept>   // For std::runtime_error

PerlinNoise::PerlinNoise() {
    // Initialize with a random seed
    std::random_device rd;
    std::mt19937 generator(rd());
    
    p.resize(256);
    std::iota(p.begin(), p.end(), 0); // Fill p with 0-255
    std::shuffle(p.begin(), p.end(), generator); // Shuffle it
    p.insert(p.end(), p.begin(), p.end()); // Duplicate the permutation vector
}

PerlinNoise::PerlinNoise(unsigned int seed) {
    p.resize(256);
    std::iota(p.begin(), p.end(), 0); // Fill p with 0-255

    std::mt19937 generator(seed);
    std::shuffle(p.begin(), p.end(), generator); // Shuffle it based on seed
    p.insert(p.end(), p.begin(), p.end()); // Duplicate the permutation vector to avoid buffer overflows
}

double PerlinNoise::fade(double t) const {
    return t * t * t * (t * (t * 6 - 15) + 10); // 6t^5 - 15t^4 + 10t^3
}

double PerlinNoise::lerp(double t, double a, double b) const {
    return a + t * (b - a);
}

double PerlinNoise::grad(int hash, double x, double y) const {
    // Convert low 2 bits of hash code into 4 gradient directions
    int h = hash & 3;       
    double u = (h < 2) ? x : y;
    double v = (h < 2) ? y : x;
    return ((h & 1) ? -u : u) + ((h & 2) ? -2.0*v : 2.0*v); // Fixed gradient directions
}

double PerlinNoise::noise(double x, double y) const {
    // Find unit cube that contains point
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;

    // Find relative x,y,z of point in cube
    x -= std::floor(x);
    y -= std::floor(y);

    // Compute fade curves for each of x,y,z
    double u = fade(x);
    double v = fade(y);

    // Hash coordinates of the 8 cube corners
    int A = p[X] + Y;
    int AA = p[A];
    int AB = p[A + 1];
    int B = p[X + 1] + Y;
    int BA = p[B];
    int BB = p[B + 1];

    // Add blended results from 4 corners of square
    double res = lerp(v, lerp(u, grad(p[AA], x, y),
                                 grad(p[BA], x - 1, y)),
                         lerp(u, grad(p[AB], x, y - 1),
                                 grad(p[BB], x - 1, y - 1)));
    return (res + 1.0) / 2.0; // To bring to 0.0 - 1.0 range
}

double PerlinNoise::octaveNoise(double x, double y, int octaves, double persistence) const {
    double total = 0;
    double frequency = 1;
    double amplitude = 1;
    double maxValue = 0;  // Used for normalizing result to 0.0 - 1.0

    for(int i = 0; i < octaves; i++) {
        total += noise(x * frequency, y * frequency) * amplitude;
        
        maxValue += amplitude;
        
        amplitude *= persistence;
        frequency *= 2;
    }
    
    if (maxValue == 0) return 0; // Avoid division by zero
    return total / maxValue;
}

/*
// Optional 3D noise and grad function
double PerlinNoise::grad(int hash, double x, double y, double z) const {
    int h = hash & 15;      // Convert low 4 bits of hash code into 12 gradient directions
    double u = h < 8 ? x : y;
    double v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

double PerlinNoise::noise(double x, double y, double z) const {
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;
    int Z = static_cast<int>(std::floor(z)) & 255;

    x -= std::floor(x);
    y -= std::floor(y);
    z -= std::floor(z);

    double u = fade(x);
    double v = fade(y);
    double w = fade(z);

    int A = p[X] + Y;
    int AA = p[A] + Z;
    int AB = p[A + 1] + Z;
    int B = p[X + 1] + Y;
    int BA = p[B] + Z;
    int BB = p[B + 1] + Z;

    double res = lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z),
                                     grad(p[BA], x - 1, y, z)),
                             lerp(u, grad(p[AB], x, y - 1, z),
                                     grad(p[BB], x - 1, y - 1, z))),
                     lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1),
                                     grad(p[BA + 1], x - 1, y, z - 1)),
                             lerp(u, grad(p[AB + 1], x, y - 1, z - 1),
                                     grad(p[BB + 1], x - 1, y - 1, z - 1))));
    return (res + 1.0) / 2.0; // To bring to 0.0 - 1.0 range
}
*/