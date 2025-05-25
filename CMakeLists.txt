cmake_minimum_required(VERSION 3.10)
project(OpenGLTerrain VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add include directories
include_directories(
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/include # Ensures glad/glad.h is found
)

# Add source files
file(GLOB SOURCES
    "src/*.cpp"  # This will pick up Texture.cpp, PerlinNoise.cpp etc.
    "src/glad.c"
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# On macOS, silence deprecation warnings for OpenGL
if(APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE GL_SILENCE_DEPRECATION)
endif()

# Find and link OpenGL
find_package(OpenGL REQUIRED)
if(OPENGL_FOUND)
    message(STATUS "Found OpenGL: ${OPENGL_LIBRARIES}")
    # Use keyword signature
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL) 
else()
    message(FATAL_ERROR "OpenGL not found!")
endif()

# Find and link GLFW
find_package(glfw3 REQUIRED)
if(glfw3_FOUND)
    message(STATUS "Found GLFW3: ${glfw3_LIBRARIES}")
    # Use keyword signature
    target_link_libraries(${PROJECT_NAME} PRIVATE glfw) 
else()
    message(FATAL_ERROR "GLFW3 not found!")
endif()

# Add Homebrew prefix to CMake's search paths for packages on macOS
if(APPLE)
    set(HOMEBREW_PREFIX_INTEL "/usr/local")
    set(HOMEBREW_PREFIX_ARM "/opt/homebrew")
    
    if(IS_DIRECTORY "${HOMEBREW_PREFIX_ARM}/include/glm" OR IS_DIRECTORY "${HOMEBREW_PREFIX_ARM}/lib/cmake/glm")
        list(APPEND CMAKE_PREFIX_PATH "${HOMEBREW_PREFIX_ARM}")
        message(STATUS "Added Homebrew ARM prefix to CMAKE_PREFIX_PATH: ${HOMEBREW_PREFIX_ARM}")
    elseif(IS_DIRECTORY "${HOMEBREW_PREFIX_INTEL}/include/glm" OR IS_DIRECTORY "${HOMEBREW_PREFIX_INTEL}/lib/cmake/glm")
        list(APPEND CMAKE_PREFIX_PATH "${HOMEBREW_PREFIX_INTEL}")
        message(STATUS "Added Homebrew Intel prefix to CMAKE_PREFIX_PATH: ${HOMEBREW_PREFIX_INTEL}")
    else()
        message(STATUS "GLM not found in standard Homebrew locations (/opt/homebrew or /usr/local). Will rely on system paths or other CMAKE_PREFIX_PATH entries. Ensure 'brew install glm' was successful.")
    endif()
endif()

# Find and link GLM
find_package(glm REQUIRED) 

if(TARGET glm::glm)
    message(STATUS "Found GLM and linking against imported target glm::glm.")
    # This is already using the keyword signature, which is good
    target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm) 
else()
    if(GLM_INCLUDE_DIRS)
         message(WARNING "GLM imported target 'glm::glm' not found. Using GLM_INCLUDE_DIRS: ${GLM_INCLUDE_DIRS}")
         target_include_directories(${PROJECT_NAME} PRIVATE ${GLM_INCLUDE_DIRS})
    elseif(GLM_INCLUDE_DIR) 
         message(WARNING "GLM imported target 'glm::glm' not found. Using GLM_INCLUDE_DIR: ${GLM_INCLUDE_DIR}")
         target_include_directories(${PROJECT_NAME} PRIVATE ${GLM_INCLUDE_DIR})
    else()
         message(FATAL_ERROR "GLM was found by find_package(glm REQUIRED), but it did not provide the 'glm::glm' target or standard include directory variables (GLM_INCLUDE_DIRS/GLM_INCLUDE_DIR). Please check your GLM installation and version.")
    endif()
endif()

# On macOS, we need to link against Cocoa and IOKit for windowing and input
# These are often handled by GLFW, but explicitly linking them can sometimes resolve issues.
if(APPLE)
    # GLFW should handle these framework links.
    # If you encounter linking errors related to Cocoa or IOKit,
    # you might need to add them explicitly:
    # target_link_libraries(${PROJECT_NAME} "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
endif()