#include <iostream>
#include <vector>
#include <string>
#include <limits> // For Mesh.h AABB initialization

// GLAD (must be included before GLFW)
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "HeightMap.h"
#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"
#include "PerlinNoise.h"
#include "Texture.h"
#include "Frustum.h" // Include Frustum
#include "terrain_manager.h" // Manages terrain chunks
#include "terrain_chunk.h"   // For Chunk static members if needed directly

// Window dimensions (use unsigned int for consistency with GLFW getframebuffersize)
unsigned int SCR_WIDTH = 1280; // Increased resolution for better detail
unsigned int SCR_HEIGHT = 720;

// Camera
Camera camera(glm::vec3(0.0f, 25.0f, 40.0f)); // Adjusted initial camera position
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f;

glm::vec3 currentFogColor = glm::vec3(0.53f, 0.81f, 0.92f); // Global for clear color

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed!" << std::endl;
        return 1;
    }

    // Set GLFW window hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on macOS
    #endif

    // Create a window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Terrain", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initialize GLAD: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    // Optional: Enable wireframe mode to see the triangles
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 


    // Build and compile our shader program
    Shader terrainShader("../shaders/basic.vert", "../shaders/basic.frag");
    if (terrainShader.getID() == 0) { std::cerr << "Failed to load shaders." << std::endl; glfwTerminate(); return -1; }
    
    Shader skyboxShader("../shaders/skybox.vert", "../shaders/skybox.frag");
    if (skyboxShader.getID() == 0) { std::cerr << "Failed to load skybox shaders." << std::endl; glfwTerminate(); return -1; }


    // Load textures
    unsigned int grassTexture = loadTexture("../textures/Grass001_1K-JPG/Grass001_1K-JPG_Color.jpg");
    unsigned int rockTexture  = loadTexture("../textures/Rock061_1K-JPG/Rock061_1K-JPG_Color.jpg");
    unsigned int snowTexture  = loadTexture("../textures/Snow010A_1K-JPG/Snow010A_1K-JPG_Color.jpg");

    if (grassTexture == 0 || rockTexture == 0 || snowTexture == 0) {
        std::cerr << "Failed to load one or more terrain textures." << std::endl;
    }

    // Skybox vertices (a simple cube)
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f
    };

    // Skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0); // Unbind skyboxVAO

    // Load skybox cubemap
    std::vector<std::string> faces {
        "../textures/skybox/space_rt.png", "../textures/skybox/space_lf.png",
        "../textures/skybox/space_up.png", "../textures/skybox/space_dn.png",
        "../textures/skybox/space_ft.png", "../textures/skybox/space_bk.png"
    };
    unsigned int cubemapTexture = loadCubemap(faces);
    if (cubemapTexture == 0) {
        std::cerr << "Failed to load cubemap texture. Check paths and filenames in the 'faces' vector." << std::endl;
    }

    // --- Terrain Manager Setup ---
    Chunk::CHUNK_WORLD_SIZE_X = 64.0f; 
    Chunk::CHUNK_WORLD_SIZE_Z = 64.0f;
    Chunk::CHUNK_VERTEX_RESOLUTION_X = 33; 
    Chunk::CHUNK_VERTEX_RESOLUTION_Z = 33;

    int loadRadius = 2; 
    TerrainManager terrainManager(loadRadius);
    // --- End Terrain Manager Setup ---

    Frustum cameraFrustum; // Create a Frustum object

    // Set texture units for shaders (once is enough)
    terrainShader.use();
    terrainShader.setInt("textureGrass", 0);
    terrainShader.setInt("textureRock", 1);
    terrainShader.setInt("textureSnow", 2);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    // unsigned int framesRendered = 0; // For simple culling feedback

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // --- Update ---
        terrainManager.update(camera); // Update terrain based on camera position

        glClearColor(currentFogColor.r, currentFogColor.g, currentFogColor.b, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        // glm::mat4 viewProjectionMatrix = projection * view; // Already calculated if needed for frustum
        // cameraFrustum.update(viewProjectionMatrix); // Frustum update is fine here

        // --- Render Terrain Chunks ---
        terrainShader.use();
        terrainShader.setMat4("projection", projection);
        terrainShader.setMat4("view", view);
        
        // Texturing uniforms (already here, good)
        terrainShader.setFloat("heightRockStart", Chunk::TERRAIN_MAX_HEIGHT * 0.35f); // Use Chunk's static max height
        terrainShader.setFloat("heightRockFull",  Chunk::TERRAIN_MAX_HEIGHT * 0.55f);
        terrainShader.setFloat("heightSnowStart", Chunk::TERRAIN_MAX_HEIGHT * 0.70f);
        terrainShader.setFloat("heightSnowFull",  Chunk::TERRAIN_MAX_HEIGHT * 0.85f);
        terrainShader.setFloat("textureTilingFactor", 16.0f);

        // Lighting uniforms (already here, good)
        glm::vec3 lightDirection_main = glm::normalize(glm::vec3(-0.5f, -1.0f, -0.3f));
        terrainShader.setVec3("lightDir_world", lightDirection_main);
        terrainShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 0.95f));
        terrainShader.setFloat("ambientStrength", 0.25f);
        terrainShader.setVec3("viewPos_world", camera.Position); 
        terrainShader.setFloat("specularStrength", 0.4f);      
        terrainShader.setInt("shininess", 32);                 

        // Fog uniforms (already here, good)
        terrainShader.setVec3("fogColor", currentFogColor);
        terrainShader.setFloat("fogDensity", 0.015f); 

        // Bind textures once before rendering all chunks (already here, good)
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grassTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, rockTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, snowTexture);

        // vvv UNCOMMENT OR ADD THIS LINE vvv
        terrainManager.renderActiveChunks(terrainShader); 
        // ^^^ END OF CHANGE ^^^

        // --- Render Skybox --- (already here, good)
        glDepthFunc(GL_LEQUAL); 
        skyboxShader.use();
        glm::mat4 skyboxView = glm::mat4(glm::mat3(camera.GetViewMatrix())); 
        skyboxShader.setMat4("view", skyboxView);
        skyboxShader.setMat4("projection", projection); 

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0); 
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); 

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // std::cout << "Total frames terrain was rendered: " << framesRendered << std::endl; // Comment out or remove

    // Clean up skybox resources
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteTextures(1, &cubemapTexture);
    // Clean up textures
    glDeleteTextures(1, &grassTexture);
    glDeleteTextures(1, &rockTexture);
    glDeleteTextures(1, &snowTexture);

    // Terminate GLFW, cleaning up all resources
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    SCR_WIDTH = width; // Update screen dimensions
    SCR_HEIGHT = height;
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}