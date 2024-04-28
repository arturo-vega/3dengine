#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtx/transform.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include <glm/glm/gtc/noise.hpp>

#include <cstdint>

#include "shader.h"
#include "camera.h"
#include "SimplexNoise.h"

#include <iostream>
#include <vector>
#include <random>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const float PLANE_SIZE = 10.0f;
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;
const float MAP_WIDTH = 500.0f;
const float MAP_LENGTH = 500.0f;
const float MAP_HEIGHT = 55.0f;
const float VIEW_DISTANCE = 1000.0f;
const float MAP_RESOLUTION = 1.0f;
const unsigned int NUM_STRIPS = (int)MAP_WIDTH * 5;
const unsigned int NUM_VERTS_PER_STRIP = (int)MAP_LENGTH * 2;
const unsigned int TEXTURE_SIZE = 4;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastX = SCR_WIDTH / 2;
float lastY = SCR_HEIGHT / 2;
bool firstMouse = true;
Camera camera (glm::vec3(0.0f, 0.0f, 3.0f));
glm::vec3 lightPosition(0.0f, 100.0f, 0.0f);


glm::vec3 calculateTriangleNormal(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void updateLastFrame(void);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);

int main(void)
{
    // simplex noise values
    // Lacunarity specifies the frequency multiplier between successive octaves (default to 2.0).
    // Persistence is the loss of amplitude between successive octaves (usually 1/lacunarity)
    const float amplitutde = 0.5f;
    const float scale = 50.0f;
    const float lacunarity = 1.99f;
    const float persistance = 0.5f;
    const int octaves = 5;

    const SimplexNoise simplex(0.1f / scale, 0.5f, lacunarity, persistance);
    //const int octaves = static_cast<int>(5 + std::log(scale));
    
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


    std::vector<float> planeVertices;
    std::vector<unsigned int> indices;
    int vertexIndex = 0;

    for (float x = 0; x <= MAP_WIDTH; x += MAP_RESOLUTION) {
        for (float z = 0; z <= MAP_LENGTH - MAP_RESOLUTION; z += MAP_RESOLUTION) {

            // will end up with each vertice having 8 floats
            // 3 position floats, 3 normal floats, 2 texture floats

            float x1, x2, x3, y1, y2, y3, z1, z2, z3;
            glm::vec3 a, b, c;
            glm::vec3 normal;
            float texCoordX1 = x / TEXTURE_SIZE;
            float texCoordZ1 = z / TEXTURE_SIZE;
            float texCoordX2 = (x + MAP_RESOLUTION) / TEXTURE_SIZE;
            float texCoordZ2 = (z + MAP_RESOLUTION) / TEXTURE_SIZE;

            // getting vertices from triangle one
            x1 = x;
            z1 = z;
            y1 = simplex.fractal(octaves, x1, z1) * MAP_HEIGHT;
            a = glm::vec3(x1, y1, z1);

            x2 = x;
            z2 = z + MAP_RESOLUTION;
            y2 = simplex.fractal(octaves, x2, z2) * MAP_HEIGHT;
            b = glm::vec3(x2, y2, z2);

            x3 = x + MAP_RESOLUTION;
            z3 = z + MAP_RESOLUTION;
            y3 = simplex.fractal(octaves, x3, z3) * MAP_HEIGHT;
            c = glm::vec3(x3, y3, z3);
            normal = calculateTriangleNormal(a, b, c);

            // pushing vertices from triangle one along with texture coords and normal vector
            planeVertices.push_back(x1);
            planeVertices.push_back(y1);
            planeVertices.push_back(z1);
            planeVertices.push_back(normal.x); // normalized coordinates
            planeVertices.push_back(normal.y);
            planeVertices.push_back(normal.z);
            planeVertices.push_back(texCoordX1); // texture coordinates
            planeVertices.push_back(texCoordZ1);

            planeVertices.push_back(x2);
            planeVertices.push_back(y2);
            planeVertices.push_back(z2);
            planeVertices.push_back(normal.x); // normalized coordinates
            planeVertices.push_back(normal.y);
            planeVertices.push_back(normal.z);
            planeVertices.push_back(texCoordX1); // texture coordinates
            planeVertices.push_back(texCoordZ2);

            planeVertices.push_back(x3);
            planeVertices.push_back(y3);
            planeVertices.push_back(z3);
            planeVertices.push_back(normal.x); // normalized coordinates
            planeVertices.push_back(normal.y);
            planeVertices.push_back(normal.z);
            planeVertices.push_back(texCoordX2); // texture coordinates
            planeVertices.push_back(texCoordZ2);
            
            // getting vertices from triangle 2
            x1 = x;
            z1 = z;
            y1 = simplex.fractal(octaves, x1, z1) * MAP_HEIGHT;
            a = glm::vec3(x1, y1, z1);

            x2 = x + MAP_RESOLUTION;
            z2 = z;
            y2 = simplex.fractal(octaves, x2, z2) * MAP_HEIGHT;
            b = glm::vec3(x2, y2, z2);

            x3 = x + MAP_RESOLUTION;
            z3 = z + MAP_RESOLUTION;
            y3 = simplex.fractal(octaves, x3, z3) * MAP_HEIGHT;
            c = glm::vec3(x3, y3, z3);
            // set it to negative because the normal vector gets the vector from the opposite side of the traingle
            // fromt the first calculation... need to fix this 
            normal = -calculateTriangleNormal(a, b, c);

            // pushing vertices from triangle 2 along with coord and normal info
            planeVertices.push_back(x1);
            planeVertices.push_back(y1);
            planeVertices.push_back(z1);
            planeVertices.push_back(normal.x); // normalized coordinates
            planeVertices.push_back(normal.y);
            planeVertices.push_back(normal.z);
            planeVertices.push_back(texCoordX1); // texture coordinates
            planeVertices.push_back(texCoordZ1);

            planeVertices.push_back(x2);
            planeVertices.push_back(y2);
            planeVertices.push_back(z2);
            planeVertices.push_back(normal.x); // normalized coordinates
            planeVertices.push_back(normal.y);
            planeVertices.push_back(normal.z);
            planeVertices.push_back(texCoordX1); // texture coordinates
            planeVertices.push_back(texCoordZ2);

            planeVertices.push_back(x3);
            planeVertices.push_back(y3);
            planeVertices.push_back(z3);
            planeVertices.push_back(normal.x); // normalized coordinates
            planeVertices.push_back(normal.y);
            planeVertices.push_back(normal.z);
            planeVertices.push_back(texCoordX2); // texture coordinates
            planeVertices.push_back(texCoordZ2);


            // fill vertices matrix
            // fill vertices matrix
            indices.push_back(vertexIndex + 1);
            indices.push_back(vertexIndex + 5);
            indices.push_back(vertexIndex + 2);

            // fill vertices matrix
            indices.push_back(vertexIndex + 2);
            indices.push_back(vertexIndex + 1);
            indices.push_back(vertexIndex + 5);

            vertexIndex += 6;
        }
    }

    std::cout << "Map: " << MAP_WIDTH << " x " << MAP_LENGTH << std::endl;

    std::cout << "Created lattice of " << NUM_STRIPS << " strips with " << NUM_VERTS_PER_STRIP << " triangles each" << std::endl;
    std::cout << "Created " << NUM_STRIPS * NUM_VERTS_PER_STRIP << " triangles total" << std::endl;

    std::cout << "Number of triangles: " << planeVertices.size() / (sizeof(float) * 24) << std::endl;

    std::cout << "Vertex index: " << (vertexIndex * 2) / 3<< std::endl;
    /*Configures some buffer objects*/


    // vao[1] and vbo[2] for plane mesh/terrain ... should probably give it a unique named variable
    unsigned int VAOs[2], VBOs[2], lightVAO, lightVBO;
    GLuint terrainEBO;

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

    // terrain mesh stuff ------------------------------------------------------------------
    glBindVertexArray(VAOs[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, planeVertices.size() * sizeof(float), &planeVertices[0], GL_STATIC_DRAW);

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
    glGenBuffers(1, &terrainEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    

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



    /* -------Loop until the user closes the window------------ */
    while (!glfwWindowShouldClose(window))
    {
        updateLastFrame();
        processInput(window);

        // Render here
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        lightingShader.use();
        lightingShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
        lightingShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
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
        
        // draw terrain
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3((MAP_WIDTH / 2) * -1, -10.0f, (MAP_LENGTH / 2) * -1));
        lightingShader.setMat4("model", model);
        glBindVertexArray(VAOs[1]);
        // draw terrain by strips
        for (unsigned int i = 0; i < NUM_STRIPS - 1; i++) {
            glDrawElements(GL_TRIANGLE_STRIP, NUM_VERTS_PER_STRIP, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * NUM_VERTS_PER_STRIP * i));
        }
        //glDrawArrays(GL_TRIANGLES, 0, (MAP_WIDTH - 1) * (MAP_LENGTH - 1) * 6);

        // draw light box
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        glm::translate(model, lightPosition);
        glm::scale(model, glm::vec3(0.2f));
        lightCubeShader.setMat4("model", model);
        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);


        // Swap front and back buffers & check and call events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(2, VAOs);
    glDeleteBuffers(2, VBOs);
    glDeleteVertexArrays(1, &lightVAO);

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

    camera.ProcessMouseMovement(xOffset, yOffset);
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

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        GLint polygonMode;
        glGetIntegerv(GL_POLYGON_MODE, &polygonMode);
        if (polygonMode == GL_LINE) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
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

glm::vec3 calculateTriangleNormal(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) {
    glm::vec3 edge1 = v2 - v1;
    glm::vec3 edge2 = v3 - v1;

    glm::vec3 normal = glm::cross(edge1, edge2);

    return glm::normalize(normal);
}
