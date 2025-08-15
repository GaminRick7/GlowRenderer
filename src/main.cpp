#include <iostream>
#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "rendering/shader.h"
#include "rendering/PBRMesh.h"
#include "rendering/OBJLoader.h"
#include "core/Camera.h"
#include "lighting/PointLight.h"
#include "lighting/DirectionalLight.h"
#include "lighting/SpotLight.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "rendering/DeferredRenderer.h"
#include "utils/FrustumCulling.h"

// Constants
constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;

// Global variables for cleanup
SDL_Window* g_window = nullptr;
SDL_GLContext g_glContext = nullptr;

// Global variables for frustum culling statistics
int g_culledObjects = 0;
int g_totalObjects = 101;  // 1 plane + 100 bunnies
int g_visibleObjects = 0;
BoundingBox g_bunnyBoundingBox;  // Global bunny bounding box
bool g_frustumCullingEnabled = true;  // Toggle for frustum culling

// Function declarations
bool initializeSDL();
bool createWindow();
bool initializeOpenGL();
bool initializeImGui();
void cleanup();
void processInput(SDL_Event& event, Camera& camera, bool& quit, bool& firstMouse, int& lastX, int& lastY, bool cameraMode);
void updateCamera(Camera& camera, const Uint8* state, float deltaTime, bool& cameraMode);
void renderImGui(Camera& camera, float currentFPS, float deltaTime, bool cameraMode);
std::vector<Vertex> createPlaneVertices();
std::vector<GLuint> createPlaneIndices();
std::vector<Vertex> calculateTangentsBitangents(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);


int main() {
    // ===== INITIALIZATION =====
    if (!initializeSDL()) {
        return -1;
    }

    if (!createWindow()) {
        cleanup();
        return -1;
    }

    if (!initializeOpenGL()) {
        cleanup();
        return -1;
    }

    if (!initializeImGui()) {
        cleanup();
        return -1;
    }

    // Set OpenGL state
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    // ===== GEOMETRY CREATION =====
    auto planeVertices = createPlaneVertices();
    auto planeIndices = createPlaneIndices();
    
    // Calculate tangents and bitangents for normal mapping
    planeVertices = calculateTangentsBitangents(planeVertices, planeIndices);

    // Create PBR material for the plane
    PBRMaterial planePBRMaterial(
        "Textures/TCom_Scifi_Panel_2K_albedo.png",
        "Textures/TCom_Scifi_Panel_2K_normal.png",
        "Textures/TCom_Scifi_Panel_2K_metallic.png",
        "Textures/TCom_Scifi_Panel_2K_roughness.png",
        "Textures/TCom_Scifi_Panel_2K_ao.png"
    );

    // Create plane mesh
    PBRMesh planeMesh(planeVertices, planeIndices, std::move(planePBRMaterial));

    // Load bunny mesh
    std::vector<Vertex> bunnyVertices;
    std::vector<GLuint> bunnyIndices;
    try {
        auto bunnyData = loadOBJFile("Meshes/bunny.obj");
        bunnyVertices = bunnyData.first;
        bunnyIndices = bunnyData.second;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load bunny model: " << e.what() << std::endl;
        return -1;
    }

    // Generate texture coordinates for bunny (spherical mapping)
    for (auto& vertex : bunnyVertices) {
        // Convert position to spherical coordinates for texture mapping
        glm::vec3 pos = vertex.position;
        float radius = glm::length(pos);
        
        if (radius > 0.0f) {
            // Spherical coordinates: u = azimuth angle, v = elevation angle
            float u = 0.5f + (atan2(pos.z, pos.x) / (2.0f * M_PI));  // Azimuth: 0 to 1
            float v = 0.5f + (asin(pos.y / radius) / M_PI);          // Elevation: 0 to 1
            
            vertex.texCoord = glm::vec2(u, v);
        } else {
            vertex.texCoord = glm::vec2(0.5f, 0.5f);  // Center point
        }
    }

    // Calculate tangents and bitangents for bunny (after texture coordinates are set)
    bunnyVertices = calculateTangentsBitangents(bunnyVertices, bunnyIndices);

    // Create PBR material for the bunny
    PBRMaterial bunnyPBRMaterial(
        "Textures/TCom_Plastic_SpaceBlanketFolds_2K_albedo.png",
        "Textures/TCom_Plastic_SpaceBlanketFolds_2K_normal.png",
        "Textures/TCom_Plastic_SpaceBlanketFolds_2K_metallic.png",
        "Textures/TCom_Plastic_SpaceBlanketFolds_2K_roughness.png",
        "Textures/TCom_Plastic_SpaceBlanketFolds_2K_ao.png"
    );
    


    // Create a single bunny mesh instance
    PBRMesh bunnyMesh(bunnyVertices, bunnyIndices, std::move(bunnyPBRMaterial));

    // Create transformation matrices for 100 bunny instances
    std::vector<glm::mat4> bunnyTransforms;
    
    // Arrange bunnies in a 10x10 grid above the plane
    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            // Calculate grid position
            float x = (col - 5) * 1.0f; // -5 to +5, spaced 1 unit apart
            float y = 0.0f;              // Fixed height above plane
            float z = (row - 5) * 1.0f; // -5 to +5, spaced 1 unit apart
            
            // Create transformation matrix
            glm::mat4 transform = glm::mat4(1.0f);
            transform = glm::translate(transform, glm::vec3(x, y, z));
            transform = glm::scale(transform, glm::vec3(3.0f)); // Scale up the bunny by 3x
            
                    bunnyTransforms.push_back(transform);
    }
    
    // Calculate bounding box for bunny mesh (in local space)
    std::vector<glm::vec3> bunnyPositions;
    for (const auto& vertex : bunnyVertices) {
        bunnyPositions.push_back(vertex.position);
    }
    g_bunnyBoundingBox = BoundingBox::fromVertices(bunnyPositions);
}

    // ===== SHADER CREATION =====
    Shader gbufferShader("Shaders/gbuffer.vert", "Shaders/gbuffer_PBR.frag");
    Shader deferredLightingShader("Shaders/deferred_lighting.vert", "Shaders/deferred_lighting_PBR.frag");

    // ===== LIGHT SETUP =====
    // Original point lights
    auto pointLight1 = std::make_unique<PointLight>(
        glm::vec3(0.0f, 0.5f, 2.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),  // White
        5.0f, 0.5f, 0.001f, 0.001f
    );

    auto pointLight2 = std::make_unique<PointLight>(
        glm::vec3(2.0f, 0.8f, 0.0f),
        glm::vec3(0.5f, 1.0f, 0.5f),  // Green
        4.0f, 1.0f, 0.8f, 0.8f
    );

    auto pointLight3 = std::make_unique<PointLight>(
        glm::vec3(-2.0f, 0.6f, -1.0f),
        glm::vec3(1.0f, 0.5f, 0.5f),  // Red
        4.5f, 0.7f, 0.5f, 0.5f
    );

    auto pointLight4 = std::make_unique<PointLight>(
        glm::vec3(0.0f, 0.7f, -3.0f),
        glm::vec3(0.5f, 0.5f, 1.0f),  // Blue
        4.2f, 0.8f, 0.6f, 0.6f
    );

    // Create 50 point lights in a 5x10 grid above the bunnies
    std::vector<std::unique_ptr<PointLight>> gridLights;
    
    // All lights are pure white
    std::vector<glm::vec3> colors = {
        glm::vec3(1.0f, 1.0f, 1.0f),   // Pure White
        glm::vec3(1.0f, 1.0f, 1.0f),   // Pure White
        glm::vec3(1.0f, 1.0f, 1.0f),   // Pure White
        glm::vec3(1.0f, 1.0f, 1.0f),   // Pure White
        glm::vec3(1.0f, 1.0f, 1.0f),   // Pure White
        glm::vec3(1.0f, 1.0f, 1.0f),   // Pure White
        glm::vec3(1.0f, 1.0f, 1.0f),   // Pure White
        glm::vec3(1.0f, 1.0f, 1.0f),   // Pure White
        glm::vec3(1.0f, 1.0f, 1.0f),   // Pure White
        glm::vec3(1.0f, 1.0f, 1.0f),   // Pure White
        glm::vec3(1.0f, 1.0f, 1.0f),   // Pure White
        glm::vec3(1.0f, 1.0f, 1.0f),   // Pure White
        glm::vec3(1.0f, 1.0f, 1.0f),   // Pure White
        glm::vec3(1.0f, 1.0f, 1.0f),   // Pure White
        glm::vec3(1.0f, 1.0f, 1.0f),   // Pure White
        glm::vec3(1.0f, 1.0f, 1.0f),   // Pure White
        glm::vec3(1.0f, 1.0f, 1.0f),   // Pure White
        glm::vec3(1.0f, 1.0f, 1.0f),   // Pure White
        glm::vec3(1.0f, 1.0f, 1.0f),   // Pure White
        glm::vec3(1.0f, 1.0f, 1.0f),   // Pure White
        glm::vec3(1.0f, 1.0f, 1.0f),   // Pure White
        glm::vec3(1.0f, 1.0f, 1.0f),   // Pure White
        glm::vec3(1.0f, 1.0f, 1.0f),   // Pure White
        glm::vec3(1.0f, 1.0f, 1.0f),   // Pure White
        glm::vec3(1.0f, 1.0f, 1.0f),   // Pure White
        glm::vec3(1.0f, 1.0f, 1.0f),   // Pure White
        glm::vec3(1.0f, 1.0f, 1.0f),   // Pure White
        glm::vec3(1.0f, 1.0f, 1.0f),   // Pure White
    };
    
    // Create 50 point lights in a 5x10 grid above the bunnies
    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 5; ++col) {
            // Calculate position to place lights between bunny rows and columns
            float x = (col - 2) * 2.0f -0.5f;  // -3 to +5, between bunny columns
            float y = 1.f;                      // Fixed height above bunnies
            float z = (row - 5) * 1.0f + 0.5f;  // -4.5 to +5.5, between bunny rows
            
            // Select color from palette (cycle through colors)
            glm::vec3 color = colors[(row * 5 + col) % colors.size()];
            
            // Create light with brighter attenuation for more intense lighting
            gridLights.push_back(std::make_unique<PointLight>(
                glm::vec3(x, y, z),
                color,
                6.0f, 0.1f, 0.01f, 0.001f  // Higher intensity, lower attenuation for brighter lights
            ));
        }
    }

    // Spotlights
    auto spotLight1 = std::make_unique<SpotLight>(
        glm::vec3(3.0f, 1.5f, 3.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),  // Pointing down
        glm::vec3(1.0f, 1.0f, 0.0f),   // Yellow
        glm::cos(glm::radians(12.5f)),  // Inner cutoff
        glm::cos(glm::radians(17.5f)),  // Outer cutoff
        6.0f, 1.0f, 0.09f, 0.032f
    );

    auto spotLight2 = std::make_unique<SpotLight>(
        glm::vec3(-3.0f, 1.2f, -2.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),  // Pointing down
        glm::vec3(1.0f, 0.0f, 1.0f),   // Magenta
        glm::cos(glm::radians(15.0f)),  // Inner cutoff
        glm::cos(glm::radians(20.0f)),  // Outer cutoff
        5.8f, 1.0f, 0.09f, 0.032f
    );

    auto spotLight3 = std::make_unique<SpotLight>(
        glm::vec3(0.0f, 1.8f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),  // Pointing down
        glm::vec3(0.0f, 1.0f, 1.0f),   // Cyan
        glm::cos(glm::radians(10.0f)),  // Inner cutoff
        glm::cos(glm::radians(15.0f)),  // Outer cutoff
        7.5f, 1.0f, 0.09f, 0.032f
    );

    auto spotLight4 = std::make_unique<SpotLight>(
        glm::vec3(4.0f, 1.0f, -4.0f),
        glm::vec3(-1.0f, -0.5f, 0.0f),  // Angled
        glm::vec3(1.0f, 0.8f, 0.2f),    // Orange
        glm::cos(glm::radians(20.0f)),  // Inner cutoff
        glm::cos(glm::radians(25.0f)),  // Outer cutoff
        5.5f, 1.0f, 0.09f, 0.032f
    );

    // Directional light (sun-like)
    auto directionalLight = std::make_unique<DirectionalLight>(
        glm::vec3(-0.2f, -1.0f, -0.3f),  // Direction
        glm::vec3(0.8f, 0.8f, 0.7f),     // Warm sunlight color
        1.5f                             // Intensity
    );

    // ===== CAMERA AND MATRICES SETUP =====
    Camera camera(glm::vec3(0.0f, 2.0f, 5.0f));
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);

    // ===== TIMING AND INPUT VARIABLES =====
    bool firstMouse = true;
    int lastX = WINDOW_WIDTH / 2;
    int lastY = WINDOW_HEIGHT / 2;
    Uint32 lastFrameTime = SDL_GetTicks();
    float deltaTime = 0.0f;
    Uint32 frameCount = 0;
    Uint32 fpsStartTime = SDL_GetTicks();
    float currentFPS = 0.0f;
    bool cameraMode = true;  // true = camera look-around, false = ImGui interaction
    
    // ===== FRUSTUM CULLING SETUP =====
    Frustum frustum;

    // Enable relative mouse mode initially
    SDL_SetRelativeMouseMode(SDL_TRUE);

    // ===== RENDERER SETUP =====
    DeferredRenderer deferredRenderer(WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!deferredRenderer.initialize()) {
        std::cerr << "Failed to initialize deferred renderer!" << std::endl;
        cleanup();
        return -1;
    }

    // Create geometry meshes vector with plane and bunny
    std::vector<PBRMesh*> geometryMeshes = {&planeMesh, &bunnyMesh};

    // ===== MAIN RENDER LOOP =====
    bool quit = false;
    SDL_Event event;
    const Uint8* state = SDL_GetKeyboardState(NULL);

    while (!quit) {
        // Calculate delta time and FPS
        Uint32 currentFrameTime = SDL_GetTicks();
        deltaTime = (currentFrameTime - lastFrameTime) / 1000.0f;
        lastFrameTime = currentFrameTime;

        frameCount++;
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - fpsStartTime >= 1000) {
            currentFPS = (float)frameCount * 1000.0f / (float)(currentTime - fpsStartTime);
            frameCount = 0;
            fpsStartTime = currentTime;
        }

        // Handle events
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            processInput(event, camera, quit, firstMouse, lastX, lastY, cameraMode);
        }

        // Update camera
        updateCamera(camera, state, deltaTime, cameraMode);

        // Clear buffers
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Render ImGui
        renderImGui(camera, currentFPS, deltaTime, cameraMode);

        // ===== DEFERRED RENDERING PASSES =====
        // Update frustum with current view-projection matrix
        glm::mat4 viewProjection = projection * camera.getViewMatrix();
        frustum.extractPlanes(viewProjection);
        
        // Geometry pass: Render to G-Buffer with frustum culling
        std::vector<glm::mat4> modelMatrices;
        std::vector<PBRMesh*> visibleMeshes;
        
        // Always render the plane (ground)
        modelMatrices.push_back(glm::mat4(1.0f)); // Identity matrix for plane
        visibleMeshes.push_back(&planeMesh);
        
        // Frustum culling for bunnies
        g_culledObjects = 0;
        if (g_frustumCullingEnabled) {
            // Frustum culling enabled - only render visible bunnies
            for (const auto& transform : bunnyTransforms) {
                // Transform bunny bounding box to world space
                BoundingBox worldBoundingBox = g_bunnyBoundingBox.transform(transform);
                
                // Check if bunny is visible
                if (frustum.isBoundingBoxInside(worldBoundingBox)) {
                    modelMatrices.push_back(transform);
                    visibleMeshes.push_back(&bunnyMesh);
                } else {
                    g_culledObjects++;
                }
            }
        } else {
            // Frustum culling disabled - render all bunnies
            for (const auto& transform : bunnyTransforms) {
                modelMatrices.push_back(transform);
                visibleMeshes.push_back(&bunnyMesh);
            }
            g_culledObjects = 0;  // No culling when disabled
        }
        
        // Update visible objects count for ImGui
        g_visibleObjects = (int)visibleMeshes.size();
        
        // Use visible meshes from frustum culling
        deferredRenderer.renderGeometryPass(visibleMeshes, modelMatrices, gbufferShader, camera.getViewMatrix(), projection);

        // Lighting pass: Calculate lighting and display result
        deferredLightingShader.use();
        
        // Set up light uniforms for deferred lighting
        std::vector<glm::vec3> lightPositions;
        std::vector<glm::vec3> lightColors;
        
        // Add original 4 point lights
        lightPositions.push_back(pointLight1->getPosition());
        lightPositions.push_back(pointLight2->getPosition());
        lightPositions.push_back(pointLight3->getPosition());
        lightPositions.push_back(pointLight4->getPosition());
        
        lightColors.push_back(pointLight1->getColor());
        lightColors.push_back(pointLight2->getColor());
        lightColors.push_back(pointLight3->getColor());
        lightColors.push_back(pointLight4->getColor());
        
        // Add all 100 grid lights
        for (const auto& light : gridLights) {
            lightPositions.push_back(light->getPosition());
            lightColors.push_back(light->getColor());
        }
        
        // Set spotlight positions, directions, colors, and cutoffs
        std::vector<glm::vec3> spotLightPositions = {
            spotLight1->getPosition(),
            spotLight2->getPosition(),
            spotLight3->getPosition(),
            spotLight4->getPosition()
        };
        std::vector<glm::vec3> spotLightDirections = {
            spotLight1->getDirection(),
            spotLight2->getDirection(),
            spotLight3->getDirection(),
            spotLight4->getDirection()
        };
        std::vector<glm::vec3> spotLightColors = {
            spotLight1->getColor(),
            spotLight2->getColor(),
            spotLight3->getColor(),
            spotLight4->getColor()
        };
        std::vector<float> spotLightInnerCutoffs = {
            spotLight1->getInnerCutoff(),
            spotLight2->getInnerCutoff(),
            spotLight3->getInnerCutoff(),
            spotLight4->getInnerCutoff()
        };
        std::vector<float> spotLightOuterCutoffs = {
            spotLight1->getOuterCutoff(),
            spotLight2->getOuterCutoff(),
            spotLight3->getOuterCutoff(),
            spotLight4->getOuterCutoff()
        };
        
        // Set all light uniforms (up to 64 lights supported)
        for (int i = 0; i < 64; i++) {
            if (i < lightPositions.size()) {
                deferredLightingShader.setVec3("lightPositions[" + std::to_string(i) + "]", lightPositions[i]);
                deferredLightingShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);
            } else {
                // Set unused lights to zero
                deferredLightingShader.setVec3("lightPositions[" + std::to_string(i) + "]", glm::vec3(0.0f));
                deferredLightingShader.setVec3("lightColors[" + std::to_string(i) + "]", glm::vec3(0.0f));
            }
            
            if (i < spotLightPositions.size()) {
                deferredLightingShader.setVec3("spotLightPositions[" + std::to_string(i) + "]", spotLightPositions[i]);
                deferredLightingShader.setVec3("spotLightDirections[" + std::to_string(i) + "]", spotLightDirections[i]);
                deferredLightingShader.setVec3("spotLightColors[" + std::to_string(i) + "]", spotLightColors[i]);
                deferredLightingShader.setFloat("spotLightInnerCutoffs[" + std::to_string(i) + "]", spotLightInnerCutoffs[i]);
                deferredLightingShader.setFloat("spotLightOuterCutoffs[" + std::to_string(i) + "]", spotLightOuterCutoffs[i]);
            } else {
                // Set unused spotlights to zero
                deferredLightingShader.setVec3("spotLightPositions[" + std::to_string(i) + "]", glm::vec3(0.0f));
                deferredLightingShader.setVec3("spotLightDirections[" + std::to_string(i) + "]", glm::vec3(0.0f));
                deferredLightingShader.setVec3("spotLightColors[" + std::to_string(i) + "]", glm::vec3(0.0f));
                deferredLightingShader.setFloat("spotLightInnerCutoffs[" + std::to_string(i) + "]", 0.0f);
                deferredLightingShader.setFloat("spotLightOuterCutoffs[" + std::to_string(i) + "]", 0.0f);
            }
        }
        
        // Set directional light
        deferredLightingShader.setVec3("dirLightDirection", directionalLight->getDirection());
        deferredLightingShader.setVec3("dirLightColor", directionalLight->getColor());
        deferredLightingShader.setBool("hasDirLight", true);
        
        deferredLightingShader.setInt("numLights", lightPositions.size());
        deferredLightingShader.setInt("numSpotLights", spotLightPositions.size());
        
        deferredRenderer.renderLightingPass(deferredLightingShader, camera.getPosition());

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        SDL_GL_SwapWindow(g_window);
    }

    // ===== CLEANUP =====
    planeMesh.destroy();
    bunnyMesh.destroy();
    deferredRenderer.cleanup();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    cleanup();
    return 0;
}

// Implementation of helper functions
bool initializeSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

bool createWindow() {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    g_window = SDL_CreateWindow(
        "OpenGL Renderer",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );

    if (!g_window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

bool initializeOpenGL() {
    g_glContext = SDL_GL_CreateContext(g_window);
    if (!g_glContext) {
        std::cerr << "OpenGL context could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }
    
    // Enable backface culling for better performance
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);  // Counter-clockwise winding order
    
    return true;
}

bool initializeImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui_ImplSDL2_InitForOpenGL(g_window, g_glContext);
    ImGui_ImplOpenGL3_Init("#version 410");
    return true;
}

void cleanup() {
    if (g_glContext) {
        SDL_GL_DeleteContext(g_glContext);
        g_glContext = nullptr;
    }
    if (g_window) {
        SDL_DestroyWindow(g_window);
        g_window = nullptr;
    }
    SDL_Quit();
}

void processInput(SDL_Event& event, Camera& camera, bool& quit, bool& firstMouse, int& lastX, int& lastY, bool cameraMode) {
    if (event.type == SDL_QUIT) {
        quit = true;
    }
    else if (event.type == SDL_MOUSEMOTION && cameraMode) {
        float xoffset = event.motion.xrel;
        float yoffset = -event.motion.yrel;
        camera.processMouseMovement(xoffset, yoffset);
    }
}

void updateCamera(Camera& camera, const Uint8* state, float deltaTime, bool& cameraMode) {
    if (state[SDL_SCANCODE_W]) camera.processKeyboard(0, deltaTime);
    if (state[SDL_SCANCODE_S]) camera.processKeyboard(1, deltaTime);
    if (state[SDL_SCANCODE_A]) camera.processKeyboard(2, deltaTime);
    if (state[SDL_SCANCODE_D]) camera.processKeyboard(3, deltaTime);
    if (state[SDL_SCANCODE_ESCAPE]) {
        cameraMode = !cameraMode;
        if (cameraMode) {
            SDL_SetRelativeMouseMode(SDL_TRUE);
        } else {
            SDL_SetRelativeMouseMode(SDL_FALSE);
        }
    }
}

void renderImGui(Camera& camera, float currentFPS, float deltaTime, bool cameraMode) {
    ImGui::Begin("Renderer Controls");
    ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", 
                camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);
    ImGui::Text("FPS: %.1f", currentFPS);
    ImGui::Text("Frame Time: %.3f ms", deltaTime * 1000.0f);
    
    ImGui::Separator();
    ImGui::Text("Camera Mode: %s", cameraMode ? "Look Around (ESC to switch)" : "ImGui Interaction (ESC to switch)");
    ImGui::Text("Controls: WASD to move, ESC to toggle mode");
    
            ImGui::Separator();
        ImGui::Text("Rendering");
        ImGui::Text("Deferred Rendering with PBR");
        ImGui::Text("Normal map visualization enabled");
        ImGui::Checkbox("Frustum Culling", &g_frustumCullingEnabled);
        ImGui::Text("Backface Culling: Enabled");
    
    ImGui::Separator();
            ImGui::Text("Scene Objects");
        ImGui::Text("Plane: 1 (Ground plane with PBR material)");
        ImGui::Text("Bunnies: 100 (10x10 tight grid, 3x scale, 1-unit spacing)");
        ImGui::Text("Total Objects: 101");
        ImGui::Text("Visible Objects: %d", g_visibleObjects);
        if (g_frustumCullingEnabled) {
            ImGui::Text("Culled Objects: %d", g_culledObjects);
            ImGui::Text("Culling Efficiency: %.1f%%", (float)g_culledObjects / g_totalObjects * 100.0f);
        } else {
            ImGui::Text("Culling: DISABLED (all objects rendered)");
        }
        ImGui::Text("Lights: 54 (4 original + 50 grid lights)");
    
    ImGui::Separator();
    ImGui::Text("Lights");
    ImGui::Text("Point Lights: 4 (White, Green, Red, Blue)");
    ImGui::Text("Spotlights: 4 (Yellow, Magenta, Cyan, Orange)");
    ImGui::Text("Directional Light: 1 (Sun-like)");
    ImGui::Text("Total: 9 lights");
    ImGui::Text("Max Capacity: 64 point lights + 64 spotlights");
    
    ImGui::End();
}

// Function to calculate tangents and bitangents for normal mapping
std::vector<Vertex> calculateTangentsBitangents(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices) {
    std::vector<Vertex> result = vertices;
    
    // Initialize tangents and bitangents to zero
    for (auto& vertex : result) {
        vertex.tangent = glm::vec3(0.0f);
        vertex.bitangent = glm::vec3(0.0f);
    }
    
    // Calculate tangents and bitangents for each triangle
    for (size_t i = 0; i < indices.size(); i += 3) {
        GLuint i0 = indices[i];
        GLuint i1 = indices[i + 1];
        GLuint i2 = indices[i + 2];
        
        Vertex& v0 = result[i0];
        Vertex& v1 = result[i1];
        Vertex& v2 = result[i2];
        
        // Calculate edges
        glm::vec3 edge1 = v1.position - v0.position;
        glm::vec3 edge2 = v2.position - v0.position;
        
        // Calculate texture coordinate differences
        glm::vec2 deltaUV1 = v1.texCoord - v0.texCoord;
        glm::vec2 deltaUV2 = v2.texCoord - v0.texCoord;
        
        // Calculate tangent and bitangent
        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        
        glm::vec3 tangent;
        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent = glm::normalize(tangent);
        
        glm::vec3 bitangent;
        bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent = glm::normalize(bitangent);
        
        // Accumulate tangents and bitangents
        v0.tangent += tangent;
        v1.tangent += tangent;
        v2.tangent += tangent;
        
        v0.bitangent += bitangent;
        v1.bitangent += bitangent;
        v2.bitangent += bitangent;
    }
    
    // Normalize accumulated tangents and bitangents
    for (auto& vertex : result) {
        vertex.tangent = glm::normalize(vertex.tangent);
        vertex.bitangent = glm::normalize(vertex.bitangent);
    }
    
    return result;
}

std::vector<Vertex> createPlaneVertices() {
    return {
        Vertex(glm::vec3(-5.0f, 0.0f, -5.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
        Vertex(glm::vec3( 5.0f, 0.0f, -5.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(5.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
        Vertex(glm::vec3( 5.0f, 0.0f,  5.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(5.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
        Vertex(glm::vec3(-5.0f, 0.0f,  5.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(0.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
    };
}

std::vector<GLuint> createPlaneIndices() {
    return {0, 3, 2, 2, 1, 0};
}


