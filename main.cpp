// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <iostream>
#include <vector>

// Include GLEW
#include <glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// GLM header file
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
using namespace glm;

// shaders header file
#include <common/shader.hpp>

// Vertex array object (VAO)
GLuint VertexArrayID;

// Vertex buffer object (VBO)
GLuint vertexbuffer;

// color buffer object (CBO)
GLuint colorbuffer;

// GLSL program from the shaders
GLuint programID;

#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 900

std::vector<glm::vec2> points;


glm::vec2 sumTuplo(const glm::vec2& tuplo1, const glm::vec2& tuplo2) {
    return tuplo1 + tuplo2;
}

glm::vec2 multTuplo(float scalar, const glm::vec2& tuplo) {
    return scalar * tuplo;
}

std::vector<glm::vec2> interpolacao(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3) {
    std::vector<glm::vec2> pontos = { p1, p2, p3 };
    glm::vec2 p1lin = p1;
    glm::vec2 p2lin = p2;
    glm::vec2 p3lin = p3;

    for (int x = 0; x < 20; x++) {
        float t = 0.05f;
        p1lin = sumTuplo(multTuplo((1 - t), p1lin), multTuplo(t, p2lin));
        p2lin = sumTuplo(multTuplo((1 - t), p2lin), multTuplo(t, p3lin));
        p3lin = sumTuplo(multTuplo((1 - t), p3lin), multTuplo(t, p1lin));

        pontos.push_back(p1lin);
        pontos.push_back(p2lin);
        pontos.push_back(p3lin);
       
    }
    return pontos;
}


std::vector<glm::vec2> verticesPent(float radius) {
    std::vector<glm::vec2> vertices;
    float angle = glm::radians(360.0f / 5.0f);
    for (int i = 0; i < 5; i++) {
        float theta = i * angle;
        float x = radius * cos(theta);
        float y = radius * sin(theta);
        vertices.push_back(glm::vec2(x, y));
    }
    return vertices;
}


//--------------------------------------------------------------------------------
void transferDataToGPUMemory(void) {

    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);


    programID = LoadShaders(
        "./SimpleVertexShader.vertexshader",
        "./SimpleFragmentShader.fragmentshader");


    float pentagonRadius = 0.5f;
    std::vector<glm::vec2> verticesPentagono = verticesPent(pentagonRadius);


    glm::vec2 center(0.0f, 0.0f);


    for (int i = 0; i < 5; i++) {
        glm::vec2 v1 = verticesPentagono[i];
        glm::vec2 v2 = verticesPentagono[(i + 1) % 5];
        std::vector<glm::vec2> interPoints = interpolacao(center, v1, v2);
        points.insert(points.end(), interPoints.begin(), interPoints.end());
    }

    // Converter os pontos do vetor pontos para VBO
    std::vector<GLfloat> vertexBuffer;
    for (const auto& point : points) {
        vertexBuffer.push_back(point.x);
        vertexBuffer.push_back(point.y);
        vertexBuffer.push_back(0.0f);
    }

     glGenBuffers(1, &vertexbuffer);
     glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
     glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size() * sizeof(GLfloat), vertexBuffer.data(), GL_STATIC_DRAW);

}
//--------------------------------------------------------------------------------
void cleanupDataFromGPU(){
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &colorbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteProgram(programID);
}

//--------------------------------------------------------------------------------
void draw(void) {
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT);

    // Use our shader
    glUseProgram(programID);

    glm::mat4 mvp = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f);
    unsigned int matrix = glGetUniformLocation(programID, "mvp");
    glUniformMatrix4fv(matrix, 1, GL_FALSE, &mvp[0][0]);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
        0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );

    // 2nd attribute buffer : colors
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glVertexAttribPointer(
        1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
        3,                                // size
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );

    glEnable(GL_PROGRAM_POINT_SIZE);

    glLineWidth(2.0f);
    
    glDrawArrays(GL_LINE_STRIP, 0, points.size());

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

//--------------------------------------------------------------------------------
int main(void) {
    GLFWwindow* window;

    // Initialize the library
    if (!glfwInit()) {
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pentagon with Interpolated Triangles", NULL, NULL);

    if (!window) {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    glewExperimental = true;
    // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // Transfer data (vertices, colors, and shaders) to GPU side
    transferDataToGPUMemory();

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window)) {
        draw();

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Cleanup VAO, VBOs, and shaders from GPU
    cleanupDataFromGPU();

    glfwTerminate();

    return 0;
}
