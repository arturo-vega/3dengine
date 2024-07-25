#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtx/transform.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "terrain.h"
#include "player.h"

#include "SimplexNoise.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

#include <iostream>
#include <vector>
#include <random>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;
const float chunkHeight = 75.0f;
const float waterLevel = (chunkHeight * 0.4f) - chunkHeight; // If chunkmap.frag's water level is changed from 0.2f adjust this value
const float VIEW_DISTANCE = 1000.0f;
const int CHUNK_MAP_SIZE = 20;
const int chunkResolution = 1;
const unsigned int TEXTURE_SIZE = 10;
const int CHUNK_SIZE = 50;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastX = SCR_WIDTH / 2;
float lastY = SCR_HEIGHT / 2;
float cubeRadians = 0.0f;
bool firstMouse = true;
bool mouseLook = true;
Camera camera (glm::vec3(25.0f, 0.0f, 25.0f));
float lightPosition_y = 0.0f, lightPosition_x = 0.0f, lightPosition_z = 0.0f;
glm::vec3 lightPosition(lightPosition_x, lightPosition_y, lightPosition_z);

bool qPressed = false;
bool tPressed = false;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void updateLastFrame(void);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);
void terrainBufferWriter(std::vector<GLuint> *terrainVAOs, std::vector<GLuint> *terrainVBOs, std::vector<GLuint> *terrainEBOs, terrainChunk *chunk);
void clearBuffer(unsigned int VAO, unsigned int VBO, unsigned int EBO);
unsigned int loadCubemap(std::vector<std::string> faces);

int main(void)
{
    // Initialize and configure library
    //glfwInit();
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window;
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    // Initialize GLAD and load all OpenGL function pointers
    // Note: GLAD points to where all the OpenGL functions are on the computer, not strictly necessary
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // initialize imgui
    IMGUI_CHECKVERSION();
    if (!ImGui::CreateContext()) {
        std::cout << "Failed to initialize ImGUi" << std::endl;
        return -1;
    }
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard control
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");


    // shaders
    Shader lightingShader("../ball_game/src/colors.vs", "../ball_game/src/colors.fs");
    Shader lightCubeShader("../ball_game/src/light_cube.vs", "../ball_game/src/light_cube.fs");
    Shader chunkMapShader("../ball_game/src/chunkmap.vert", "../ball_game/src/chunkmap.frag");
    Shader skyBoxShader("../ball_game/src/skybox.vert", "../ball_game/src/skybox.frag");
    Shader waterShader("../ball_game/src/water.vert", "../ball_game/src/water.frag");

    GLfloat verticesLightCube[] = {
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };
    GLfloat vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f
    };
    GLfloat waterPlaneVertices[] = {
        -CHUNK_SIZE / 2.0f, waterLevel, -CHUNK_SIZE / 2.0f, 0.0f, 1.0f, 0.0f,
        -CHUNK_SIZE / 2.0f, waterLevel,  CHUNK_SIZE / 2.0f, 0.0f, 1.0f, 0.0f,
         CHUNK_SIZE / 2.0f, waterLevel,  CHUNK_SIZE / 2.0f, 0.0f, 1.0f, 0.0f,
         CHUNK_SIZE / 2.0f, waterLevel,  CHUNK_SIZE / 2.0f, 0.0f, 1.0f, 0.0f,
         CHUNK_SIZE / 2.0f, waterLevel, -CHUNK_SIZE / 2.0f, 0.0f, 1.0f, 0.0f,
        -CHUNK_SIZE / 2.0f, waterLevel, -CHUNK_SIZE / 2.0f, 0.0f, 1.0f, 0.0f
    };
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    

    // simplex noise values
    // Lacunarity specifies the frequency multiplier between successive octaves (default to 2.0).
    // Persistence is the loss of amplitude between successive octaves (usually 1/lacunarity)
    float lacunarity = 2.0f; // 2.0f
    float persistance = 0.3f; // 0.5f
    int octaves = 7; // 5
    // initialize terrain
    Terrain terrainMap(chunkHeight, chunkResolution, lacunarity, persistance, octaves, CHUNK_MAP_SIZE, CHUNK_SIZE);

    // vao[1] and vbo[2] for plane mesh/terrain ... should probably give it a unique named variable
    unsigned int VAOs[2], VBOs[2], lightVAO, lightVBO, skyboxVAO, skyboxVBO, waterPlaneVAO, waterPlaneVBO;
    
    // creating buffers for every chunk
    std::vector<GLuint> terrainVAOs;
    std::vector<GLuint> terrainVBOs;
    std::vector<GLuint> terrainEBOs;

    // skybox buffer
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // water plane buffer ----------------------------------------------------------------
    glGenVertexArrays(1, &waterPlaneVAO);
    glGenBuffers(1, &waterPlaneVBO);
    glBindVertexArray(waterPlaneVAO);
    glBindBuffer(GL_ARRAY_BUFFER, waterPlaneVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(waterPlaneVertices), waterPlaneVertices, GL_STATIC_DRAW);

    // verticies
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    // normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    // cube stuff -----------------------------------------------------------------------
    glGenVertexArrays(2, VAOs);
    glGenBuffers(2, VBOs);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(VAOs[0]);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

   // texture attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // light cube stuff -----------------------------------------------------------------
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glGenBuffers(1, &lightVBO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    // position attribute
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesLightCube), verticesLightCube, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // can use glgentextures to generate more than one texture at a time 
    unsigned int brick, grass;
    glGenTextures(1, &brick);
    glBindTexture(GL_TEXTURE_2D, brick);

    // texture wrapping/filtering options for current texture(s)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // load brick texture
    int width, height, nrChannels;
    unsigned char* data = stbi_load("../ball_game/src/wall.jpg", &width, &height, &nrChannels, 0);
    
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture: brick " << std::endl;
    }
    // Done loading texture so free data
    stbi_image_free(data);

    // ---- load grass texture
    glGenTextures(1, &grass);
    glBindTexture(GL_TEXTURE_2D, grass);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    data = stbi_load("../ball_game/src/grass.jpg", &width, &height, &nrChannels, 0);

    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture: grass " << std::endl;
    }
    // Done loading texture so free data
    stbi_image_free(data);

    // Load skybox into memory buffer
    std::vector<std::string> faces
	{
		"../ball_game/src/skybox/right.jpg",
		"../ball_game/src/skybox/left.jpg",
		"../ball_game/src/skybox/top.jpg",
		"../ball_game/src/skybox/bottom.jpg",
		"../ball_game/src/skybox/front.jpg",
		"../ball_game/src/skybox/back.jpg"
	};
    unsigned int cubemapTexture = loadCubemap(faces);

    skyBoxShader.use();
    skyBoxShader.setInt("skybox", 0);

 
    // Sets the color of the background
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    glEnable(GL_DEPTH_TEST);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // just stuff for the imgui thing, move it later
    bool show_demo_window = false;

    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    std::pair<int, int> lastChunk = terrainMap.currentChunk;
    std::vector<std::pair<int, int>> lastChunkList;
    bool generatedFirstChunk = false;

    /* -------Loop until the user closes the window------------ */
    while (!glfwWindowShouldClose(window))
    {
        updateLastFrame();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame(); 

        processInput(window);

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            ImGui::Begin("World Settings");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state

            ImGui::SliderFloat("Light X", &lightPosition_x, -200.0f, 200.0f);
            ImGui::SliderFloat("Light Y", &lightPosition_y, -100.0f, 200.0f);
            ImGui::SliderFloat("Light Z", &lightPosition_z, -200.0f, 200.0f);
            ImGui::SliderFloat("Light Rotation", &cubeRadians, -360.0f, 360.0f);
            ImGui::ColorEdit3("Light Color", (float*)&lightColor);

            ImGui::Text("Position: x = %.1f, y = %.1f, z = %.1f", camera.Position.x, camera.Position.y, camera.Position.z);
            ImGui::Text("Front: x = %.1f, y = %.1f, z = %.1f", camera.Front.x, camera.Front.y, camera.Front.z);
            ImGui::Text("Chunk Map Position: x = %i, z = %i", terrainMap.currentChunk.first, terrainMap.currentChunk.second);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        lightPosition = glm::vec3(lightPosition_x, lightPosition_y, lightPosition_z);


        // Render here
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        lightingShader.use();
        lightingShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
        lightingShader.setVec3("lightColor", lightColor);
        lightingShader.setVec3("lightPosition", lightPosition);
        lightingShader.setVec3("viewPosition", lightPosition);

        // projection/view matrix
        glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, VIEW_DISTANCE);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        //bind brick texture for boxes
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, brick);

        // draw first box
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model,  glm::vec3(4*cos((float)glfwGetTime() * 2.0f), 0.0f, 4*sin((float)glfwGetTime() * 2.0f)));
        lightingShader.setMat4("model", model);
        glBindVertexArray(VAOs[0]);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // draw second box
        model = glm::mat4(1.0f);
        model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(1.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(0.0f, 5 * cos((float)glfwGetTime() * 1.0f), 5 * sin((float)glfwGetTime() * 1.0f)));
        lightingShader.setMat4("model", model);
        glBindVertexArray(VAOs[0]);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // bind grass for terrain
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grass);


        // Vector of x, z pairs to be used to get chunks to be drawn
        std::vector<std::pair<int, int>> chunksToDraw;
        chunksToDraw = terrainMap.checkForVisibleChunks(CHUNK_MAP_SIZE, camera.Position.x, camera.Position.z, camera.Front);
        
        chunkMapShader.use();
        chunkMapShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
        chunkMapShader.setVec3("lightColor", lightColor);
        chunkMapShader.setVec3("lightPosition", lightPosition);
        chunkMapShader.setVec3("viewPosition", lightPosition);
        chunkMapShader.setMat4("projection", projection);
        chunkMapShader.setMat4("view", view);
        chunkMapShader.setFloat("mapHeight", chunkHeight); // Passes in the height of the chunkmap to the shader for colors

        // draw terrain
        for (int i = 0; i < chunksToDraw.size(); i++) {
            if (!generatedFirstChunk) {
                terrainChunk* chunk = &terrainMap.chunkMap[chunksToDraw[i]];
                terrainBufferWriter(&terrainVAOs, &terrainVBOs, &terrainEBOs, chunk);

                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
                chunkMapShader.setMat4("model", model);
                glBindVertexArray(terrainVAOs[i]);
                glBindBuffer(GL_ARRAY_BUFFER, terrainVBOs[i]);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBOs[i]);

                for (unsigned int strip = 0; strip <= chunk->numStrips; strip++) {
                    glDrawElements(GL_TRIANGLE_STRIP, chunk->numVertsPerStrip, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * chunk->numVertsPerStrip * strip));
                }

                if (chunk->hasWater) {
                    waterShader.use();
                    waterShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
                    waterShader.setVec3("lightColor", lightColor);
                    waterShader.setVec3("lightPosition", lightPosition);
                    waterShader.setVec3("viewPosition", lightPosition);
                    waterShader.setMat4("projection", projection);
                    waterShader.setMat4("view", view);
                    
					glBindVertexArray(waterPlaneVAO);
					glBindBuffer(GL_ARRAY_BUFFER, waterPlaneVBO);
					model = glm::mat4(1.0f);
					model = glm::translate(model, glm::vec3(chunk->posX, 0.0f, chunk->posZ));
                    waterShader.setMat4("model", model);
					glDrawArrays(GL_TRIANGLES, 0, 6);
				}
            } 
            else {
                chunkMapShader.use();
                terrainChunk* chunk = &terrainMap.chunkMap[chunksToDraw[i]];
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(0.0f, 0.0f ,0.0f));
                lightingShader.setMat4("model", model);

                glBindVertexArray(terrainVAOs[i]);

                // changes the buffers only if its a new chunk, currently rewrites every chunk, should change this so it only does it for new chunks FIX IT
                if (lastChunk.first != terrainMap.currentChunk.first || lastChunk.second != terrainMap.currentChunk.second) {
                    if (std::find(lastChunkList.begin(), lastChunkList.end(), chunk->chunkMapCoords) == lastChunkList.end()) {
                        glBindBuffer(GL_ARRAY_BUFFER, terrainVBOs[i]);
                        glBufferData(GL_ARRAY_BUFFER, chunk->vertices.size() * sizeof(float), &chunk->vertices[0], GL_STATIC_DRAW);

                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBOs[i]);
                        glBufferData(GL_ELEMENT_ARRAY_BUFFER, chunk->indices.size() * sizeof(unsigned int), &chunk->indices[0], GL_STATIC_DRAW);
                        //std::cout << "Drawing terrain chunk: " << chunk->chunkID << std::endl;
                    }
                }

                for (unsigned int strip = 0; strip <= chunk->numStrips; strip++) {
                    glDrawElements(GL_TRIANGLE_STRIP, chunk->numVertsPerStrip, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * chunk->numVertsPerStrip * strip));
                }
                if (chunk->hasWater) {
                    waterShader.use();
                    waterShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
                    waterShader.setVec3("lightColor", lightColor);
                    waterShader.setVec3("lightPosition", lightPosition);
                    waterShader.setVec3("viewPosition", lightPosition);
                    waterShader.setMat4("projection", projection);
                    waterShader.setMat4("view", view);

                    glBindVertexArray(waterPlaneVAO);
                    glBindBuffer(GL_ARRAY_BUFFER, waterPlaneVBO);
                    model = glm::mat4(1.0f);
                    model = glm::translate(model, glm::vec3(chunk->posX, 0.0f, chunk->posZ));
                    waterShader.setMat4("model", model);
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                }
            }
        }
        generatedFirstChunk = true;

        // draw light box
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        glm::rotate(model, glm::radians(cubeRadians), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, lightPosition);
        glm::scale(model, glm::vec3(0.2f));
        lightCubeShader.setMat4("model", model);
        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyBoxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
        skyBoxShader.setMat4("view", view);
        skyBoxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        // this part actually renders the gui
        ImGui::Render();
        ImGui::EndFrame();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        lastChunk = terrainMap.currentChunk;

        // Swap front and back buffers & check and call events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(2, VAOs);
    glDeleteBuffers(2, VBOs);
    glDeleteVertexArrays(1, &lightVAO);
    ImGui::DestroyContext();
    ImGui_ImplOpenGL3_Shutdown();

    glfwTerminate();
    return 0;
}


void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {

    float xpos = (float)xposIn;
    float ypos = (float)yposIn;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xOffset = xpos - lastX;
    float yOffset = ypos - lastY;
    lastX = xpos;
    lastY = ypos;
    if (mouseLook) {
        camera.ProcessMouseMovement(xOffset, yOffset);
    }
}
void processInput(GLFWwindow* window) {

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == true)
        camera.ProcessKeyboard(FAST, deltaTime);

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

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && qPressed != true) {
        GLint polygonMode;
        qPressed = true;
        glGetIntegerv(GL_POLYGON_MODE, &polygonMode);
        if (polygonMode == GL_LINE) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
    }

    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && tPressed != true) {
        tPressed = true;
        
        if (!mouseLook) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            mouseLook = true;
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            mouseLook = false;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE) {
        qPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) {
        tPressed = false;
    }
}
void updateLastFrame(void) {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
void terrainBufferWriter(std::vector<GLuint> *terrainVAOs, std::vector<GLuint> *terrainVBOs, std::vector<GLuint> *terrainEBOs, terrainChunk *chunk) {
    // terrain mesh stuff ------------------------------------------------------------------
    GLuint VAO, VBO, EBO;

    // Generate and bind the VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Generate and bind the VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, chunk->vertices.size() * sizeof(float), &chunk->vertices[0], GL_STATIC_DRAW);

    // Generate and bind the EBO
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, chunk->indices.size() * sizeof(unsigned int), &chunk->indices[0], GL_STATIC_DRAW);

    // Set up the vertex attributes
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // texture attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Store the VAO, VBO, and EBO for later use
    terrainVAOs->push_back(VAO);
    terrainVBOs->push_back(VBO);
    terrainEBOs->push_back(EBO);
}
unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void clearBuffer(unsigned int VAO, unsigned int VBO, unsigned int EBO) {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
}