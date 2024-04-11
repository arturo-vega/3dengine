#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader.h"

#include <iostream>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main(void)
{
    // Initialize and configure library
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

    // Initialize GLAD and load all OpenGL function pointers
    // Note: GLAD points to where all the OpenGL functions are on the computer, not strictly necessary
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // Build and compile shader from shader.h
    Shader myShader("../ball_game/src/vertexshader.vs", "../ball_game/src/fragmentshader.fs");

    float vertices[] = {
        // positions         // colors
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom left
         0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f   // top 
    };


    /*
    Personal notes
    VBO = Vertex Buffer Object, sends vertices all at once to the GPU
    VAO = Vertex Array Object, creates and stores all attribute calls into the VAO
    so that you can switch between different VAO's without having to set the VBO each
    time.
    EBO = Element Buffer Object, buffer that stores indices that OpenGL can use to decide
    which verticies to draw

    VBO's and VAO's are necessary but EBO's are not but can eliminate unecessary vertexes

    Can generate multiple buffers at at ime
    */

    /*Configures some buffer objects*/
    
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    /*Sets the color of the background*/
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    /* Viewport and automatic resizing when window size is changed */
    //glViewport(0, 0, 800, 600);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    /* -------Loop until the user closes the window------------ */
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        myShader.use();
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);


        /* Swap front and back buffers & check and call events */
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}