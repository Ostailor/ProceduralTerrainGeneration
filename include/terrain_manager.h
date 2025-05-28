#ifndef TERRAIN_MANAGER_H
#define TERRAIN_MANAGER_H

#include "terrain_types.h" // For Vec2i
#include "terrain_chunk.h" // For Chunk class
#include "Camera.h"        
#include "PerlinNoise.h"   // Add this
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <memory>          // For std::unique_ptr
#include <cmath>           // For std::floor
#include <iostream>        // For debugging

// Forward declaration
class Shader;

class TerrainManager {
public:
    // The radius of chunks to load around the camera's current chunk.
    // A radius of 0 means only the camera's current chunk.
    // A radius of 1 means a 3x3 grid of chunks.
    // A radius of 2 means a 5x5 grid of chunks.
    int loadRadius;

private:
    // Stores currently active/loaded chunks, keyed by their grid coordinates.
    // Using unique_ptr to manage the lifetime of Chunk objects.
    std::unordered_map<Vec2i, std::unique_ptr<Chunk>> activeChunks_;

    // Keep track of the camera's last known chunk coordinates to avoid unnecessary updates
    Vec2i lastCameraChunkCoords_;
    bool firstUpdate_ = true;

    PerlinNoise perlinGenerator_; // Owns a PerlinNoise instance

public:
    // Constructor
    TerrainManager(int pLoadRadius);
    ~TerrainManager();

    // Main update function, called every frame.
    // Determines which chunks to load/unload based on camera position.
    void update(const Camera& camera);

    // Renders all currently active and loaded chunks.
    // (Implementation will be more detailed in a later step)
    void renderActiveChunks(Shader& terrainShader);

private:
    // Calculates the grid coordinates of the chunk the camera is currently in.
    Vec2i getCameraChunkCoordinates(const Camera& camera) const;

    // Manages loading a specific chunk.
    void loadChunk(Vec2i chunkCoords);

    // Manages unloading a specific chunk.
    void unloadChunk(Vec2i chunkCoords);
};

#endif // TERRAIN_MANAGER_H