#ifndef MESH_H
#define MESH_H

#include <vector>
#include <glad/glad.h>
#include "HeightMap.h"
#include <glm/glm.hpp> // For glm::vec3
#include <limits> // For std::numeric_limits

struct Vertex {
    glm::vec3 Position;     // Using glm::vec3 for convenience
    glm::vec3 Normal;       // Normal vector
    glm::vec2 TexCoords;    // Texture Coordinates
};

struct AABB {
    glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());

    // Optional: A method to get the 8 corners of the AABB
    std::vector<glm::vec3> getCorners() const {
        return {
            glm::vec3(min.x, min.y, min.z), glm::vec3(max.x, min.y, min.z),
            glm::vec3(min.x, max.y, min.z), glm::vec3(max.x, max.y, min.z),
            glm::vec3(min.x, min.y, max.z), glm::vec3(max.x, min.y, max.z),
            glm::vec3(min.x, max.y, max.z), glm::vec3(max.x, max.y, max.z)
        };
    }
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    unsigned int VAO;
    AABB boundingBox; // Add AABB member

    Mesh();
    ~Mesh();

    void generateFromHeightMap(const HeightMap& heightMap, float horizontalScale = 1.0f, float verticalScale = 1.0f);
    void setupMesh();
    void draw() const;

private:
    unsigned int VBO, EBO;
    void calculateNormals(); // Helper function to calculate normals
    void calculateBoundingBox(); // Helper to calculate AABB
};

#endif // MESH_H