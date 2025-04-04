#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const unsigned int NUM_COMPONENTS_IN_PIXEL = 4;

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
"uniform sampler2D texture1;\n"
"void main()\n"
"{\n"
"    FragColor = texture(texture1, TexCoord);\n"
"}\n\0";

bool pressedRight = false;
bool pressedLeft = false;
bool pressedUp = false;
bool pressedDown = false;

struct Colour {

public:

    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

int GetRedFlattenedImageDataSlotForPixel(int pixelX, int pixelY, int imageSizeX) {
    return (pixelX + pixelY * imageSizeX) * 4;
}

void FillSubPixels(std::vector<unsigned char>& imageData, int centreX, int centreY, int halfSizeMinusOne, Colour colourToFillWith) {
    for (int x = -halfSizeMinusOne; x < halfSizeMinusOne; x++)
    {
        for (int y = -halfSizeMinusOne; y < halfSizeMinusOne; y++)
        {
            int index = GetRedFlattenedImageDataSlotForPixel(centreX + x, centreY + y, SCR_WIDTH);
            if (index >= 0 && (index + 3) < imageData.size()) {

                imageData[index + 0] = colourToFillWith.r;
                imageData[index + 1] = colourToFillWith.g;
                imageData[index + 2] = colourToFillWith.b;
                imageData[index + 3] = colourToFillWith.a;
            }
        }
    }
}

int main()
{
    Colour backgroundColour = { 255, 255, 255, 255 };
    Colour playerColour = { 255, 0, 0, 255 };

    std::vector<unsigned char> imageData(SCR_WIDTH * SCR_HEIGHT * NUM_COMPONENTS_IN_PIXEL);
    //for (int i = 0; i < SCR_HEIGHT * SCR_WIDTH * NUM_COMPONENTS_IN_PIXEL; i += 4)
    //{
    //    imageData[i + 0] = backgroundColour.r;
    //    imageData[i + 1] = backgroundColour.g;
    //    imageData[i + 2] = backgroundColour.b;
    //    imageData[i + 3] = backgroundColour.a;
    //}

    //for (int y = 0; y < SCR_HEIGHT; y++)
    //{
    //    for (int x = 0; x < SCR_WIDTH; x++)
    //    {
    //        unsigned char curColourFracR = (unsigned char)((x / (float)SCR_WIDTH) * 255.0f);
    //        unsigned char curColourFracG = (unsigned char)((y / (float)SCR_HEIGHT) * 255.0f);

    //        int curIndexR = (x + (y * SCR_WIDTH)) * NUM_COMPONENTS_IN_PIXEL;
    //        imageData[curIndexR + 0] = curColourFracR;
    //        imageData[curIndexR + 1] = curColourFracG;
    //        imageData[curIndexR + 2] = 0;
    //        imageData[curIndexR + 3] = 255;
    //    }
    //}

    for (int y = 0; y < SCR_HEIGHT; y++)
    {
        for (int x = 0; x < SCR_WIDTH; x++)
        {
            int curIndexR = (x + (y * SCR_WIDTH)) * NUM_COMPONENTS_IN_PIXEL;
            imageData[curIndexR + 0] = backgroundColour.r;
            imageData[curIndexR + 1] = backgroundColour.g;
            imageData[curIndexR + 2] = backgroundColour.b;
            imageData[curIndexR + 3] = backgroundColour.a;
        }
    }

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

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindTexture(GL_TEXTURE_2D, texture);
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

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();

        frame++;
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
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
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}