#ifndef TERRAIN_TYPES_H
#define TERRAIN_TYPES_H

#include <functional> // For std::hash
#include <glm/glm.hpp> // For potential future use or if you prefer glm::ivec2

// Simple struct for 2D integer coordinates (e.g., for chunk grid)
struct Vec2i {
    int x, z;

    Vec2i(int x_val = 0, int z_val = 0) : x(x_val), z(z_val) {}

    // Equality operator for using Vec2i as a key in std::unordered_map
    bool operator==(const Vec2i& other) const {
        return x == other.x && z == other.z;
    }

    // Less-than operator for using Vec2i as a key in std::map (if needed)
    bool operator<(const Vec2i& other) const {
        if (x < other.x) return true;
        if (x > other.x) return false;
        return z < other.z;
    }
};

// Hash function for Vec2i to be used with std::unordered_map
namespace std {
    template <>
    struct hash<Vec2i> {
        size_t operator()(const Vec2i& v) const {
            // A common way to combine hashes
            size_t h1 = hash<int>()(v.x);
            size_t h2 = hash<int>()(v.z);
            return h1 ^ (h2 << 1); // Or use boost::hash_combine if available
        }
    };
}

#endif // TERRAIN_TYPES_H