#ifndef PERLINNOISE_H
#define PERLINNOISE_H

#include <vector>
#include <random>    // For std::mt19937, std::uniform_real_distribution, std::random_device
#include <numeric>   // For std::iota
#include <algorithm> // For std::shuffle

class PerlinNoise {
public:
    PerlinNoise(); // Initialize with a random seed
    PerlinNoise(unsigned int seed); // Initialize with a specific seed

    double noise(double x, double y) const;
    // Optional: 3D noise
    // double noise(double x, double y, double z) const;

    // Fractional Brownian Motion (fBm) for more detailed noise
    double octaveNoise(double x, double y, int octaves, double persistence) const;


private:
    std::vector<int> p; // Permutation vector

    // Helper functions
    double fade(double t) const;
    double lerp(double t, double a, double b) const;
    double grad(int hash, double x, double y) const;
    // double grad(int hash, double x, double y, double z) const; // For 3D
};

#endif // PERLINNOISE_H