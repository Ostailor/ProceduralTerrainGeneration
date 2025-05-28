#include "terrain_chunk.h"
#include "Shader.h" 
#include <glm/gtc/matrix_transform.hpp> 
#include <cmath> // For std::pow, std::max, std::min

// Initialize static members for terrain generation
// These values should be similar to what you used for your single large terrain
float Chunk::CHUNK_WORLD_SIZE_X = 64.0f; 
float Chunk::CHUNK_WORLD_SIZE_Z = 64.0f;
int Chunk::CHUNK_VERTEX_RESOLUTION_X = 33; 
int Chunk::CHUNK_VERTEX_RESOLUTION_Z = 33;

// Initialize static terrain generation parameters
float Chunk::TERRAIN_SCALE = 60.0f; // Adjusted for potentially larger features across chunks
int   Chunk::TERRAIN_OCTAVES = 5;
float Chunk::TERRAIN_PERSISTENCE = 0.5f;
float Chunk::TERRAIN_MIN_HEIGHT = 0.0f;
float Chunk::TERRAIN_MAX_HEIGHT = 30.0f; // Max height of terrain features
float Chunk::TERRAIN_PEAK_EXPONENT = 1.2f;
float Chunk::MESH_VERTICAL_SCALE = 1.0f;  // Multiplier for the generated height values
// MESH_HORIZONTAL_SCALE will be calculated dynamically in load()

// Constructor now takes a PerlinNoise generator
Chunk::Chunk(Vec2i pGridCoords, const PerlinNoise* pNoiseGenerator) // Modified constructor
    : gridCoords(pGridCoords), 
      perlinGenerator_(pNoiseGenerator), // Store the generator
      isLoaded_(false), 
      isActive_(false) {
    
    worldPosition.x = static_cast<float>(gridCoords.x) * CHUNK_WORLD_SIZE_X;
    worldPosition.y = 0.0f; 
    worldPosition.z = static_cast<float>(gridCoords.z) * CHUNK_WORLD_SIZE_Z;

    calculateModelMatrix();

    std::cout << "Chunk created at grid (" << gridCoords.x << ", " << gridCoords.z 
              << ") world origin (" << worldPosition.x << ", " << worldPosition.z << ")" << std::endl;
    
    if (!perlinGenerator_) {
        std::cerr << "WARNING: Chunk (" << gridCoords.x << ", " << gridCoords.z 
                  << ") created with a NULL PerlinNoise generator!" << std::endl;
    }
}

Chunk::~Chunk() {
    unload(); 
    std::cout << "Chunk at grid (" << gridCoords.x << ", " << gridCoords.z << ") destroyed." << std::endl;
}

void Chunk::calculateModelMatrix() {
    float chunkCenterX = worldPosition.x + CHUNK_WORLD_SIZE_X / 2.0f;
    float chunkCenterZ = worldPosition.z + CHUNK_WORLD_SIZE_Z / 2.0f;
    modelMatrix_ = glm::translate(glm::mat4(1.0f), glm::vec3(chunkCenterX, 0.0f, chunkCenterZ));
}

void Chunk::load() {
    if (isLoaded_) {
        return;
    }
    if (!perlinGenerator_) {
        std::cerr << "ERROR: Cannot load Chunk (" << gridCoords.x << ", " << gridCoords.z 
                  << ") because PerlinNoise generator is null." << std::endl;
        return;
    }

    std::cout << "Chunk (" << gridCoords.x << ", " << gridCoords.z << "): Actual LOAD initiated." << std::endl;

    // Create a local HeightMap for this chunk
    // The HeightMap dimensions are the number of vertices
    HeightMap localHeightMap(CHUNK_VERTEX_RESOLUTION_X, CHUNK_VERTEX_RESOLUTION_Z);

    // Calculate horizontal scaling for the mesh generation.
    // This is the distance between vertices in the heightmap/mesh.
    // Mesh::generateFromHeightMap uses one 'horizontalScale'.
    // Ensure CHUNK_WORLD_SIZE / (CHUNK_VERTEX_RESOLUTION - 1) is consistent.
    // If CHUNK_VERTEX_RESOLUTION_X and _Z differ, this might need adjustment
    // or your Mesh::generateFromHeightMap might need separate X and Z scales.
    // Assuming square cells for now.
    float MESH_HORIZONTAL_SCALE = CHUNK_WORLD_SIZE_X / static_cast<float>(CHUNK_VERTEX_RESOLUTION_X - 1);


    for (int z_idx = 0; z_idx < CHUNK_VERTEX_RESOLUTION_Z; ++z_idx) {
        for (int x_idx = 0; x_idx < CHUNK_VERTEX_RESOLUTION_X; ++x_idx) {
            // Calculate the world coordinates for this specific vertex in the chunk
            // worldPosition is the origin (min corner) of the chunk.
            double vertexWorldX = static_cast<double>(worldPosition.x) + 
                                  static_cast<double>(x_idx) * MESH_HORIZONTAL_SCALE;
            double vertexWorldZ = static_cast<double>(worldPosition.z) + 
                                  static_cast<double>(z_idx) * MESH_HORIZONTAL_SCALE; // Use same scale for Z

            // Scale world coordinates for Perlin noise input
            // Using the static TERRAIN_SCALE for consistent feature size across chunks
            double noiseSampleX = vertexWorldX / TERRAIN_SCALE;
            double noiseSampleZ = vertexWorldZ / TERRAIN_SCALE;

            // Generate Perlin noise value (typically 0.0 to 1.0 from your PerlinNoise class)
            double perlinValue = perlinGenerator_->octaveNoise(noiseSampleX, noiseSampleZ, 
                                                              TERRAIN_OCTAVES, TERRAIN_PERSISTENCE);

            // Apply peak exponent
            if (TERRAIN_PEAK_EXPONENT != 1.0f && TERRAIN_PEAK_EXPONENT > 0.0f) {
                perlinValue = std::pow(perlinValue, static_cast<double>(TERRAIN_PEAK_EXPONENT));
            }
            // Clamp perlinValue to [0, 1] just in case octaveNoise goes slightly out of bounds
            perlinValue = std::max(0.0, std::min(1.0, perlinValue));


            // Map Perlin value to height range
            float finalHeight = static_cast<float>(TERRAIN_MIN_HEIGHT + perlinValue * (TERRAIN_MAX_HEIGHT - TERRAIN_MIN_HEIGHT));
            
            localHeightMap.setHeight(x_idx, z_idx, finalHeight);
        }
    }
    
    // Optional: Smooth the generated heightmap for this chunk if desired
    // localHeightMap.smoothHeights(1, 1); // Example: 1 iteration, 3x3 kernel

    // Generate mesh using the local heightmap
    // The MESH_HORIZONTAL_SCALE determines the spacing of vertices in the XZ plane.
    // The MESH_VERTICAL_SCALE multiplies the height values.
    mesh_.generateFromHeightMap(localHeightMap, MESH_HORIZONTAL_SCALE, MESH_VERTICAL_SCALE);
    mesh_.setupMesh(); // Creates VAO, VBO, EBO

    isLoaded_ = true;
    isActive_ = true; // Mark as active for rendering once loaded
    std::cout << "Chunk (" << gridCoords.x << ", " << gridCoords.z << ") LOADED. Mesh generated: "
              << mesh_.getVerticesCount() << " vertices, " << mesh_.getIndicesCount() << " indices." << std::endl;
}

void Chunk::unload() {
    if (!isLoaded_) {
        return;
    }
    std::cout << "Chunk (" << gridCoords.x << ", " << gridCoords.z << "): UNLOAD initiated." << std::endl;
    
    // The Mesh object (mesh_) is a member. Its destructor will be called when the Chunk
    // object is destroyed, and that destructor should handle glDeleteVertexArrays, glDeleteBuffers.
    // If you need to free GPU resources *without* destroying the Chunk object itself
    // (e.g., for a more complex caching system), your Mesh class would need a
    // specific method like `clearGPUData()` or `destroyBuffers()`.
    // For now, relying on Mesh destructor is fine as unique_ptr in TerrainManager deletes Chunks.
    
    // mesh_.clearGPUData(); // If Mesh had such a method for explicit cleanup.

    isLoaded_ = false;
    isActive_ = false;
    std::cout << "Chunk (" << gridCoords.x << ", " << gridCoords.z << ") UNLOADED." << std::endl;
}

void Chunk::render(Shader& shader) {
    if (!isLoaded_ || !isActive_) { // isActive_ is set in load() and unset in unload()
        return;
    }

    // The shader should already be in use (shader.use() called by TerrainManager or main)
    // View and projection matrices should also be set globally on the shader.
    
    // Set the model matrix specific to this chunk
    shader.setMat4("model", modelMatrix_);

    // Draw the chunk's mesh
    mesh_.draw();
}
