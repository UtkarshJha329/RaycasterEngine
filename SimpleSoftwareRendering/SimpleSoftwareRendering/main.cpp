#include <iostream>
#include <vector>

#include <chrono>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Input.h"

#include "Instrumentor.h"
#include "Geometry.h"
#include "Model.h"
#include "RenderGeometry.h"
#include "MeshLoader.h"
#include "CameraUtils.h"

std::vector<Texture> Model::textures;

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

void UpdateKeyStates(GLFWwindow* window) {

    //---------------------------------Keyboard Input--------------------------------------

    int wKeyState = glfwGetKey(window, GLFW_KEY_W);
    SetKeyBasedOnState(KEY_W, wKeyState > 0 ? PRESSED_OR_HELD : RELEASED);

    int sKeyState = glfwGetKey(window, GLFW_KEY_S);
    SetKeyBasedOnState(KEY_S, sKeyState > 0 ? PRESSED_OR_HELD : RELEASED);

    int dKeyState = glfwGetKey(window, GLFW_KEY_D);
    SetKeyBasedOnState(KEY_D, dKeyState > 0 ? PRESSED_OR_HELD : RELEASED);

    int aKeyState = glfwGetKey(window, GLFW_KEY_A);
    SetKeyBasedOnState(KEY_A, aKeyState > 0 ? PRESSED_OR_HELD : RELEASED);

    int pKeyState = glfwGetKey(window, GLFW_KEY_P);
    SetKeyBasedOnState(KEY_P, pKeyState > 0 ? PRESSED_OR_HELD : RELEASED);

    int leftShiftKeyState = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
    SetKeyBasedOnState(KEY_LEFT_SHIFT, leftShiftKeyState > 0 ? PRESSED_OR_HELD : RELEASED);

    int leftCTRLKeyState = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL);
    SetKeyBasedOnState(KEY_LEFT_CTRL, leftCTRLKeyState > 0 ? PRESSED_OR_HELD : RELEASED);

    int spaceKeyState = glfwGetKey(window, GLFW_KEY_SPACE);
    SetKeyBasedOnState(KEY_SPACE, spaceKeyState > 0 ? PRESSED_OR_HELD : RELEASED);

    //---------------------------------------Mouse Input------------------------------------------

    int mouseButtonLeftState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    SetKeyBasedOnState(MOUSE_BUTTON_LEFT, mouseButtonLeftState > 0 ? PRESSED_OR_HELD : RELEASED);

    int mouseButtonRightState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
    SetKeyBasedOnState(MOUSE_BUTTON_RIGHT, mouseButtonRightState > 0 ? PRESSED_OR_HELD : RELEASED);

    mouseXFromPreviousFrame = mouseX;
    mouseYFromPreviousFrame = mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
}

std::string modelsPath = "Assets/Models/";
std::string testCubeFileName = "TestCube.obj";
std::string testBlenderMonkeyFileName = "Suzanne.obj";
std::string testUtahTeaPotFileName = "UtahTeapot.obj";
std::string testCubeTexturedFileName = "TestCubeTextured.obj";

bool pressedRight = false;
bool pressedLeft = false;
bool pressedUp = false;
bool pressedDown = false;

bool freezeRotation = false;
bool pressedK = false;

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

    //Mesh randomMesh;
    //LoadMeshFromOBJFile(randomMesh, modelsPath, testCubeFileName);
    //LoadMeshFromOBJFile(randomMesh, modelsPath, testBlenderMonkeyFileName);
    //LoadMeshFromOBJFile(randomMesh, modelsPath, testUtahTeaPotFileName);
    //LoadMeshFromOBJFile(randomMesh, modelsPath, testCubeTexturedFileName);

    Colour backgroundColour = { 255, 255, 255, 255 };
    Colour red = { 255, 0, 0, 255 };
    Colour green = { 0, 255, 0, 255 };
    Colour blue = { 0, 0, 255, 255 };
    Colour playerColour = { 255, 0, 0, 255 };

    Mat4x4 perspectiveProjectionMatrix = glm::perspectiveFovRH_NO(glm::radians(fov * 0.5f), (float)SCR_WIDTH, (float)SCR_HEIGHT, distToNearPlane, distToFarPlane);
    //Mat4x4 perspectiveProjectionMatrix = glm::perspectiveFovRH_ZO(glm::radians(fov * 0.5f), (float)SCR_WIDTH, (float)SCR_HEIGHT, distToNearPlane, distToFarPlane);

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

    //Any key press will flip the key state to pressed until it is checked. If key is released during this time, once it is checked it will flip back to released.
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

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

    Vector3 worldUP = Vector3{ 0.0f, 1.0f, 0.0f };
    Vector3 worldForward = Vector3{ 0.0f, 0.0f, 1.0f };

    Vector3 objectPosition = { 0.0f, 0.0f, 4.0f };

    Vector3 cameraPosition = Vector3{ 0.0f, -3.0f, 0.0f };
    Vector3 cameraTargetPosition = cameraPosition + worldForward;
    Vector3 cameraLookingDirection = glm::normalize(cameraTargetPosition - cameraPosition);
    Vector3 cameraRightDirection = glm::cross({ 0.0f, 1.0f, 0.0f }, cameraLookingDirection);
    Vector3 cameraUpDirection = glm::cross(cameraLookingDirection, cameraRightDirection);
    Mat4x4 cameraViewMatrix = Mat4x4(0.0f);

    float cameraXRot = 0.0f;
    float cameraYRot = 0.0f;
    float clampCameraXRot = 45.0f;
    float clampCameraYRot = 360.0f;
    float cameraMoveSpeed = 10.0f;
    float cameraRotSpeed = 1.0f;

    Vector3 rotationAxis = { 0.0f, 0.0f, 0.0f };

    float angle = 0.0f;
    float rotationSpeed = 10.0f;

    Model testCubeModel;
    //LoadModel(modelsPath + testCubeFileName, testCubeModel);
    //LoadModel(modelsPath + testUtahTeaPotFileName, testCubeModel);
    //LoadModel(modelsPath + testBlenderMonkeyFileName, testCubeModel);
    LoadModel(modelsPath + testCubeTexturedFileName, testCubeModel);

    //for (int i = 0; i < Model::textures.size(); i++)
    //{
    //    Texture* curTexture = &Model::textures[i];
    //    std::cout << curTexture->data.size() << std::endl;
    //    //std::cout << curTexture->width << ", " << curTexture->height << std::endl;
    //    for (int y = 0; y < curTexture->height; y++)
    //    {
    //        std::cout << "Y := " << y << std::endl;
    //        for (int x = 0; x < curTexture->width; x++)
    //        {
    //            //std::cout << x << std::endl;
    //            Colour curColourOfTexture = GetColourFromTexCoord(*curTexture, Vector2{ x / (float)curTexture->width, y / (float)curTexture->height});
    //            //std::cout << (int)curColourOfTexture.r << ", " << (int)curColourOfTexture.g << ", " << (int)curColourOfTexture.b << std::endl;
    //            std::cout << (int)curColourOfTexture.r << std::endl;
    //        }
    //    }
    //}

    //Texture testTexture;
    ////LoadTextureFromFile("Assets/Models/testImage.jpg", testTexture);
    //LoadTextureFromFile("Assets/Models/SimpleUV05.png", testTexture);

    auto previousTime = std::chrono::high_resolution_clock::now();
    while (!glfwWindowShouldClose(window))
    {
        PROFILE_SCOPE("GAME LOOP.");

        UpdateKeyStates(window);

        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> difBetweenPreviousFrameTimeAndCurrentTime = currentTime - previousTime;
        float deltaTime = difBetweenPreviousFrameTimeAndCurrentTime.count() / 1000.0f;
        //std::cout << deltaTime.count() << std::endl;

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ClearImage(imageData, SCR_WIDTH, SCR_HEIGHT, backgroundColour);
        ClearImageDepth(imageDepthData, SCR_WIDTH, SCR_HEIGHT, 1.0f);

        freezeRotation = (GetKeyHeld(KEY_P));

        if (!freezeRotation) {
            angle -= rotationSpeed * deltaTime;
            if (angle <= 0.0f) {
                angle += 360.0f;
            }
        }

        if (GetKeyHeld(MOUSE_BUTTON_RIGHT)) {

            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            float camMoveSpeed = cameraMoveSpeed;

            if (GetKeyHeld(KEY_LEFT_SHIFT)) {
                camMoveSpeed = 0.1f;
            }

            if (GetKeyHeld(KEY_W)) {
                cameraPosition += cameraLookingDirection * camMoveSpeed * deltaTime;
            }
            if (GetKeyHeld(KEY_S)) {
                cameraPosition -= cameraLookingDirection * camMoveSpeed * deltaTime;
            }

            if (GetKeyHeld(KEY_D)) {
                cameraPosition -= cameraRightDirection * camMoveSpeed * deltaTime;
            }
            if (GetKeyHeld(KEY_A)) {
                cameraPosition += cameraRightDirection * camMoveSpeed * deltaTime;
            }

            if (GetKeyHeld(KEY_SPACE)) {
                cameraPosition.y -= camMoveSpeed * deltaTime;
            }
            if (GetKeyHeld(KEY_LEFT_CTRL)) {
                cameraPosition.y += camMoveSpeed * deltaTime;
            }
            cameraTargetPosition = cameraPosition + cameraLookingDirection;

            float mouseDeltaX = mouseX - mouseXFromPreviousFrame;
            float mouseDeltaY = mouseY - mouseYFromPreviousFrame;

            cameraXRot -= cameraRotSpeed * mouseDeltaY * deltaTime;
            cameraYRot -= cameraRotSpeed * mouseDeltaX * deltaTime;

            if (glm::abs(cameraXRot) > glm::radians(clampCameraXRot)) {
                cameraXRot = glm::sign(cameraXRot) * glm::radians(clampCameraXRot);
            }
            if (glm::abs(cameraYRot) >= glm::radians(clampCameraYRot)) {
                cameraYRot = 0.0f;
            }            
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        Mat4x4 modelMat = glm::identity<Mat4x4>();
        modelMat = glm::translate(modelMat, objectPosition);
        modelMat = glm::rotate(modelMat, glm::radians(angle), Vector3{ 0.0f, 1.0f, 0.0f });
        modelMat = glm::scale(modelMat, Vector3{ 1.0f, 1.0f, 1.0f });


        Mat4x4 cameraTransformMatrix    = glm::identity<Mat4x4>();
        cameraTransformMatrix           = glm::translate(cameraTransformMatrix, cameraPosition);

        Mat4x4 cameraRotationMatrix     = glm::identity<Mat4x4>();
        cameraRotationMatrix            = glm::rotate(cameraRotationMatrix, cameraXRot, cameraRightDirection);
        cameraRotationMatrix            = glm::rotate(cameraRotationMatrix, cameraYRot, worldUP);

        cameraTransformMatrix           =  cameraTransformMatrix * cameraRotationMatrix;

        cameraTransformMatrix           = glm::scale(cameraTransformMatrix, { 1.0f, 1.0f, 1.0f });

        cameraViewMatrix = glm::inverse(cameraTransformMatrix);

        cameraLookingDirection = Vector3{ cameraRotationMatrix * Vector4{0.0f, 0.0f, 1.0f, 1.0f } };
        cameraRightDirection = glm::cross({ 0.0f, 1.0f, 0.0f }, cameraLookingDirection);


        //DrawMeshOnScreenFromWorldWithTransform(imageData, imageDepthData, SCR_WIDTH, SCR_HEIGHT, randomMesh, modelMat, cameraPosition, cameraLookingDirection, cameraViewMatrix, perspectiveProjectionMatrix, lineThickness, red);
        DrawMeshOnScreenFromWorldWithTransform(imageData, imageDepthData, SCR_WIDTH, SCR_HEIGHT, testCubeModel.meshes[0], modelMat, cameraPosition, cameraLookingDirection, cameraViewMatrix, perspectiveProjectionMatrix, lineThickness, red);

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData.data());
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Model::textures[0].width, Model::textures[0].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Model::textures[0].data.data());
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, testTexture.width, testTexture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, testTexture.data.data());
        glGenerateMipmap(GL_TEXTURE_2D);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();

        frame++;
        previousTime = currentTime;
        ResetKeysReleased();
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
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}