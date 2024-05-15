#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtx/transform.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "terrain.h"

#include "SimplexNoise.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

#include <iostream>
#include <vector>
#include <random>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const int SCR_WIDTH = 1600;
const int SCR_HEIGHT = 1200;
float chunkHeight = 35.0f;
const float VIEW_DISTANCE = 1000.0f;
const int CHUNK_MAP_SIZE = 5; // keep this number odd... Or change the nested for loops for checking visible chunks
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
void terrainBufferWriter(unsigned int VAO, unsigned int VBO, unsigned int EBO, terrainChunk chunk);
void clearBuffer(unsigned int VAO, unsigned int VBO, unsigned int EBO);

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

    // Build and compile shader from shader.h
    //Shader myShader("../ball_game/src/vertexshader.vs", "../ball_game/src/fragmentshader.fs");

    // light shaders
    Shader lightingShader("../ball_game/src/colors.vs", "../ball_game/src/colors.fs");
    Shader lightCubeShader("../ball_game/src/light_cube.vs", "../ball_game/src/light_cube.fs");

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

    // simplex noise values
    // Lacunarity specifies the frequency multiplier between successive octaves (default to 2.0).
    // Persistence is the loss of amplitude between successive octaves (usually 1/lacunarity)
    float lacunarity = 1.99f;
    float persistance = 0.5f;
    int octaves = 5;
    // initialize terrain, generates initial chunks
    Terrain terrainMap(chunkHeight, chunkResolution, lacunarity, persistance, octaves, CHUNK_MAP_SIZE, CHUNK_SIZE);

    // vao[1] and vbo[2] for plane mesh/terrain ... should probably give it a unique named variable
    unsigned int VAOs[2], VBOs[2], lightVAO, lightVBO;
    unsigned int terrainEBO = 0;    

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
 
    // Sets the color of the background
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    glEnable(GL_DEPTH_TEST);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // just stuff for the imgui thing, move it later
    bool show_demo_window = false;

    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    std::pair<int, int> lastChunk = terrainMap.currentChunk;
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
        chunksToDraw = terrainMap.checkForVisibleChunks(CHUNK_MAP_SIZE, camera.Position.x, camera.Position.z);

        //std::cout << "SIZE OF VECTOR: " << chunksToDraw.size() << std::endl;

        // draw terrain
        for (int i = 0; i < chunksToDraw.size(); i++) {
            terrainChunk *chunk = &terrainMap.chunkMap[chunksToDraw[i]];

            if (!generatedFirstChunk) {
                terrainBufferWriter(VAOs[1], VBOs[1], terrainEBO, *chunk);
                generatedFirstChunk = true;
            }
            
            model = glm::mat4(1.0f);
            // I subtrack the chunk x and z coordinates multiplied by the chunk size so that the map does not advance faster than the movemen tof the camera
            // Subtracking by multiples of two of the chunk size puts the camera in the center of the center chunk
            model = glm::translate(model, glm::vec3(chunk->posX + (CHUNK_SIZE * 2), -10.0f, chunk->posZ + (CHUNK_SIZE * 2)));

            //model = glm::translate(model, glm::vec3((chunk->posX - (CHUNK_SIZE * 2)) - (terrainMap.currentChunk.first * CHUNK_SIZE), -10.0f,(chunk->posZ - (CHUNK_SIZE * 2)) - (terrainMap.currentChunk.second * CHUNK_SIZE)));

            lightingShader.setMat4("model", model);
            glBindVertexArray(VAOs[1]);

            for (unsigned int i = 0; i <= chunk->numStrips; i++) {
                glDrawElements(GL_TRIANGLE_STRIP, chunk->numVertsPerStrip, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * chunk->numVertsPerStrip * i));
            }
        }

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

        // this part actually renders the gui
        ImGui::Render();
        ImGui::EndFrame();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap front and back buffers & check and call events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(2, VAOs);
    glDeleteBuffers(2, VBOs);
    glDeleteVertexArrays(1, &lightVAO);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();

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
void terrainBufferWriter(unsigned int VAO, unsigned int VBO, unsigned int EBO, terrainChunk chunk) {
    // terrain mesh stuff ------------------------------------------------------------------
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, chunk.vertices.size() * sizeof(float), &chunk.vertices[0], GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // texture attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // ebo buffer that takes in indices
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, chunk.indices.size() * sizeof(unsigned int), &chunk.indices[0], GL_STATIC_DRAW);
}

void clearBuffer(unsigned int VAO, unsigned int VBO, unsigned int EBO) {
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
}