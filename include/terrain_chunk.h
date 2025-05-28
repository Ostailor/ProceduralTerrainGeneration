#ifndef TERRAIN_CHUNK_H
#define TERRAIN_CHUNK_H

#include "terrain_types.h" // For Vec2i
#include "Mesh.h"          // We will need this later
#include "PerlinNoise.h"   // Add this
#include "HeightMap.h"     // Add this
#include <glm/glm.hpp>
#include <string>
#include <iostream>        // For debugging output

// Forward declaration of Shader, if Chunk's render method will take it
class Shader; 

class Chunk {
public:
    const Vec2i gridCoords; // The (X, Z) coordinate of this chunk in the world grid
    
    // World position of the chunk's origin (e.g., bottom-left corner)
    // This will be calculated based on gridCoords and CHUNK_WORLD_SIZE
    glm::vec3 worldPosition; 

    // Dimensions of the chunk in world units
    // These will likely be constants defined in TerrainManager or globally
    static float CHUNK_WORLD_SIZE_X; 
    static float CHUNK_WORLD_SIZE_Z;

    // Resolution of the chunk's mesh (number of vertices along each axis)
    // e.g., 33 vertices for 32 segments
    static int CHUNK_VERTEX_RESOLUTION_X;
    static int CHUNK_VERTEX_RESOLUTION_Z;

    // Terrain generation parameters (can be static if all chunks share them,
    // or passed in constructor/load method if they can vary per chunk type)
    static float TERRAIN_SCALE;          // Overall scale for Perlin noise sampling
    static int   TERRAIN_OCTAVES;
    static float TERRAIN_PERSISTENCE;
    static float TERRAIN_MIN_HEIGHT;
    static float TERRAIN_MAX_HEIGHT;
    static float TERRAIN_PEAK_EXPONENT;
    static float MESH_VERTICAL_SCALE;    // Vertical scaling factor for the mesh
    static float MESH_HORIZONTAL_SCALE;  // Calculated from world size and resolution

private:
    // Mesh object for this chunk's terrain.
    // Will be default-constructed initially, then generated in load().
    Mesh mesh_; 
    bool isLoaded_;
    bool isActive_; // Could be used to mark for rendering vs. just loaded

    glm::mat4 modelMatrix_; // To position this chunk in the world

    const PerlinNoise* perlinGenerator_; // Pointer to a PerlinNoise instance

public:
    // Constructor now takes a PerlinNoise generator
    Chunk(Vec2i pGridCoords, const PerlinNoise* pNoiseGenerator);
    ~Chunk();

    // Core methods (to be implemented in later steps)
    void load();    // Generate or load heightmap, create mesh, setup GPU buffers
    void unload();  // Free GPU buffers and potentially other mesh data
    
    // Render this chunk (implementation in a later step)
    void render(Shader& shader); 

    bool isLoaded() const { return isLoaded_; }
    // const Mesh& getMesh() const { return mesh_; } // If needed for external access

    // Method to calculate and set the model matrix
    void calculateModelMatrix();
};

#endif // TERRAIN_CHUNK_H