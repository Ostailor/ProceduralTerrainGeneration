#ifndef MESH_H
#define MESH_H

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp> 
#include <limits> 

// Forward declare HeightMap if its full definition isn't needed in this header
class HeightMap; 

struct Vertex {
    glm::vec3 Position;     
    glm::vec3 Normal;       
    glm::vec2 TexCoords;    
};

struct BoundingBox { 
    glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());

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
    BoundingBox boundingBox; 

    Mesh();
    ~Mesh(); // <<< ADD THIS LINE (declare the destructor)

    void generateFromHeightMap(const HeightMap& heightMap, float horizontalScale, float verticalScale);
    void setupMesh();
    void draw() const; 
    void clearGPUData(); 

    size_t getVerticesCount() const;
    size_t getIndicesCount() const;

private:
    unsigned int VAO, VBO, EBO;
    void calculateNormals();
    void calculateBoundingBox(); 
};

#endif // MESH_H