#include "Frustum.h"
#include <vector> // For AABB corners

void Frustum::update(const glm::mat4& vpMatrix) {
    // Left Plane
    planes[0].normal.x = vpMatrix[0][3] + vpMatrix[0][0];
    planes[0].normal.y = vpMatrix[1][3] + vpMatrix[1][0];
    planes[0].normal.z = vpMatrix[2][3] + vpMatrix[2][0];
    planes[0].distance   = vpMatrix[3][3] + vpMatrix[3][0];

    // Right Plane
    planes[1].normal.x = vpMatrix[0][3] - vpMatrix[0][0];
    planes[1].normal.y = vpMatrix[1][3] - vpMatrix[1][0];
    planes[1].normal.z = vpMatrix[2][3] - vpMatrix[2][0];
    planes[1].distance   = vpMatrix[3][3] - vpMatrix[3][0];

    // Bottom Plane
    planes[2].normal.x = vpMatrix[0][3] + vpMatrix[0][1];
    planes[2].normal.y = vpMatrix[1][3] + vpMatrix[1][1];
    planes[2].normal.z = vpMatrix[2][3] + vpMatrix[2][1];
    planes[2].distance   = vpMatrix[3][3] + vpMatrix[3][1];

    // Top Plane
    planes[3].normal.x = vpMatrix[0][3] - vpMatrix[0][1];
    planes[3].normal.y = vpMatrix[1][3] - vpMatrix[1][1];
    planes[3].normal.z = vpMatrix[2][3] - vpMatrix[2][1];
    planes[3].distance   = vpMatrix[3][3] - vpMatrix[3][1];

    // Near Plane
    planes[4].normal.x = vpMatrix[0][3] + vpMatrix[0][2];
    planes[4].normal.y = vpMatrix[1][3] + vpMatrix[1][2];
    planes[4].normal.z = vpMatrix[2][3] + vpMatrix[2][2];
    planes[4].distance   = vpMatrix[3][3] + vpMatrix[3][2];

    // Far Plane
    planes[5].normal.x = vpMatrix[0][3] - vpMatrix[0][2];
    planes[5].normal.y = vpMatrix[1][3] - vpMatrix[1][2];
    planes[5].normal.z = vpMatrix[2][3] - vpMatrix[2][2];
    planes[5].distance   = vpMatrix[3][3] - vpMatrix[3][2];

    // Normalize all planes
    for (int i = 0; i < 6; ++i) {
        planes[i].normalize();
    }
}

// Check if an AABB is visible (AABB vs Frustum intersection test)
// This is a common conservative test: if all 8 corners of AABB are outside any single plane, it's culled.
// A more accurate test (sphere vs frustum, or separating axis theorem for AABB) can be used.
// For now, we use a simpler "point in frustum" style test for the AABB corners.
// A better AABB test checks if the AABB is on the "negative" side of any plane.
bool Frustum::isAABBVisible(const BoundingBox& bbox) const {
    // For each plane
    for (int i = 0; i < 6; ++i) {
        // Check if all 8 AABB corners are on the outside of this plane
        // If so, the AABB is outside the frustum
        // The "positive" or "p-vertex" is the corner furthest along the plane's normal
        // The "negative" or "n-vertex" is the corner furthest in the opposite direction of the plane's normal
        glm::vec3 pVertex = bbox.min;
        if (planes[i].normal.x >= 0) pVertex.x = bbox.max.x;
        if (planes[i].normal.y >= 0) pVertex.y = bbox.max.y;
        if (planes[i].normal.z >= 0) pVertex.z = bbox.max.z;

        // If p-vertex is outside this plane (on the negative side), then the whole AABB is outside
        if (planes[i].getSignedDistanceToPoint(pVertex) < 0) {
            return false; // AABB is outside this plane, so it's culled
        }
    }
    return true; // AABB is intersecting or inside all planes
}