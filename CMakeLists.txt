cmake_minimum_required(VERSION 3.10.0)
project(renderer VERSION 1.0.0)
set(CMAKE_CXX_FLAGS "-std=c++20")
find_package(OpenGL REQUIRED)

# Source files
set(SOURCES
    src/main.cpp
    src/core/Camera.cpp
    src/rendering/shader.cpp
    src/rendering/VBO.cpp
    src/rendering/VAO.cpp
    src/rendering/EBO.cpp
    src/rendering/Texture.cpp
    src/rendering/Mesh.cpp
    src/rendering/OBJLoader.cpp
    src/rendering/PBRMaterial.cpp
    src/rendering/PBRMesh.cpp
    src/rendering/DeferredRenderer.cpp
    src/lighting/Light.cpp
    src/lighting/PointLight.cpp
    src/lighting/DirectionalLight.cpp
    src/lighting/SpotLight.cpp
    src/utils/FrustumCulling.cpp
    lib/external/dependencies/glad/glad.c
    lib/external/dependencies/imgui.cpp
    lib/external/dependencies/imgui_draw.cpp
    lib/external/dependencies/imgui_tables.cpp
    lib/external/dependencies/imgui_widgets.cpp
    lib/external/dependencies/imgui_impl_sdl2.cpp
    lib/external/dependencies/imgui_impl_opengl3.cpp
)

# Header files
set(HEADERS
    include/core/Camera.h
    include/rendering/shader.h
    include/rendering/VBO.h
    include/rendering/VAO.h
    include/rendering/EBO.h
    include/rendering/Texture.h
    include/rendering/Mesh.h
    include/rendering/OBJLoader.h
    include/rendering/PBRMaterial.h
    include/rendering/PBRMesh.h
    include/lighting/Light.h
    include/lighting/PointLight.h
    include/lighting/DirectionalLight.h
    include/lighting/SpotLight.h
    include/utils/FrustumCulling.h
)

add_executable(renderer ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(renderer PRIVATE 
    include
    lib/external/dependencies
)

find_package(SDL2 REQUIRED COMPONENTS SDL2)
target_link_libraries(renderer PRIVATE SDL2::SDL2)
target_link_libraries(renderer PRIVATE ${SDL2_LIBRARIES})
include_directories(SDL2 ${SDL2_INCLUDE_DIRS})

# Copy Shaders and Textures directories to build directory
add_custom_command(TARGET renderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Shaders
    ${CMAKE_BINARY_DIR}/Shaders
)

add_custom_command(TARGET renderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Textures
    ${CMAKE_BINARY_DIR}/Textures
)

add_custom_command(TARGET renderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Meshes
    ${CMAKE_BINARY_DIR}/Meshes
)