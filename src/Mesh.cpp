#include "Mesh.h"
#include "HeightMap.h" // <<< ADD THIS LINE
#include <glad/glad.h> // For OpenGL functions
#include <iostream>
#include <glm/geometric.hpp>
#include <limits> // For std::numeric_limits

Mesh::Mesh() : VAO(0), VBO(0), EBO(0) {}

Mesh::~Mesh() {
    clearGPUData();
}

void Mesh::generateFromHeightMap(const HeightMap& heightMap, float horizontalScale, float verticalScale) {
    vertices.clear();
    indices.clear();

    // Now the compiler will know what heightMap.getWidth(), .getDepth(), .getHeight() are
    int mapWidth = heightMap.getWidth();
    int mapDepth = heightMap.getDepth();

    if (mapWidth <= 0 || mapDepth <= 0) {
        std::cerr << "Error: HeightMap dimensions are invalid for mesh generation." << std::endl;
        return;
    }

    for (int z_coord = 0; z_coord < mapDepth; ++z_coord) {
        for (int x_coord = 0; x_coord < mapWidth; ++x_coord) {
            Vertex vertex;
            vertex.Position.x = static_cast<float>(x_coord) * horizontalScale - (static_cast<float>(mapWidth) * horizontalScale / 2.0f);
            vertex.Position.y = heightMap.getHeight(x_coord, z_coord) * verticalScale;
            vertex.Position.z = static_cast<float>(z_coord) * horizontalScale - (static_cast<float>(mapDepth) * horizontalScale / 2.0f);
            
            vertex.TexCoords.x = static_cast<float>(x_coord) / static_cast<float>(mapWidth > 1 ? mapWidth - 1 : 1); // Avoid division by zero
            vertex.TexCoords.y = static_cast<float>(z_coord) / static_cast<float>(mapDepth > 1 ? mapDepth - 1 : 1); // Avoid division by zero
            
            vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f); 
            vertices.push_back(vertex);
        }
    }

    for (int z_coord = 0; z_coord < mapDepth - 1; ++z_coord) { 
        for (int x_coord = 0; x_coord < mapWidth - 1; ++x_coord) { 
            unsigned int topLeft = z_coord * mapWidth + x_coord;
            unsigned int topRight = topLeft + 1;
            unsigned int bottomLeft = (z_coord + 1) * mapWidth + x_coord;
            unsigned int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
    
    calculateNormals();
    calculateBoundingBox(); // Call after vertices are generated

    std::cout << "Mesh generated: " << vertices.size() << " vertices, " << indices.size() << " indices." << std::endl;
    std::cout << "Mesh AABB Min: (" << boundingBox.min.x << ", " << boundingBox.min.y << ", " << boundingBox.min.z << ")" << std::endl;
    std::cout << "Mesh AABB Max: (" << boundingBox.max.x << ", " << boundingBox.max.y << ", " << boundingBox.max.z << ")" << std::endl;
}

void Mesh::calculateBoundingBox() {
    if (vertices.empty()) return;

    boundingBox.min = glm::vec3(std::numeric_limits<float>::max());
    boundingBox.max = glm::vec3(std::numeric_limits<float>::lowest());

    for (const auto& vertex : vertices) {
        boundingBox.min.x = std::min(boundingBox.min.x, vertex.Position.x);
        boundingBox.min.y = std::min(boundingBox.min.y, vertex.Position.y);
        boundingBox.min.z = std::min(boundingBox.min.z, vertex.Position.z);

        boundingBox.max.x = std::max(boundingBox.max.x, vertex.Position.x);
        boundingBox.max.y = std::max(boundingBox.max.y, vertex.Position.y);
        boundingBox.max.z = std::max(boundingBox.max.z, vertex.Position.z);
    }
}

void Mesh::calculateNormals() {
    for (size_t i = 0; i < vertices.size(); ++i) {
        vertices[i].Normal = glm::vec3(0.0f, 0.0f, 0.0f);
    }
    for (size_t i = 0; i < indices.size(); i += 3) {
        Vertex& v1 = vertices[indices[i]];
        Vertex& v2 = vertices[indices[i + 1]];
        Vertex& v3 = vertices[indices[i + 2]];
        glm::vec3 edge1 = v2.Position - v1.Position;
        glm::vec3 edge2 = v3.Position - v1.Position;
        glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));
        v1.Normal += faceNormal;
        v2.Normal += faceNormal;
        v3.Normal += faceNormal;
    }
    for (size_t i = 0; i < vertices.size(); ++i) {
        vertices[i].Normal = glm::normalize(vertices[i].Normal);
    }
}

void Mesh::setupMesh() {
    if (vertices.empty() || indices.empty()) {
        std::cerr << "Mesh::setupMesh() called with no vertex or index data." << std::endl;
        return;
    }
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    glEnableVertexAttribArray(2); 
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    glBindVertexArray(0);
}

void Mesh::draw() const {
    if (VAO == 0) {
        // std::cerr << "Mesh::draw() called but VAO is not set up." << std::endl; // Optional debug
        return;
    }
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

size_t Mesh::getVerticesCount() const {
    return vertices.size();
}

size_t Mesh::getIndicesCount() const {
    return indices.size();
}

void Mesh::clearGPUData() {
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0; // Reset to 0 to indicate it's no longer valid
    }
    if (VBO != 0) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
    if (EBO != 0) {
        glDeleteBuffers(1, &EBO);
        EBO = 0;
    }
    // Optionally clear CPU-side data if desired
    // vertices.clear();
    // vertices.shrink_to_fit();
    // indices.clear();
    // indices.shrink_to_fit();
    // std::cout << "Mesh GPU data cleared." << std::endl; // Optional debug
}

// Ensure calculateNormals and calculateBoundingBox are implemented if generateFromHeightMap calls them.
// void Mesh::calculateNormals() { /* ... your implementation ... */ }
// void Mesh::calculateBoundingBox() { /* ... your implementation ... */ }