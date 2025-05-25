#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h" 
#include "Texture.h"
#include <iostream>

unsigned int loadTexture(const char* path, bool flipVertically) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    if (flipVertically) {
        stbi_set_flip_vertically_on_load(true);
    } else {
        stbi_set_flip_vertically_on_load(false);
    }
    
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        else {
            std::cerr << "Texture failed to load at path: " << path << " (unsupported number of components: " << nrComponents << ")" << std::endl;
            stbi_image_free(data);
            glDeleteTextures(1, &textureID); 
            return 0;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cerr << "Texture failed to load at path: " << path << std::endl;
        glDeleteTextures(1, &textureID); 
        return 0;
    }
    stbi_set_flip_vertically_on_load(false); 
    return textureID;
}

unsigned int loadCubemap(const std::vector<std::string>& faces, bool flipVerticallyCubemap) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    stbi_set_flip_vertically_on_load(flipVerticallyCubemap);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            GLenum format;
            if (nrChannels == 1) format = GL_RED;
            else if (nrChannels == 3) format = GL_RGB;
            else if (nrChannels == 4) format = GL_RGBA;
            else {
                 std::cerr << "Cubemap texture failed to load at path: " << faces[i] << " (unsupported number of components: " << nrChannels << ")" << std::endl;
                 stbi_image_free(data);
                 glDeleteTextures(1, &textureID);
                 stbi_set_flip_vertically_on_load(false); // Reset
                 return 0;
            }
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                         0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cerr << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data); // data is null here
            glDeleteTextures(1, &textureID);
            stbi_set_flip_vertically_on_load(false); // Reset
            return 0;
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    stbi_set_flip_vertically_on_load(false); // Reset flip state
    return textureID;
}