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
    // Make sure the path to shaders is correct relative to your executable's working directory
    // If running from build/, paths would be "../shaders/basic.vert"
    Shader terrainShader("../shaders/basic.vert", "../shaders/basic.frag");
    if (terrainShader.getID() == 0) { std::cerr << "Failed to load shaders." << std::endl; glfwTerminate(); return -1; }
    
    Shader skyboxShader("../shaders/skybox.vert", "../shaders/skybox.frag");
    if (skyboxShader.getID() == 0) { std::cerr << "Failed to load skybox shaders." << std::endl; glfwTerminate(); return -1; }


    // Load textures
    // Adjust paths to be relative to the build directory (where the executable runs)
    // Go up one level ("../") to the project root, then into "textures/"
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
    // Order: Right, Left, Top, Bottom, Front, Back
    std::vector<std::string> faces {
        "../textures/skybox/space_rt.png", "../textures/skybox/space_lf.png",
        "../textures/skybox/space_up.png", "../textures/skybox/space_dn.png",
        "../textures/skybox/space_ft.png", "../textures/skybox/space_bk.png"
    };
    unsigned int cubemapTexture = loadCubemap(faces);
    if (cubemapTexture == 0) {
        std::cerr << "Failed to load cubemap texture. Check paths and filenames in the 'faces' vector." << std::endl;
    }

    PerlinNoise perlinGenerator; 
    int mapWidth = 128;
    int mapDepth = 128;
    HeightMap terrainHeightMap(mapWidth, mapDepth);
    
    // --- Adjust these Perlin Noise parameters ---
    float terrainScale = 30.0f;       
    int octaves = 5;                
    float persistence = 0.5f;       
    float minHeight = 0.0f;
    float maxHeight = 25.0f;
    
    float peakExponent = 1.2f; // Values > 1.0 accentuate peaks.
    // --- End of parameter adjustment ---

    terrainHeightMap.generatePerlinHeights(perlinGenerator, terrainScale, octaves, persistence, minHeight, maxHeight, peakExponent);
    
    // --- Apply smoothing with the kernel ---
    // You can adjust the number of iterations and kernelSize here.
    // kernelSize = 1 means a 3x3 averaging window.
    // kernelSize = 0 means a 1x1 window (effectively no spatial smoothing from this function).
    terrainHeightMap.smoothHeights(1, 1); // Example: 1 iteration, 3x3 kernel
    // --- End of smoothing ---

    Mesh terrainMesh;
    terrainMesh.generateFromHeightMap(terrainHeightMap, 0.5f, 1.0f); 
    terrainMesh.setupMesh();

    Frustum cameraFrustum; // Create a Frustum object

    // Set texture units for shaders (once is enough)
    terrainShader.use();
    terrainShader.setInt("textureGrass", 0);
    terrainShader.setInt("textureRock", 1);
    terrainShader.setInt("textureSnow", 2);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    unsigned int framesRendered = 0; // For simple culling feedback

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(currentFogColor.r, currentFogColor.g, currentFogColor.b, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 viewProjectionMatrix = projection * view; // Combined matrix

        // Update frustum planes for the current frame
        cameraFrustum.update(viewProjectionMatrix);

        // --- Render Terrain (conditionally) ---
        terrainShader.use();
        terrainShader.setMat4("projection", projection);
        terrainShader.setMat4("view", view);
        glm::mat4 model = glm::mat4(1.0f); // Assuming terrain is at origin
        terrainShader.setMat4("model", model);
        
        // Texturing uniforms
        terrainShader.setFloat("heightRockStart", maxHeight * 0.35f);
        terrainShader.setFloat("heightRockFull",  maxHeight * 0.55f);
        terrainShader.setFloat("heightSnowStart", maxHeight * 0.70f);
        terrainShader.setFloat("heightSnowFull",  maxHeight * 0.85f);
        terrainShader.setFloat("textureTilingFactor", 16.0f);

        // Lighting uniforms
        glm::vec3 lightDirection_main = glm::normalize(glm::vec3(-0.5f, -1.0f, -0.3f));
        terrainShader.setVec3("lightDir_world", lightDirection_main);
        terrainShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 0.95f));
        terrainShader.setFloat("ambientStrength", 0.25f);
        terrainShader.setVec3("viewPos_world", camera.Position); 
        terrainShader.setFloat("specularStrength", 0.4f);      
        terrainShader.setInt("shininess", 32);                 

        // Fog uniforms
        terrainShader.setVec3("fogColor", currentFogColor);
        terrainShader.setFloat("fogDensity", 0.015f); // Adjust this for fog thickness (0.01 to 0.05 is a good range to start)
        // For linear fog (if you switched in shader):
        // terrainShader.setFloat("fogStart", 50.0f);
        // terrainShader.setFloat("fogEnd", 200.0f);


        // Frustum Culling Check for terrain
        // The terrain's AABB is in model space. If model matrix is not identity, transform AABB first.
        // For now, assuming model matrix is identity (terrain at origin).
        if (cameraFrustum.isAABBVisible(terrainMesh.boundingBox)) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, grassTexture);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, rockTexture);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, snowTexture);
            terrainMesh.draw();
            framesRendered++; // Increment if drawn
        } else {
            // Optional: Log when culled for debugging
            // std::cout << "Terrain culled!" << std::endl;
        }


        // --- Render Skybox ---
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
    std::cout << "Total frames terrain was rendered: " << framesRendered << std::endl;

    // Clean up skybox resources
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteTextures(1, &cubemapTexture);
    // Clean up textures (optional, as OS does it on exit, but good practice)
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