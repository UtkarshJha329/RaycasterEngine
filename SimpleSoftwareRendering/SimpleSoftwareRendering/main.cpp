#include <iostream>
#include <vector>

#include <chrono>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Input.h"

#include "Instrumentor.h"
#include "Geometry.h"
#include "UIGeometry.h"
#include "Model.h"
#include "RenderGeometry.h"
#include "RenderUI.h"
#include "MeshLoader.h"
#include "CameraUtils.h"

std::vector<Texture> Model::textures;
std::vector<UI_Rect> UI_Rect::uiRects;
std::vector<std::vector<unsigned int>> UI_CollisionGrid::uiRectIndexInCollisionGrid(numGridsOnScreen.x * numGridsOnScreen.y);

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

    int iKeyState = glfwGetKey(window, GLFW_KEY_I);
    SetKeyBasedOnState(KEY_I, iKeyState > 0 ? PRESSED_OR_HELD : RELEASED);

    int oKeyState = glfwGetKey(window, GLFW_KEY_O);
    SetKeyBasedOnState(KEY_O, oKeyState > 0 ? PRESSED_OR_HELD : RELEASED);

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
std::string truckTexturedFileName = "Truck/Truck.obj";
std::string utahTeapotTexturedFileName = "UtahTeapot/UtahTeapot.obj";
std::string planeTexturedFileName = "TexturedPlane/TexturedPlane.obj";
std::string colouredCubeFileName = "ColouredCube/ColouredCube.obj";
std::string colouredAndTexturedCubeFileName = "ColouredAndTexturedCube/ColouredAndTexturedCube.obj";
std::string monu2FileName = "Monu2MagicaVoxel/monu2.obj";
std::string LowPolyForestTerrainFileName = "LowPolyForestTerrain/LowPolyForestTerrain.obj";
std::string texturedSuzanneFileName = "TexturedSuzanne/TexturedSuzanne.obj";

bool freezeRotation = true;

const int lineThickness = 2;

const float distToNearPlane = 0.1f;
const float distToFarPlane = 1000.0f;
const float fov = 90.0f;
const float aspectRatio = (float)screenWidth / (float)screenHeight;
const float aspectRatioR = (float)screenHeight / (float)screenWidth;
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

    Mat4x4 perspectiveProjectionMatrix = glm::perspectiveFovRH_NO(glm::radians(fov * 0.5f), (float)screenWidth, (float)screenHeight, distToNearPlane, distToFarPlane);
    //Mat4x4 perspectiveProjectionMatrix = glm::perspectiveFovRH_ZO(glm::radians(fov * 0.5f), (float)SCR_WIDTH, (float)SCR_HEIGHT, distToNearPlane, distToFarPlane);

    std::vector<unsigned char> imageData(screenWidth * screenHeight * NUM_COMPONENTS_IN_PIXEL);
    std::vector<float> imageDepthData(screenWidth * screenHeight);
    ClearImage(imageData, screenWidth, screenHeight, backgroundColour);
    ClearImageDepth(imageDepthData, screenWidth, screenHeight, 0.0f);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "LearnOpenGL", NULL, NULL);
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    int frame = 0;

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
    float rotationSpeedDelta = 100.0f;

    Model testModel;
    //Model eyeballModel;
    //LoadModel(modelsPath + testCubeFileName, testCubeModel);
    //LoadModel(modelsPath + testUtahTeaPotFileName, testCubeModel);
    //LoadModel(modelsPath + testBlenderMonkeyFileName, testCubeModel);
    //LoadModel(modelsPath + testCubeTexturedFileName, testCubeModel);
    //LoadModel(modelsPath + truckTexturedFileName, testCubeModel);
    //LoadModel(modelsPath + colouredCubeFileName, testCubeModel);
    //LoadModel(modelsPath + colouredAndTexturedCubeFileName, testCubeModel);
    //LoadModel(modelsPath + planeTexturedFileName, testCubeModel);
    //LoadModel(modelsPath + utahTeapotTexturedFileName, testCubeModel);
    //LoadModel(modelsPath + monu2FileName, testCubeModel);
    //LoadModel(modelsPath + LowPolyForestTerrainFileName, testCubeModel);
    LoadModel(modelsPath + texturedSuzanneFileName, testModel);


    const int rootUIRectIndex = UI_Rect::uiRects.size();
    UI_Rect::uiRects.push_back({ rootUIRectIndex, { 10.0f, 10.0f, 0.0f }, { 400.0f, 500.0f, 0.0f }, { colour_red.r, colour_red.g, colour_red.b, colour_red.a }, MiddleMiddle });
    UI_Rect::uiRects[rootUIRectIndex].normalColour = colour_red;
    UI_Rect::uiRects[rootUIRectIndex].mouseHoverColour = colour_green;

    int numChildrenUIRects = 4;
    int numChildrenForChildren = 4;

    for (int i = 0; i < numChildrenUIRects; i++)
    {
        UI_Rect someChildUIRect;
        someChildUIRect.start = { 0.0f, 0.0f, 0.0f };
        someChildUIRect.end = { 150.0f, 150.0f, 0.0f };
        someChildUIRect.index = UI_Rect::uiRects.size();
        //someChildUIRect.colour = ColourToVector4(colour_yellow);

        someChildUIRect.normalColour = colour_yellow;
        someChildUIRect.mouseHoverColour = colour_red;

        if (i == 0) {
            someChildUIRect.anchorPosition = TopLeft;
            //someChildUIRect.colour = ColourToVector4(colour_blue);
        }
        else if (i == 1) {
            someChildUIRect.anchorPosition = TopRight;
            //someChildUIRect.colour = ColourToVector4(colour_red);
        }
        else if (i == 2) {
            someChildUIRect.anchorPosition = BottomLeft;
            //someChildUIRect.colour = ColourToVector4(colour_green);
        }
        else if (i == 3) {
            someChildUIRect.anchorPosition = BottomRight;
            //someChildUIRect.colour = ColourToVector4(colour_black);
        }


        UI_Rect::uiRects[rootUIRectIndex].children.push_back(someChildUIRect.index);
        UI_Rect::uiRects.push_back(someChildUIRect);
    }

    for (int i = 0; i < numChildrenUIRects; i++)
    {
        for (int j = 0; j < numChildrenForChildren; j++)
        {
            UI_Rect someChildUIRect;
            someChildUIRect.start = { 0.0f, 0.0f, 0.0f };
            someChildUIRect.end = { 50.0f, 50.0f, 0.0f };
            //someChildUIRect.colour = { colour_blue.r, colour_blue.g, colour_blue.b, colour_blue.a };
            someChildUIRect.index = UI_Rect::uiRects.size();

            someChildUIRect.normalColour = colour_blue;
            someChildUIRect.mouseHoverColour = colour_pink;

            if (j == 0) {
                someChildUIRect.anchorPosition = TopMiddle;
            }
            else if (j == 1) {
                someChildUIRect.anchorPosition = MiddleRight;
            }
            else if (j == 2) {
                someChildUIRect.anchorPosition = BottomMiddle;
            }
            else if (j == 3) {
                someChildUIRect.anchorPosition = MiddleLeft;
            }

            //rootUIRect.children[i].children.push_back(someChildUIRect);
            UI_Rect::uiRects[UI_Rect::uiRects[rootUIRectIndex].children[i]].children.push_back(someChildUIRect.index);
            UI_Rect::uiRects.push_back(someChildUIRect);

        }
    }

    for (int i = 0; i < UI_Rect::uiRects.size(); i++)
    {
        UI_Rect::uiRects[i].colour = ColourToVector4(UI_Rect::uiRects[i].normalColour);
    }

    RenderUITree(UI_Rect::uiRects[rootUIRectIndex], screenWidth, screenHeight, imageData);
    AddUITreeToCollisionGrid();

    auto previousTime = std::chrono::high_resolution_clock::now();
    while (!glfwWindowShouldClose(window))
    {
        PROFILE_SCOPE("GAME LOOP.");

        //std::cout << "Running." << std::endl;

        UpdateKeyStates(window);

        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> difBetweenPreviousFrameTimeAndCurrentTime = currentTime - previousTime;
        float deltaTime = difBetweenPreviousFrameTimeAndCurrentTime.count() / 1000.0f;
        //std::cout << deltaTime * 1000.0f << " ms." << std::endl;

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ClearImage(imageData, screenWidth, screenHeight, backgroundColour);
        ClearImageDepth(imageDepthData, screenWidth, screenHeight, 0.0f);

        //freezeRotation = (GetKeyHeld(KEY_P));
        if (GetKeyPressedInThisFrame(KEY_P)) {
            freezeRotation = !freezeRotation;
        }

        if (GetKeyPressedInThisFrame(KEY_I)) {
            rotationSpeed += rotationSpeedDelta;
        }

        if (GetKeyPressedInThisFrame(KEY_O)) {
            rotationSpeed -= rotationSpeedDelta;
        }

        if (!freezeRotation) {
            //angle -= rotationSpeed * deltaTime;
            //if (angle <= 0.0f) {
            //    angle += 360.0f;
            //}
            angle += rotationSpeed * deltaTime;
            if (angle >= 360.0f) {
                angle = 0.0f;
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

        {
            //PROFILE_SCOPE("MODEL, VIEW AND PROJECTION MATRIX CREATION");

            Mat4x4 modelMat = glm::identity<Mat4x4>();
            modelMat = glm::translate(modelMat, objectPosition);
            modelMat = glm::rotate(modelMat, glm::radians(angle), Vector3{ 0.0f, 1.0f, 0.0f });
            modelMat = glm::scale(modelMat, Vector3{ 1.0f, 1.0f, 1.0f });

            //std::cout << std::endl;
            //PrintMat4x4Pos0(modelMat);
            //PrintMat4x4(modelMat);
            //std::cout << std::endl;

            Mat4x4 cameraTransformMatrix = glm::identity<Mat4x4>();
            cameraTransformMatrix = glm::translate(cameraTransformMatrix, cameraPosition);

            Mat4x4 cameraRotationMatrix = glm::identity<Mat4x4>();
            cameraRotationMatrix = glm::rotate(cameraRotationMatrix, cameraXRot, cameraRightDirection);
            cameraRotationMatrix = glm::rotate(cameraRotationMatrix, cameraYRot, worldUP);

            cameraTransformMatrix = cameraTransformMatrix * cameraRotationMatrix;

            cameraTransformMatrix = glm::scale(cameraTransformMatrix, { 1.0f, 1.0f, 1.0f });

            cameraViewMatrix = glm::inverse(cameraTransformMatrix);

            cameraLookingDirection = Vector3{ cameraRotationMatrix * Vector4{0.0f, 0.0f, 1.0f, 1.0f } };
            cameraRightDirection = glm::cross({ 0.0f, 1.0f, 0.0f }, cameraLookingDirection);


            {
                //PROFILE_SCOPE("RENDERING");
                int totalTrianglesRendered = 0;
                for (int i = 0; i < testModel.meshes.size(); i++)
                {
                    DrawMeshOnScreenFromWorldWithTransform(imageData, imageDepthData, screenWidth, screenHeight, testModel.meshes[i], modelMat, cameraPosition, cameraLookingDirection, cameraViewMatrix, perspectiveProjectionMatrix, lineThickness, red, totalTrianglesRendered);
                }
                //std::cout << "Total triangles rendered := " << totalTrianglesRendered << std::endl;
            }
        }

        //std::cout << (int)mouseX / (int)collisionGridCellSize.x << ", " << (int)(screenHeight - mouseY) / (int)collisionGridCellSize.y << std::endl;

        UpdateUITreeStates(UI_Rect::uiRects[rootUIRectIndex], mouseX, mouseY);
        RenderUITree(UI_Rect::uiRects[rootUIRectIndex], screenWidth, screenHeight, imageData);

        //float screenY = mouseY;
        //if (mouseX >= 0 && mouseX < screenWidth && screenY >= 0 && screenY < screenHeight) {
        //    Vector2Int collisionGridCoords = { (int)mouseX / (int)collisionGridCellSize.x, (int)screenY / (int)collisionGridCellSize.y };
        //    Vector3 start = { collisionGridCoords.x * collisionGridCellSize.x, collisionGridCoords.y * collisionGridCellSize.y, 0.0f };
        //    Vector3 end = start + Vector3{ collisionGridCellSize.x, collisionGridCellSize.y, 0.0f };
        //    Vector4 colourOfGridSection = ColourToVector4(colour_pink);
        //    colourOfGridSection.w = 125;
        //    RenderRectangleOnScreen(start, end, colourOfGridSection, screenWidth, screenHeight, imageData);
        //}

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData.data());
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