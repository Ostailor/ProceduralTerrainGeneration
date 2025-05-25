#ifndef FRUSTUM_H
#define FRUSTUM_H

#include <glm/glm.hpp>
#include "Mesh.h" // For AABB struct

// Defines a plane equation: normal.x*x + normal.y*y + normal.z*z + distance = 0
struct Plane {
    glm::vec3 normal = {0.f, 1.f, 0.f};
    float distance = 0.f; // This is the D coefficient in Ax + By + Cz + D = 0

    Plane() = default;

    // This constructor defines 'distance' as dot(normal, p1), so plane is N.X - distance = 0
    // However, Frustum::update directly sets 'distance' as D from N.X + D = 0.
    // We will ensure getSignedDistanceToPoint is consistent with N.X + D = 0.
    Plane(const glm::vec3& p1, const glm::vec3& norm)
        : normal(glm::normalize(norm)),
          distance(glm::dot(normal, p1)) // For N.X - d = 0, d = dot(normal,p1)
    {}

    void normalize() {
        float mag = glm::length(normal);
        if (mag > 0.00001f) {
            normal /= mag;
            distance /= mag; // D coefficient is also scaled
        }
    }

    // Distance from a point to the plane.
    // Assumes plane equation is normal.x*x + normal.y*y + normal.z*z + distance = 0
    // where 'normal' is (normal.x, normal.y, normal.z) and 'distance' is the D coefficient.
    // A positive result means the point is on the side of the plane the normal points to (inside for frustum).
    float getSignedDistanceToPoint(const glm::vec3& point) const {
        return glm::dot(normal, point) + distance; // Changed from '-' to '+'
    }
};

class Frustum {
public:
    Plane planes[6]; // 0:Left, 1:Right, 2:Bottom, 3:Top, 4:Near, 5:Far

    Frustum() = default;
    void update(const glm::mat4& viewProjectionMatrix);
    bool isAABBVisible(const AABB& aabb) const;
};

#endif // FRUSTUM_H