#include "terrain_manager.h"
#include "Shader.h" 

TerrainManager::TerrainManager(int pLoadRadius)
    : loadRadius(pLoadRadius), lastCameraChunkCoords_(-9999, -9999), firstUpdate_(true) {
    // perlinGenerator_ is default constructed here
    std::cout << "TerrainManager created with load radius: " << loadRadius << std::endl;
}

TerrainManager::~TerrainManager() {
    // unique_ptr will automatically delete the Chunk objects,
    // which in turn will call their destructors (and their unload methods).
    activeChunks_.clear();
    std::cout << "TerrainManager destroyed, all active chunks cleared." << std::endl;
}

Vec2i TerrainManager::getCameraChunkCoordinates(const Camera& camera) const {
    // Uses the static CHUNK_WORLD_SIZE from the Chunk class.
    // Ensure camera.Position is the correct member for your Camera class.
    int camChunkX = static_cast<int>(std::floor(camera.Position.x / Chunk::CHUNK_WORLD_SIZE_X));
    int camChunkZ = static_cast<int>(std::floor(camera.Position.z / Chunk::CHUNK_WORLD_SIZE_Z));
    return Vec2i(camChunkX, camChunkZ);
}

void TerrainManager::update(const Camera& camera) {
    Vec2i currentCameraChunkCoords = getCameraChunkCoordinates(camera);

    if (currentCameraChunkCoords == lastCameraChunkCoords_ && !firstUpdate_) {
        return; // Camera hasn't moved to a new chunk, no update needed
    }
    lastCameraChunkCoords_ = currentCameraChunkCoords;
    firstUpdate_ = false;

    std::cout << "TerrainManager update: Camera in chunk (" << currentCameraChunkCoords.x << ", " << currentCameraChunkCoords.z << ")" << std::endl;

    // Determine the set of chunks that *should* be active
    std::vector<Vec2i> desiredActiveChunks;
    desiredActiveChunks.reserve((2 * loadRadius + 1) * (2 * loadRadius + 1));

    for (int dz = -loadRadius; dz <= loadRadius; ++dz) {
        for (int dx = -loadRadius; dx <= loadRadius; ++dx) {
            desiredActiveChunks.push_back(Vec2i(currentCameraChunkCoords.x + dx, currentCameraChunkCoords.z + dz));
        }
    }

    // Step 1: Unload chunks that are no longer in the desired set
    std::vector<Vec2i> chunksToUnloadKeys;
    for (const auto& pair : activeChunks_) {
        const Vec2i& loadedChunkCoord = pair.first;
        bool foundInDesired = false;
        for (const auto& desiredCoord : desiredActiveChunks) {
            if (loadedChunkCoord == desiredCoord) {
                foundInDesired = true;
                break;
            }
        }
        if (!foundInDesired) {
            chunksToUnloadKeys.push_back(loadedChunkCoord);
        }
    }

    for (const auto& key : chunksToUnloadKeys) {
        unloadChunk(key);
    }

    // Step 2: Load chunks that are in the desired set but not yet active
    for (const auto& desiredCoord : desiredActiveChunks) {
        if (activeChunks_.find(desiredCoord) == activeChunks_.end()) {
            loadChunk(desiredCoord);
        }
    }
}

void TerrainManager::loadChunk(Vec2i chunkCoords) {
    if (activeChunks_.count(chunkCoords)) {
        return; 
    }

    std::cout << "TerrainManager: Requesting load for chunk (" << chunkCoords.x << ", " << chunkCoords.z << ")" << std::endl;
    // Pass the address of the TerrainManager's perlinGenerator_ to the Chunk constructor
    auto newChunk = std::make_unique<Chunk>(chunkCoords, &perlinGenerator_); 
    
    // The actual mesh generation now happens inside newChunk->load()
    newChunk->load(); // Call the actual load method which generates the mesh
    
    activeChunks_[chunkCoords] = std::move(newChunk);
}

void TerrainManager::unloadChunk(Vec2i chunkCoords) {
    auto it = activeChunks_.find(chunkCoords);
    if (it != activeChunks_.end()) {
        std::cout << "TerrainManager: Requesting unload for chunk (" << chunkCoords.x << ", " << chunkCoords.z << ")" << std::endl;
        // The Chunk's unload() method will be called by its destructor when the unique_ptr is reset.
        // If Chunk::unload() needs to be called explicitly before destruction for some reason:
        // if (it->second) {
        //     it->second->unload(); 
        // }
        activeChunks_.erase(it); // This erases the unique_ptr, deleting the Chunk
    }
}

void TerrainManager::renderActiveChunks(Shader& terrainShader) {
    // The terrainShader should already be in use (shader.use())
    // and have global uniforms like view, projection, lighting, fog, textures set by main.cpp.

    for (const auto& pair : activeChunks_) {
        Chunk* chunk = pair.second.get(); // Get raw pointer from unique_ptr
        if (chunk && chunk->isLoaded()) { // Check if chunk exists and is loaded
            chunk->render(terrainShader);
        }
    }
}