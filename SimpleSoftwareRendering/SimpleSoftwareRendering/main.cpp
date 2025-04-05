#include <iostream>
#include <vector>

#include <chrono>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Instrumentor.h"
#include "Geometry.h"
#include "RenderGeometry.h"
#include "MeshLoader.h"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec2 aTexCoord;\n"
"out vec2 TexCoord;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"	TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
"}\0";

const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec2 TexCoord;\n"
"uniform sampler2D ourTexture;\n"
"void main()\n"
"{\n"
"    FragColor = texture(ourTexture, TexCoord);\n"
"}\n\0";

void ClearImage(std::vector<unsigned char>& imageData, int width, int height, Colour clearColour) {

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int curIndex = (x + (y * width)) * NUM_COMPONENTS_IN_PIXEL;
            imageData[curIndex + 0] = clearColour.r;
            imageData[curIndex + 1] = clearColour.g;
            imageData[curIndex + 2] = clearColour.b;
            imageData[curIndex + 3] = clearColour.a;
        }
    }
}

void ClearImageDepth(std::vector<float>& imageDepthData, int width, int height, float clearValue) {

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int curIndex = GetFlattenedImageDataSlotForDepthData(Vector2Int{ x, y }, width);
            imageDepthData[curIndex] = clearValue;
        }
    }
}

std::string modelsPath = "Assets/Models/";
std::string testCubeFileName = "TestCube.obj";
std::string testBlenderMonkeyFileName = "Suzanne.obj";

bool pressedRight = false;
bool pressedLeft = false;
bool pressedUp = false;
bool pressedDown = false;

bool freezeRotation = false;

const int lineThickness = 2;

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const float distToNearPlane = 0.1f;
const float distToFarPlane = 1000.0f;
const float fov = 90.0f;
const float aspectRatio = (float)SCR_WIDTH / (float)SCR_HEIGHT;
const float aspectRatioR = (float)SCR_HEIGHT / (float)SCR_WIDTH;
const float oneOverFOV = 1.0f / glm::tan(glm::radians(fov * 0.5f));

int main()
{
    Instrumentor::Instance().BeginSession("Simple Software Renderer.");

    Mesh randomMesh;
    //LoadMeshFromOBJFile(randomMesh, modelsPath, testCubeFileName);
    LoadMeshFromOBJFile(randomMesh, modelsPath, testBlenderMonkeyFileName);

    Colour backgroundColour = { 255, 255, 255, 255 };
    Colour red = { 255, 0, 0, 255 };
    Colour green = { 0, 255, 0, 255 };
    Colour blue = { 0, 0, 255, 255 };
    Colour playerColour = { 255, 0, 0, 255 };

    Mat4x4 perspectiveProjectionMatrix = glm::perspective(glm::radians(fov * 0.5f), aspectRatio, distToNearPlane, distToFarPlane);
    //Mat4x4 perspectiveProjectionMatrix = glm::mat4(1.0);
    //Mat4x4 calculatedPerspectiveProjectionMatrix = glm::mat4(0.0f);
    //calculatedPerspectiveProjectionMatrix[0][0] = aspectRatioR * oneOverFOV;
    //calculatedPerspectiveProjectionMatrix[1][1] = oneOverFOV;
    //calculatedPerspectiveProjectionMatrix[2][2] = distToFarPlane / (distToFarPlane - distToNearPlane);
    //calculatedPerspectiveProjectionMatrix[3][2] = (-distToFarPlane * distToNearPlane) / (distToFarPlane - distToNearPlane);
    //calculatedPerspectiveProjectionMatrix[2][3] = 1.0f;
    //calculatedPerspectiveProjectionMatrix[3][3] = 0.0f;

    std::vector<unsigned char> imageData(SCR_WIDTH * SCR_HEIGHT * NUM_COMPONENTS_IN_PIXEL);
    std::vector<float> imageDepthData(SCR_WIDTH * SCR_HEIGHT);
    ClearImage(imageData, SCR_WIDTH, SCR_HEIGHT, backgroundColour);
    ClearImageDepth(imageDepthData, SCR_WIDTH, SCR_HEIGHT, 0.0f);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    float vertices[] = {
        // positions            // texture coords
         1.0f,  1.0f, 0.0f,     1.0f, 1.0f,         // top right
         1.0f, -1.0f, 0.0f,     1.0f, 0.0f,         // bottom right
        -1.0f, -1.0f, 0.0f,     0.0f, 0.0f,         // bottom left
        -1.0f,  1.0f, 0.0f,     0.0f, 1.0f          // top left 
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    int frame = 0;

    int curPosX = SCR_WIDTH / 2;
    int curPosY = SCR_HEIGHT / 2;

    //float zCoord = 0.0f;
    //Triangle simpleWorldTriangle = Triangle{ Point{{1.0f, 0.0f, zCoord}}, Point{{0.0f, 1.0f, zCoord}}, Point{{-1.0f, 0.0f, zCoord}} };
    ////LineSegment simpleWorldLineSegment = LineSegment{ Point{{10.0f, 0.0f, 0.0f}}, Point{{-10.0f, 0.0f, 0.0f}} };

    //Vector3 trianglePosition = { 0.0f, 0.0f, 10.0f };

    //Mesh simpleTriangle =
    //{
    //    { 
    //        // SOUTH
    //        //{ Point{{0.0f, 0.0f, 0.0f}},    Point{{0.0f, 1.0f, 0.0f}},    Point{{1.0f, 1.0f, 0.0f}} },
    //        //{ Point{{0.0f, 0.0f, 0.0f}},    Point{{1.0f, 1.0f, 0.0f}},    Point{{1.0f, 0.0f, 0.0f}} },

    //        //// EAST                                                      
    //        //{ Point{{1.0f, 0.0f, 0.0f}},    Point{{1.0f, 1.0f, 0.0f}},    Point{{1.0f, 1.0f, 1.0f}} },
    //        //{ Point{{1.0f, 0.0f, 0.0f}},    Point{{1.0f, 1.0f, 1.0f}},    Point{{1.0f, 0.0f, 1.0f}} },

    //        //// NORTH                                                     
    //        //{ Point{{1.0f, 0.0f, 1.0f}},    Point{{1.0f, 1.0f, 1.0f}},    Point{{0.0f, 1.0f, 1.0f}} },
    //        //{ Point{{1.0f, 0.0f, 1.0f}},    Point{{0.0f, 1.0f, 1.0f}},    Point{{0.0f, 0.0f, 1.0f}} },

    //        //// WEST                                                      
    //        //{ Point{{0.0f, 0.0f, 1.0f}},    Point{{0.0f, 1.0f, 1.0f}},    Point{{0.0f, 1.0f, 0.0f}} },
    //        //{ Point{{0.0f, 0.0f, 1.0f}},    Point{{0.0f, 1.0f, 0.0f}},    Point{{0.0f, 0.0f, 0.0f}} },

    //        //// TOP                                                       
    //        //{ Point{{0.0f, 1.0f, 0.0f}},    Point{{0.0f, 1.0f, 1.0f}},    Point{{1.0f, 1.0f, 1.0f}} },
    //        //{ Point{{0.0f, 1.0f, 0.0f}},    Point{{1.0f, 1.0f, 1.0f}},    Point{{1.0f, 1.0f, 0.0f}} },

    //        //// BOTTOM                                                    
    //        { Point{{1.0f, 0.0f, 1.0f}},    Point{{0.0f, 0.0f, 1.0f}},    Point{{0.0f, 0.0f, 0.0f}} },
    //        { Point{{1.0f, 0.0f, 1.0f}},    Point{{0.0f, 0.0f, 0.0f}},    Point{{1.0f, 0.0f, 0.0f}} },
    //    }
    //};
    //Vector3 simpleTrianglePosition = { 0.0f, 0.0f, 5.0f };

    //Mesh cubeMesh = 
    //{
    //    {
    //        // SOUTH
    //        { Point{{0.0f, 0.0f, 0.0f}},    Point{{0.0f, 1.0f, 0.0f}},    Point{{1.0f, 1.0f, 0.0f}} },
    //        { Point{{0.0f, 0.0f, 0.0f}},    Point{{1.0f, 1.0f, 0.0f}},    Point{{1.0f, 0.0f, 0.0f}} },

    //        // EAST                                                      
    //        { Point{{1.0f, 0.0f, 0.0f}},    Point{{1.0f, 1.0f, 0.0f}},    Point{{1.0f, 1.0f, 1.0f}} },
    //        { Point{{1.0f, 0.0f, 0.0f}},    Point{{1.0f, 1.0f, 1.0f}},    Point{{1.0f, 0.0f, 1.0f}} },

    //        // NORTH                                                     
    //        { Point{{1.0f, 0.0f, 1.0f}},    Point{{1.0f, 1.0f, 1.0f}},    Point{{0.0f, 1.0f, 1.0f}} },
    //        { Point{{1.0f, 0.0f, 1.0f}},    Point{{0.0f, 1.0f, 1.0f}},    Point{{0.0f, 0.0f, 1.0f}} },

    //        // WEST                                                      
    //        { Point{{0.0f, 0.0f, 1.0f}},    Point{{0.0f, 1.0f, 1.0f}},    Point{{0.0f, 1.0f, 0.0f}} },
    //        { Point{{0.0f, 0.0f, 1.0f}},    Point{{0.0f, 1.0f, 0.0f}},    Point{{0.0f, 0.0f, 0.0f}} },

    //        // TOP                                                       
    //        { Point{{0.0f, 1.0f, 0.0f}},    Point{{0.0f, 1.0f, 1.0f}},    Point{{1.0f, 1.0f, 1.0f}} },
    //        { Point{{0.0f, 1.0f, 0.0f}},    Point{{1.0f, 1.0f, 1.0f}},    Point{{1.0f, 1.0f, 0.0f}} },

    //        // BOTTOM                                                    
    //        { Point{{1.0f, 0.0f, 1.0f}},    Point{{0.0f, 0.0f, 1.0f}},    Point{{0.0f, 0.0f, 0.0f}} },
    //        { Point{{1.0f, 0.0f, 1.0f}},    Point{{0.0f, 0.0f, 0.0f}},    Point{{1.0f, 0.0f, 0.0f}} },
    //    }
    //};

    Vector3 cubePosition = { 0.0f, 0.0f, 10.0f };

    Vector3 cameraPosition = { 0.0f, 0.0f, 0.0f };

    Vector3 rotationAxis = { 0.0f, 0.0f, 0.0f };

    float angle = 0.0f;

    auto previousTime = std::chrono::high_resolution_clock::now();
    while (!glfwWindowShouldClose(window))
    {
        PROFILE_SCOPE("GAME LOOP.");

        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> deltaTime = currentTime - previousTime;
        //std::cout << deltaTime.count() << std::endl;

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ClearImage(imageData, SCR_WIDTH, SCR_HEIGHT, backgroundColour);
        ClearImageDepth(imageDepthData, SCR_WIDTH, SCR_HEIGHT, 0.0f);

        if (!freezeRotation) {
            angle += (float)deltaTime.count() * 0.1f;
            if (angle >= 360.0f) {
                angle -= 360.0f;
            }
        }

        Mat4x4 modelMat = glm::identity<Mat4x4>();
        modelMat = glm::translate(modelMat, cubePosition);
        //modelMat = glm::translate(modelMat, simpleTrianglePosition);
        modelMat = glm::rotate(modelMat, glm::radians(angle), Vector3{ 0.0f, 1.0f, 0.0f });
        //modelMat = glm::rotate(modelMat, glm::radians(90.0f), Vector3{ 1.0f, 0.0f, 1.0f });
        modelMat = glm::scale(modelMat, Vector3{ 1.0f, 1.0f, 1.0f });

        //DrawMeshOnScreenFromWorldWithTransform(imageData, SCR_WIDTH, SCR_HEIGHT, simpleTriangle, modelMat, cameraPosition, perspectiveProjectionMatrix, lineThickness, red);
        DrawMeshOnScreenFromWorldWithTransform(imageData, imageDepthData, SCR_WIDTH, SCR_HEIGHT, randomMesh, modelMat, cameraPosition, perspectiveProjectionMatrix, lineThickness, red);

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData.data());
        glGenerateMipmap(GL_TEXTURE_2D);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();

        frame++;
        previousTime = currentTime;
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();

    Instrumentor::Instance().EndSession();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    pressedRight = pressedRight ? (key == GLFW_KEY_RIGHT && action != GLFW_RELEASE) : (key == GLFW_KEY_RIGHT && action == GLFW_PRESS);
    pressedLeft = pressedLeft ? (key == GLFW_KEY_LEFT && action != GLFW_RELEASE) : (key == GLFW_KEY_LEFT && action == GLFW_PRESS);
    pressedUp = pressedUp ? (key == GLFW_KEY_UP && action != GLFW_RELEASE) : (key == GLFW_KEY_UP && action == GLFW_PRESS);
    pressedDown = pressedDown ? (key == GLFW_KEY_DOWN && action != GLFW_RELEASE) : (key == GLFW_KEY_DOWN && action == GLFW_PRESS);

    freezeRotation = freezeRotation ? (key == GLFW_KEY_P && action != GLFW_RELEASE) : (key == GLFW_KEY_P && action == GLFW_PRESS);

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

/*
        if (frame % 10 == 0) {

            int oldPixelLocation = (curPosX + (curPosY * SCR_WIDTH)) * NUM_COMPONENTS_IN_PIXEL;
            FillSubPixels(imageData, curPosX, curPosY, 50, backgroundColour);

            if (pressedRight) {
                //std::cout << " Moved right." << curPosX << std::endl;
                curPosX++;
            }
            if (pressedLeft) {
                //std::cout << " Moved left." << curPosX << std::endl;
                curPosX--;
            }
            if (pressedUp) {
                //std::cout << " Moved up." << curPosY << std::endl;
                curPosY++;
            }
            if (pressedDown) {
                //std::cout << " Moved down." << curPosY << std::endl;
                curPosY--;
            }

            int curPixelLocation = (curPosX + (curPosY * SCR_WIDTH)) * NUM_COMPONENTS_IN_PIXEL;
            FillSubPixels(imageData, curPosX, curPosY, 50, playerColour);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData.data());
            glGenerateMipmap(GL_TEXTURE_2D);
        }
*/