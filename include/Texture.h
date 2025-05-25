// filepath: include/Texture.h
#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <string>
#include <vector> // For vector of strings

// Function to load a 2D texture from file
// Returns the texture ID, or 0 if loading failed
unsigned int loadTexture(const char* path, bool flipVertically = true);

// Function to load a cubemap texture from 6 individual texture faces
// faces should be a vector of 6 strings: right, left, top, bottom, front, back
unsigned int loadCubemap(const std::vector<std::string>& faces, bool flipVertically = false);

#endif // TEXTURE_H