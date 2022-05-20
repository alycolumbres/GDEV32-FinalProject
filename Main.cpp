// Quick note: GLAD needs to be included first before GLFW.
// Otherwise, GLAD will complain about gl.h being already included.
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// ---------------
// Function declarations
// ---------------

/// <summary>
/// Creates a shader program based on the provided file paths for the vertex and fragment shaders.
/// </summary>
/// <param name="vertexShaderFilePath">Vertex shader file path</param>
/// <param name="fragmentShaderFilePath">Fragment shader file path</param>
/// <returns>OpenGL handle to the created shader program</returns>
GLuint CreateShaderProgram(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath);

/// <summary>
/// Creates a shader based on the provided shader type and the path to the file containing the shader source.
/// </summary>
/// <param name="shaderType">Shader type</param>
/// <param name="shaderFilePath">Path to the file containing the shader source</param>
/// <returns>OpenGL handle to the created shader</returns>
GLuint CreateShaderFromFile(const GLuint& shaderType, const std::string& shaderFilePath);

/// <summary>
/// Creates a shader based on the provided shader type and the string containing the shader source.
/// </summary>
/// <param name="shaderType">Shader type</param>
/// <param name="shaderSource">Shader source string</param>
/// <returns>OpenGL handle to the created shader</returns>
GLuint CreateShaderFromSource(const GLuint& shaderType, const std::string& shaderSource);

/// <summary>
/// Function for handling the event when the size of the framebuffer changed.
/// </summary>
/// <param name="window">Reference to the window</param>
/// <param name="width">New width</param>
/// <param name="height">New height</param>
void FramebufferSizeChangedCallback(GLFWwindow* window, int width, int height);

void keyboardInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

/// <summary>
/// Struct containing data about a vertex
/// </summary>
struct Vertex
{
	GLfloat x, y, z;	// Position
	GLubyte r, g, b;	// Color
	GLfloat u, v;		// UV coordinates
    GLfloat nx, ny, nz; // Normal vector coordinates
};

glm::vec3 cameraPosition = glm::vec3(0.0f, 5.0f, 7.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool firstMouseMovement = true;
float lastX = 800.0f / 2.0;
float lastY = 600.0f / 2.0;

float yaw = -90.0f;
float pitch = 0.0f;
float fov = 50.0f;

/// <summary>
/// Main function.
/// </summary>
/// <returns>An integer indicating whether the program ended successfully or not.
/// A value of 0 indicates the program ended succesfully, while a non-zero value indicates
/// something wrong happened during execution.</returns>
int main()
{
	// Initialize GLFW
	int glfwInitStatus = glfwInit();
	if (glfwInitStatus == GLFW_FALSE)
	{
		std::cerr << "Failed to initialize GLFW!" << std::endl;
		return 1;
	}

	// Tell GLFW that we prefer to use OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	// Tell GLFW that we prefer to use the modern OpenGL
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Tell GLFW to create a window
	float windowWidth = 1024;
	float windowHeight = 576;
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Final Project", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cerr << "Failed to create GLFW window!" << std::endl;
		glfwTerminate();
		return 1;
	}

	// Tell GLFW to use the OpenGL context that was assigned to the window that we just created
	glfwMakeContextCurrent(window);

	// Register the callback function that handles when the framebuffer size has changed
	glfwSetFramebufferSizeCallback(window, FramebufferSizeChangedCallback);

	glfwSetCursorPosCallback(window, mouse_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// Tell GLAD to load the OpenGL function pointers
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::cerr << "Failed to initialize GLAD!" << std::endl;
		return 1;
	}

	// --- VERTEX SPECIFICATION ---
    
    Vertex floorVertices[6];
    
    // Bottom face of cube
    floorVertices[0] = { 1.0f, -1.0f, -1.0f,      255, 255, 255,     1.0f, 0.5f }; // bottom left
    floorVertices[1] = { -1.0f, -1.0f, 1.0f,      255, 255, 255,     0.5f, 1.0f }; // top right
    floorVertices[2] = { 1.0f, -1.0f, 1.0f,       255, 255, 255,     0.5f, 0.5f }; // top left
    floorVertices[3] = { -1.0f, -1.0f, -1.0f,     255, 255, 255,     1.0f, 1.0f }; // bottom right
    floorVertices[4] = { 1.0f, -1.0f, -1.0f,      255, 255, 255,     1.0f, 0.5f }; // bottom left
    floorVertices[5] = { -1.0f, -1.0f, 1.0f,      255, 255, 255,     0.5f, 1.0f }; // top right
    
    // Loop to calculate normal vertices per face
    for( int i=0; i < (sizeof(floorVertices)/sizeof(floorVertices[0])); i+=6 )
    {
        glm::vec3 v1( floorVertices[i].x, floorVertices[i].y, floorVertices[i].z );
        glm::vec3 v2( floorVertices[i+1].x, floorVertices[i+1].y, floorVertices[i+1].z );
        glm::vec3 v3( floorVertices[i+2].x, floorVertices[i+2].y, floorVertices[i+2].z );
        
        glm::vec3 a;
        a.x = v2.x - v1.x;
        a.y = v2.y - v1.y;
        a.z = v2.z - v1.z;
        
        glm::vec3 b;
        b.x = v3.x - v1.x;
        b.y = v3.y - v1.y;
        b.z = v3.z - v1.z;
        
        glm::vec3 crossProd( glm::cross(b, a) );
        
        for( int j=i; j < (i+6); j+=1 )
        {
            floorVertices[j].nx = crossProd.x;
            floorVertices[j].ny = crossProd.y;
            floorVertices[j].nz = crossProd.z;
        }
    }
    

	Vertex cubeVertices[36];

    // NOTE: NEGATIVE Z FACING FORWARD

    // right
    cubeVertices[0] = { 1.0f, -1.0f, -1.0f,       255, 255, 255,     0.0f, 0.5f }; // bottom left
    cubeVertices[1] = { 1.0f, 1.0f, 1.0f,         255, 255, 255,     0.5f, 1.0f }; // top right
    cubeVertices[2] = { 1.0f, 1.0f, -1.0f,        255, 255, 255,     0.0f, 1.0f }; // top left
    cubeVertices[3] = { 1.0f, -1.0f, 1.0f,        255, 255, 255,     0.5f, 0.5f }; // bottom right
    cubeVertices[4] = { 1.0f, 1.0f, 1.0f,         255, 255, 255,     0.5f, 1.0f }; // top right
    cubeVertices[5] = { 1.0f, -1.0f, -1.0f,       255, 255, 255,     0.0f, 0.5f }; // bottom left

    // top
    cubeVertices[6] = { -1.0f, 1.0f, -1.0f,       255, 255, 255,     1.0f, 1.0f }; // bottom left
    cubeVertices[7] = { 1.0f, 1.0f, 1.0f,         255, 255, 255,     0.5f, 0.5f }; // top right
    cubeVertices[8] = { -1.0f, 1.0f, 1.0f,        255, 255, 255,     0.5f, 1.0f }; // top left
    cubeVertices[9] = { 1.0f, 1.0f, -1.0f,        255, 255, 255,     1.0f, 0.5f }; // bottom right
    cubeVertices[10] = { -1.0f, 1.0f, -1.0f,      255, 255, 255,     1.0f, 1.0f }; // bottom left
    cubeVertices[11] = { 1.0f, 1.0f, 1.0f,        255, 255, 255,     0.5f, 0.5f }; // top right

    // left
    cubeVertices[12] = { -1.0f, -1.0f, 1.0f,      255, 255, 255,     0.5f, 0.5f }; // bottom left
    cubeVertices[13] = { -1.0f, 1.0f, -1.0f,      255, 255, 255,     0.0f, 1.0f }; // top right
    cubeVertices[14] = { -1.0f, 1.0f, 1.0f,       255, 255, 255,     0.5f, 1.0f }; // top left
    cubeVertices[15] = { -1.0f, -1.0f, -1.0f,     255, 255, 255,     0.0f, 0.5f }; // bottom right
    cubeVertices[16] = { -1.0f, 1.0f, -1.0f,      255, 255, 255,     0.0f, 1.0f }; // top right
    cubeVertices[17] = { -1.0f, -1.0f, 1.0f,      255, 255, 255,     0.5f, 0.5f }; // bottom left

    // bottom
    cubeVertices[18] = { 1.0f, -1.0f, -1.0f,      255, 255, 255,     1.0f, 0.5f }; // bottom left
    cubeVertices[19] = { -1.0f, -1.0f, 1.0f,      255, 255, 255,     0.5f, 1.0f }; // top right
    cubeVertices[20] = { 1.0f, -1.0f, 1.0f,       255, 255, 255,     0.5f, 0.5f }; // top left
    cubeVertices[21] = { -1.0f, -1.0f, -1.0f,     255, 255, 255,     1.0f, 1.0f }; // bottom right
    cubeVertices[22] = { 1.0f, -1.0f, -1.0f,      255, 255, 255,     1.0f, 0.5f }; // bottom left
    cubeVertices[23] = { -1.0f, -1.0f, 1.0f,      255, 255, 255,     0.5f, 1.0f }; // top right

    // back
    cubeVertices[24] = { 1.0f, -1.0f, 1.0f,       255, 255, 255,     0.0f, 0.5f }; // bottom left
    cubeVertices[25] = { -1.0f, 1.0f, 1.0f,       255, 255, 255,     0.5f, 1.0f }; // top right
    cubeVertices[26] = { 1.0f, 1.0f, 1.0f,        255, 255, 255,     0.0f, 1.0f }; // top left
    cubeVertices[27] = { -1.0f, -1.0f, 1.0f,      255, 255, 255,     0.5f, 0.5f }; // bottom right
    cubeVertices[28] = { -1.0f, 1.0f, 1.0f,       255, 255, 255,     0.5f, 1.0f }; // top right
    cubeVertices[29] = { 1.0f, -1.0f, 1.0f,       255, 255, 255,     0.0f, 0.5f }; // bottom left
    
    // front
    cubeVertices[30] = { -1.0f, -1.0f, -1.0f,     255, 255, 255,     0.5f, 0.5f }; // bottom left
    cubeVertices[31] = { 1.0f, 1.0f, -1.0f,       255, 255, 255,     0.0f, 1.0f }; // top right
    cubeVertices[32] = { -1.0f, 1.0f, -1.0f,      255, 255, 255,     0.5f, 1.0f }; // top left
    cubeVertices[33] = { 1.0f, -1.0f, -1.0f,      255, 255, 255,     0.0f, 0.5f }; // bottom right
    cubeVertices[34] = { 1.0f, 1.0f, -1.0f,       255, 255, 255,     0.0f, 1.0f }; // top right
    cubeVertices[35] = { -1.0f, -1.0f, -1.0f,     255, 255, 255,     0.5f, 0.5f }; // bottom left

	Vertex pyramidVertices[18];

	// base
	pyramidVertices[0] = { 1.0f, 0.0f, -1.0f,      255, 255, 255,     1.0f, 0.5f }; // bottom left
	pyramidVertices[1] = { -1.0f, 0.0f, 1.0f,      255, 255, 255,     0.5f, 1.0f }; // top right
	pyramidVertices[2] = { 1.0f, 0.0f, 1.0f,       255, 255, 255,     0.5f, 0.5f }; // top left
	pyramidVertices[3] = { -1.0f, 0.0f, -1.0f,     255, 255, 255,     1.0f, 1.0f }; // bottom right
	pyramidVertices[4] = { 1.0f, 0.0f, -1.0f,      255, 255, 255,     1.0f, 0.5f }; // bottom left
	pyramidVertices[5] = { -1.0f, 0.0f, 1.0f,      255, 255, 255,     0.5f, 1.0f }; // top right

	// back 
	pyramidVertices[6] = { 1.0f, 0.0f, 1.0f,	255, 255, 255,		0.0f, 0.0f }; 
	pyramidVertices[7] = { -1.0f, 0.0f, 1.0f,	255, 255, 255,		1.0f, 0.0f }; 
	pyramidVertices[8] = { 0.0f, 1.0f, 0.0f,	255, 255, 255,		0.5f, 0.1f }; 

	// right 
	pyramidVertices[9] = { 1.0f, 0.0f, -1.0f,	255, 255, 255,		0.0f, 0.0f };
	pyramidVertices[10] = { 1.0f, 0.0f, 1.0f,	255, 255, 255,		1.0f, 0.0f };
	pyramidVertices[11] = { 0.0f, 1.0f, 0.0f,	255, 255, 255,		0.5f, 0.1f };

	// front
	pyramidVertices[12] = { -1.0f, 0.0f, -1.0f,	255, 255, 255,		0.0f, 0.0f };
	pyramidVertices[13] = { 1.0f, 0.0f, -1.0f,	255, 255, 255,		1.0f, 0.0f };
	pyramidVertices[14] = { 0.0f, 1.0f, 0.0f,	255, 255, 255,		0.5f, 0.1f };

	// left
	pyramidVertices[15] = { -1.0f, 0.0f, 1.0f,	255, 255, 255,		0.0f, 0.0f };
	pyramidVertices[16] = { -1.0f, 0.0f, -1.0f,	255, 255, 255,		1.0f, 0.0f };
	pyramidVertices[17] = { 0.0f, 1.0f, 0.0f,	255, 255, 255,		0.5f, 0.1f };

	Vertex hexVertices[52];

	// front hexagon
	hexVertices[0] = { 0.0f, 0.0f, 0.0f,	255, 255, 255,		0.5f, 0.5f };
	hexVertices[1] = { 0.5f, 1.0f, 0.0f,	255, 255, 255,		0.75f, 1.0f };
	hexVertices[2] = { 1.0f, 0.0f, 0.0f,	255, 255, 255,		1.0f, 0.5f };
	hexVertices[3] = { 0.5f, -1.0f, 0.0f,	255, 255, 255,		0.75f, 0.0f };
	hexVertices[4] = { -0.5f, -1.0f, 0.0f,	255, 255, 255,		0.25f, 0.0f };
	hexVertices[5] = { -1.0f, 0.0f, 0.0f,	255, 255, 255,		0.0f, 0.5f };
	hexVertices[6] = { -0.5f, 1.0f, 0.0f,	255, 255, 255,		0.25f, 1.0f };
	hexVertices[7] = { 0.5f, 1.0f, 0.0f,	255, 255, 255,		0.75f, 1.0f };

	// front hexagon
	hexVertices[8] = { 0.0f, 0.0f, 2.0f,	255, 255, 255,		0.5f, 0.5f };
	hexVertices[9] = { 0.5f, 1.0f, 2.0f,	255, 255, 255,		0.75f, 1.0f };
	hexVertices[10] = { 1.0f, 0.0f, 2.0f,	255, 255, 255,		1.0f, 0.5f };
	hexVertices[11] = { 0.5f, -1.0f, 2.0f,	255, 255, 255,		0.75f, 0.0f };
	hexVertices[12] = { -0.5f, -1.0f, 2.0f,	255, 255, 255,		0.25f, 0.0f };
	hexVertices[13] = { -1.0f, 0.0f, 2.0f,	255, 255, 255,		0.0f, 0.5f };
	hexVertices[14] = { -0.5f, 1.0f, 2.0f,	255, 255, 255,		0.25f, 1.0f };
	hexVertices[15] = { 0.5f, 1.0f, 2.0f,	255, 255, 255,		0.75f, 1.0f };

	// top right
	hexVertices[16] = { 0.5f, 1.0f, 0.0f,	255, 255, 255,		1.0f, 0.75f };
	hexVertices[17] = { 1.0f, 0.0f, 0.0f,	255, 255, 255,		1.0f, 0.25f };
	hexVertices[18] = { 1.0f, 0.0f, 2.0f,	255, 255, 255,		0.0f, 0.25f };
	hexVertices[19] = { 0.5f, 1.0f, 0.0f,	255, 255, 255,		1.0f, 0.75f };
	hexVertices[20] = { 0.5f, 1.0f, 2.0f,	255, 255, 255,		0.0f, 0.75f };
	hexVertices[21] = { 1.0f, 0.0f, 2.0f,	255, 255, 255,		0.0f, 0.25f };

	// top
	hexVertices[22] = { 0.5f, 1.0f, 0.0f,	255, 255, 255,		0.25f, 0.15f };
	hexVertices[23] = { -0.5f, 1.0f, 0.0f,	255, 255, 255,		0.25f, 0.15f };
	hexVertices[24] = { 0.5f, 1.0f, 2.0f,	255, 255, 255,		0.25f, 0.15f };
	hexVertices[25] = { -0.5f, 1.0f, 0.0f,	255, 255, 255,		0.25f, 0.15f };
	hexVertices[26] = { -0.5f, 1.0f, 2.0f,	255, 255, 255,		0.25f, 0.15f };
	hexVertices[27] = { 0.5f, 1.0f, 2.0f,	255, 255, 255,		0.25f, 0.15f };

	// top left
	hexVertices[28] = { -0.5f, 1.0f, 0.0f,	255, 255, 255,		0.0f, 0.75f };
	hexVertices[29] = { -1.0f, 0.0f, 0.0f,	255, 255, 255,		0.0f, 0.25f };
	hexVertices[30] = { -0.5f, 1.0f, 2.0f,	255, 255, 255,		1.0f, 0.75f };
	hexVertices[31] = { -1.0f, 0.0f, 0.0f,	255, 255, 255,		0.0f, 0.25f };
	hexVertices[32] = { -1.0f, 0.0f, 2.0f,	255, 255, 255,		1.0f, 0.25f };
	hexVertices[33] = { -0.5f, 1.0f, 2.0f,	255, 255, 255,		1.0f, 0.75f };

	// bottom left
	hexVertices[34] = { -0.5f, -1.0f, 0.0f,	255, 255, 255,		0.25f, 0.15f };
	hexVertices[35] = { -1.0f, 0.0f, 0.0f,	255, 255, 255,		0.25f, 0.15f };
	hexVertices[36] = { -1.0f, 0.0f, 2.0f,	255, 255, 255,		0.25f, 0.15f };
	hexVertices[37] = { -0.5f, -1.0f, 0.0f,	255, 255, 255,		0.25f, 0.15f };
	hexVertices[38] = { -0.5f, -1.0f, 2.0f,	255, 255, 255,		0.25f, 0.15f };
	hexVertices[39] = { -1.0f, 0.0f, 2.0f,	255, 255, 255,		0.25f, 0.15f };

	// bottom
	hexVertices[40] = { 0.5f, -1.0f, 0.0f,	255, 255, 255,		0.0f, 0.75f };
	hexVertices[41] = { -0.5f, -1.0f, 0.0f,	255, 255, 255,		0.0f, 0.25f };
	hexVertices[42] = { 0.5f, -1.0f, 2.0f,	255, 255, 255,		1.0f, 0.75f };
	hexVertices[43] = { -0.5f, -1.0f, 0.0f,	255, 255, 255,		0.0f, 0.25f };
	hexVertices[44] = { -0.5f, -1.0f, 2.0f,	255, 255, 255,		1.0f, 0.25f };
	hexVertices[45] = { 0.5f, -1.0f, 2.0f,	255, 255, 255,		1.0f, 0.75f };

	// bottom right
	hexVertices[46] = { 1.0f, 0.0f, 0.0f,	255, 255, 255,		0.25f, 0.15f };
	hexVertices[47] = { 0.5f, -1.0f, 0.0f,	255, 255, 255,		0.25f, 0.15f };
	hexVertices[48] = { 1.0f, 0.0f, 2.0f,	255, 255, 255,		0.25f, 0.15f };
	hexVertices[49] = { 0.5f, -1.0f, 0.0f,	255, 255, 255,		0.25f, 0.15f };
	hexVertices[50] = { 1.0f, 0.0f, 2.0f,	255, 255, 255,		0.25f, 0.15f };
	hexVertices[51] = { 0.5f, -1.0f, 2.0f,	255, 255, 255,		0.25f, 0.15f };

    
    
    // --- SETTING UP OF VBOs, VAOs ---
    
    // FLOOR VBO, VAO

    // Create a vertex buffer object (VBO), and upload our vertices data to the VBO
    GLuint vboFloor;
    glGenBuffers(1, &vboFloor);
    glBindBuffer(GL_ARRAY_BUFFER, vboFloor);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Create a vertex array object that contains data on how to map vertex attributes
    // (e.g., position, color) to vertex shader properties.
    GLuint vaoFloor;
    glGenVertexArrays(1, &vaoFloor);
    glBindVertexArray(vaoFloor);

    glBindBuffer(GL_ARRAY_BUFFER, vboFloor);

    // Vertex attribute 0 - Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));

    // Vertex attribute 1 - Color
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)(offsetof(Vertex, r)));

    // Vertex attribute 2 - UV coordinate
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, u)));

    glBindVertexArray(0);
    

	// CUBE VBO, VAO

	// Create a vertex buffer object (VBO), and upload our vertices data to the VBO
	GLuint vboCube;
	glGenBuffers(1, &vboCube);
	glBindBuffer(GL_ARRAY_BUFFER, vboCube);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create a vertex array object that contains data on how to map vertex attributes
	// (e.g., position, color) to vertex shader properties.
	GLuint vaoCube;
	glGenVertexArrays(1, &vaoCube);
	glBindVertexArray(vaoCube);

	glBindBuffer(GL_ARRAY_BUFFER, vboCube);

	// Vertex attribute 0 - Position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));

	// Vertex attribute 1 - Color
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)(offsetof(Vertex, r)));

	// Vertex attribute 2 - UV coordinate
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, u)));

	glBindVertexArray(0);


	// PYRAMID VBO, VAO

	GLuint vboPyramid;
	glGenBuffers(1, &vboPyramid);
	glBindBuffer(GL_ARRAY_BUFFER, vboPyramid);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create a vertex array object that contains data on how to map vertex attributes
	// (e.g., position, color) to vertex shader properties.
	GLuint vaoPyramid;
	glGenVertexArrays(1, &vaoPyramid);
	glBindVertexArray(vaoPyramid);

	glBindBuffer(GL_ARRAY_BUFFER, vboPyramid);

	// Vertex attribute 0 - Position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));

	// Vertex attribute 1 - Color
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)(offsetof(Vertex, r)));

	// Vertex attribute 2 - UV coordinate
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, u)));

	glBindVertexArray(0);


	// HEXAGONAL PRISM VBO, VAO

	GLuint vboHex;
	glGenBuffers(1, &vboHex);
	glBindBuffer(GL_ARRAY_BUFFER, vboHex);
	glBufferData(GL_ARRAY_BUFFER, sizeof(hexVertices), hexVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create a vertex array object that contains data on how to map vertex attributes
	// (e.g., position, color) to vertex shader properties.
	GLuint vaoHex;
	glGenVertexArrays(1, &vaoHex);
	glBindVertexArray(vaoHex);

	glBindBuffer(GL_ARRAY_BUFFER, vboHex);

	// Vertex attribute 0 - Position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));

	// Vertex attribute 1 - Color
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)(offsetof(Vertex, r)));

	// Vertex attribute 2 - UV coordinate
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, u)));

	glBindVertexArray(0);


    
	// Create a shader program
	GLuint program = CreateShaderProgram("main.vsh", "main.fsh");

	// Tell OpenGL the dimensions of the region where stuff will be drawn.
	// For now, tell OpenGL to use the whole screen
    
    // FOR WINDOWS
	// glViewport(0, 0, windowWidth, windowHeight);
    
    // FOR MAC
    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    
    
    
    // --- TEXTURES ---

	// Create a variable that will contain the ID for our texture,
	// and use glGenTextures() to generate the texture itself
	GLuint tex;
	glGenTextures(1, &tex);

	// --- Load our image using stb_image ---

	// Im image-space (pixels), (0, 0) is the upper-left corner of the image
	// However, in u-v coordinates, (0, 0) is the lower-left corner of the image
	// This means that the image will appear upside-down when we use the image data as is
	// This function tells stbi to flip the image vertically so that it is not upside-down when we use it
	stbi_set_flip_vertically_on_load(true);

	// 'imageWidth' and imageHeight will contain the width and height of the loaded image respectively
	int imageWidth, imageHeight, numChannels;

	// Read the image data and store it in an unsigned char array
	unsigned char* imageData = stbi_load("texture.png", &imageWidth, &imageHeight, &numChannels, 0);

	// Make sure that we actually loaded the image before uploading the data to the GPU
	if (imageData != nullptr)
	{
		// Our texture is 2D, so we bind our texture to the GL_TEXTURE_2D target
		glBindTexture(GL_TEXTURE_2D, tex);

		// Set the filtering methods for magnification and minification
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		// Set the wrapping method for the s-axis (x-axis) and t-axis (y-axis)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Upload the image data to GPU memory
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);

		// If we set minification to use mipmaps, we can tell OpenGL to generate the mipmaps for us
		//glGenerateMipmap(GL_TEXTURE_2D);

		// Once we have copied the data over to the GPU, we can delete
		// the data on the CPU side, since we won't be using it anymore
		stbi_image_free(imageData);
		imageData = nullptr;
	}
	else
	{
		std::cerr << "Failed to load image" << std::endl;
	}

	// Create a variable that will contain the ID for our texture,
	// and use glGenTextures() to generate the texture itself
	GLuint tex2;
	glGenTextures(1, &tex2);

	// --- Load our image using stb_image ---

	// Im image-space (pixels), (0, 0) is the upper-left corner of the image
	// However, in u-v coordinates, (0, 0) is the lower-left corner of the image
	// This means that the image will appear upside-down when we use the image data as is
	// This function tells stbi to flip the image vertically so that it is not upside-down when we use it
	stbi_set_flip_vertically_on_load(true);


	// Read the image data and store it in an unsigned char array
	imageData = stbi_load("texture2.jpg", &imageWidth, &imageHeight, &numChannels, 0);

	// Make sure that we actually loaded the image before uploading the data to the GPU
	if (imageData != nullptr)
	{
		// Our texture is 2D, so we bind our texture to the GL_TEXTURE_2D target
		glBindTexture(GL_TEXTURE_2D, tex2);

		// Set the filtering methods for magnification and minification
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		// Set the wrapping method for the s-axis (x-axis) and t-axis (y-axis)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Upload the image data to GPU memory
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);

		// If we set minification to use mipmaps, we can tell OpenGL to generate the mipmaps for us
		//glGenerateMipmap(GL_TEXTURE_2D);

		// Once we have copied the data over to the GPU, we can delete
		// the data on the CPU side, since we won't be using it anymore
		stbi_image_free(imageData);
		imageData = nullptr;
	}
	else
	{
		std::cerr << "Failed to load image" << std::endl;
	}

	// Create a variable that will contain the ID for our texture,
	// and use glGenTextures() to generate the texture itself
	GLuint tex3;
	glGenTextures(1, &tex3);

	// --- Load our image using stb_image ---

	// Im image-space (pixels), (0, 0) is the upper-left corner of the image
	// However, in u-v coordinates, (0, 0) is the lower-left corner of the image
	// This means that the image will appear upside-down when we use the image data as is
	// This function tells stbi to flip the image vertically so that it is not upside-down when we use it
	stbi_set_flip_vertically_on_load(true);


	// Make sure that we actually loaded the image before uploading the data to the GPU
	if (imageData != nullptr)
	{
		// Our texture is 2D, so we bind our texture to the GL_TEXTURE_2D target
		glBindTexture(GL_TEXTURE_2D, tex3);

		// Set the filtering methods for magnification and minification
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		// Set the wrapping method for the s-axis (x-axis) and t-axis (y-axis)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Upload the image data to GPU memory
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);

		// If we set minification to use mipmaps, we can tell OpenGL to generate the mipmaps for us
		//glGenerateMipmap(GL_TEXTURE_2D);

		// Once we have copied the data over to the GPU, we can delete
		// the data on the CPU side, since we won't be using it anymore
		stbi_image_free(imageData);
		imageData = nullptr;
	}
	else
	{
		std::cerr << "Failed to load image" << std::endl;
	}

	// Create a variable that will contain the ID for our texture,
	// and use glGenTextures() to generate the texture itself
	GLuint floorTex;
	glGenTextures(1, &floorTex);

	// --- Load our image using stb_image ---

	// Im image-space (pixels), (0, 0) is the upper-left corner of the image
	// However, in u-v coordinates, (0, 0) is the lower-left corner of the image
	// This means that the image will appear upside-down when we use the image data as is
	// This function tells stbi to flip the image vertically so that it is not upside-down when we use it
	stbi_set_flip_vertically_on_load(true);

	// 'imageWidth' and imageHeight will contain the width and height of the loaded image respectively
	//int imageWidth, imageHeight, numChannels;

	// Read the image data and store it in an unsigned char array
	imageData = stbi_load("sand texture.jpeg", &imageWidth, &imageHeight, &numChannels, 0);

	// Make sure that we actually loaded the image before uploading the data to the GPU
	if (imageData != nullptr)
	{
		// Our texture is 2D, so we bind our texture to the GL_TEXTURE_2D target
		glBindTexture(GL_TEXTURE_2D, floorTex);

		// Set the filtering methods for magnification and minification
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		// Set the wrapping method for the s-axis (x-axis) and t-axis (y-axis)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Upload the image data to GPU memory
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);

		// If we set minification to use mipmaps, we can tell OpenGL to generate the mipmaps for us
		//glGenerateMipmap(GL_TEXTURE_2D);

		// Once we have copied the data over to the GPU, we can delete
		// the data on the CPU side, since we won't be using it anymore
		stbi_image_free(imageData);
		imageData = nullptr;
	}
	else
	{
		std::cerr << "Failed to load image" << std::endl;
	}


	glEnable(GL_DEPTH_TEST);

	// Render loop
	while (!glfwWindowShouldClose(window))
	{
		keyboardInput(window);
		float time = glfwGetTime();
		deltaTime = time - lastFrame;
		lastFrame = time;

		// Use the shader program that we created
		glUseProgram(program);
        
        
        
        // --- LIGHTING ---
        
        // Eye position
        glm::vec3 eyePosition = cameraPosition;
        GLint eyePositionUniformLocation = glGetUniformLocation(program, "eyePosition");
        glUniform3fv(eyePositionUniformLocation, 1, glm::value_ptr(eyePosition));
        
        
        // Ambient intensity FOR DIRECTIONAL LIGHT
        float ambientDirectionalIntensity = 0.8f;
        GLint ambientDirectionalIntensityUniformLocation = glGetUniformLocation(program, "ambientDirectionalIntensity");
        glUniform1f(ambientDirectionalIntensityUniformLocation, ambientDirectionalIntensity);
        
        // Ambient component FOR DIRECTIONAL LIGHT
        glm::vec3 ambientDirectionalComponent = glm::vec3(1.0f, 0.9f, 0.8f);
        GLint ambientDirectionalComponentUniformLocation = glGetUniformLocation(program, "ambientDirectionalComponent");
        glUniform3fv(ambientDirectionalComponentUniformLocation, 1, glm::value_ptr(ambientDirectionalComponent));
        
        
        // Diffuse intensity
        float diffuseIntensity = 0.8f;
        GLint diffuseIntensityUniformLocation = glGetUniformLocation(program, "diffuseIntensity");
        glUniform1f(diffuseIntensityUniformLocation, diffuseIntensity);
        
        // Diffuse component
        glm::vec3 diffuseComponent = glm::vec3(0.8f, 0.8f, 0.8f);
        GLint diffuseComponentUniformLocation = glGetUniformLocation(program, "diffuseComponent");
        glUniform3fv(diffuseComponentUniformLocation, 1, glm::value_ptr(diffuseComponent));
        
        
        // Specular intensity
        float specularIntensity = 5.0f;
        GLint specularIntensityUniformLocation = glGetUniformLocation(program, "specularIntensity");
        glUniform1f(specularIntensityUniformLocation, specularIntensity);
        
        // Specular component
        glm::vec3 specularComponent = glm::vec3(0.4f, 0.4f, 0.4f);
        GLint specularComponentUniformLocation = glGetUniformLocation(program, "specularComponent");
        glUniform3fv(specularComponentUniformLocation, 1, glm::value_ptr(specularComponent));
        
        
        // Shininess
        float shininess = 64.0f;
        GLint shininessUniformLocation = glGetUniformLocation(program, "shininess");
        glUniform1f(shininessUniformLocation, shininess);
        
        
        // Clear the color and depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        

		// FLOOR

		// Use the vertex array object that we created
		glBindVertexArray(vaoFloor);

		// Bind our texture to texture unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, floorTex);

		// Make our sampler in the fragment shader use texture unit 0
		GLint texUniformLocation = glGetUniformLocation(program, "tex");
		glUniform1i(texUniformLocation, 0);

		glm::mat4 floorModelMatrix = glm::mat4(1.0f);
		floorModelMatrix = glm::translate(floorModelMatrix, glm::vec3(0.0f, 40.0f, -50.0f));
		floorModelMatrix = glm::scale(floorModelMatrix, glm::vec3(50.0f, 50.0f, 50.0f));
		floorModelMatrix = glm::rotate(floorModelMatrix, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        
		glm::mat4 viewMatrix = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
		glm::mat4 projectionMatrix = glm::perspective(glm::radians(fov), windowWidth / windowHeight, 0.1f, 100.0f);
		glm::mat4 finalMatrix = projectionMatrix * viewMatrix * floorModelMatrix;

		GLint matUniformLocation = glGetUniformLocation(program, "mat");
		glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
        
        GLint modelUniformLocation = glGetUniformLocation(program, "model");
        glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, glm::value_ptr(floorModelMatrix));
        
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Static Pyramid

		glBindVertexArray(vaoPyramid);
		glBindTexture(GL_TEXTURE_2D, tex3);

		glm::mat4 pyramidModelMatrix = glm::mat4(1.0f);
		pyramidModelMatrix = glm::translate(pyramidModelMatrix, glm::vec3(-6.0f, -10.0f, 4.0f));
		pyramidModelMatrix = glm::scale(pyramidModelMatrix, glm::vec3(1.5f, 1.5f, 1.5f));
		finalMatrix = projectionMatrix * viewMatrix * pyramidModelMatrix;

		matUniformLocation = glGetUniformLocation(program, "mat");
		glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
		//glDrawArrays(GL_TRIANGLES, 0, 18);

		// Static Pyramid 2

		glm::mat4 pyramid2ModelMatrix = glm::mat4(1.0f);
		pyramid2ModelMatrix = glm::translate(pyramid2ModelMatrix, glm::vec3(6.0f, -10.0f, 4.0f));
		pyramid2ModelMatrix = glm::scale(pyramid2ModelMatrix, glm::vec3(2.0f, 2.0f, 2.0f));
		pyramid2ModelMatrix = glm::rotate(pyramid2ModelMatrix, glm::radians(35.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		finalMatrix = projectionMatrix * viewMatrix * pyramid2ModelMatrix;

		matUniformLocation = glGetUniformLocation(program, "mat");
		glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
		//glDrawArrays(GL_TRIANGLES, 0, 18);

		// Moving Pyramid

		glm::mat4 pyramid3ModelMatrix = glm::mat4(1.0f);
		pyramid3ModelMatrix = glm::translate(pyramid3ModelMatrix, glm::vec3(0.0f, -6.0f, -5.0f));
		pyramid3ModelMatrix = glm::scale(pyramid3ModelMatrix, glm::vec3(2.5f, 2.5f, 2.5f));
		pyramid3ModelMatrix = glm::rotate(pyramid3ModelMatrix, glm::radians(45.0f * time), glm::vec3(0.0f, 1.0f, 0.0f));
		finalMatrix = projectionMatrix * viewMatrix * pyramid3ModelMatrix;

		matUniformLocation = glGetUniformLocation(program, "mat");
		glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
		//glDrawArrays(GL_TRIANGLES, 0, 18);
		

		// Moving Hexagonal Prism
		
		glBindVertexArray(vaoHex);
		glBindTexture(GL_TEXTURE_2D, tex2);

		glm::mat4 hexModelMatrix = glm::mat4(1.0f);
		hexModelMatrix = glm::translate(hexModelMatrix, glm::vec3(5.0f, 0.0f, -7.0f));
		hexModelMatrix = glm::scale(hexModelMatrix, glm::vec3(2.0f, 2.0f, 2.0f));
		hexModelMatrix = glm::rotate(hexModelMatrix, glm::radians(50.0f * time), glm::vec3(1.0f, 1.0f, 1.0f));
		finalMatrix = projectionMatrix * viewMatrix * hexModelMatrix;

		matUniformLocation = glGetUniformLocation(program, "mat");
		glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));

		/*glDrawArrays(GL_TRIANGLE_FAN, 0, 8);
		glDrawArrays(GL_TRIANGLE_FAN, 8, 8);
		glDrawArrays(GL_TRIANGLES, 16, 36);	*/

		// Static Hexagonal Prism

		glm::mat4 hex2ModelMatrix = glm::mat4(1.0f);
		hex2ModelMatrix = glm::translate(hex2ModelMatrix, glm::vec3(0.0f, -6.0f, -5.0f));
		//hex2ModelMatrix = glm::scale(hex2ModelMatrix, glm::vec3(2.0f, 2.0f, 2.0f));
		//hex2ModelMatrix = glm::rotate(hex2ModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		finalMatrix = projectionMatrix * viewMatrix * hex2ModelMatrix;

		matUniformLocation = glGetUniformLocation(program, "mat");
		glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
		
		/*glDrawArrays(GL_TRIANGLE_FAN, 0, 8);
		glDrawArrays(GL_TRIANGLE_FAN, 8, 8);
		glDrawArrays(GL_TRIANGLES, 16, 36);*/

		// "Unuse" the vertex array object
		glBindVertexArray(0);

		// Tell GLFW to swap the screen buffer with the offscreen buffer
		glfwSwapBuffers(window);

		// Tell GLFW to process window events (e.g., input events, window closed events, etc.)
		glfwPollEvents();
	}

	// --- Cleanup ---

	// Make sure to delete the shader program
	glDeleteProgram(program);

	// Delete the VBO that contains our vertices
	glDeleteBuffers(1, &vboCube);
	glDeleteBuffers(1, &vboPyramid);
	glDeleteBuffers(1, &vboHex);

	// Delete the vertex array object
	glDeleteVertexArrays(1, &vaoCube);
	glDeleteVertexArrays(1, &vaoPyramid);
	glDeleteVertexArrays(1, &vaoHex);

	// Remember to tell GLFW to clean itself up before exiting the application
	glfwTerminate();

	return 0;
}

void keyboardInput(GLFWwindow* window)
{
	float speed = 20.0 * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		cameraPosition += speed * cameraFront;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		cameraPosition -= speed * cameraFront;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		cameraPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouseMovement)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouseMovement = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 90.0f)
		pitch = 90.0f;
	if (pitch < -90.0f)
		pitch = -90.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}

/// <summary>
/// Creates a shader program based on the provided file paths for the vertex and fragment shaders.
/// </summary>
/// <param name="vertexShaderFilePath">Vertex shader file path</param>
/// <param name="fragmentShaderFilePath">Fragment shader file path</param>
/// <returns>OpenGL handle to the created shader program</returns>
GLuint CreateShaderProgram(const std::string& vertexShaderFilePath, const std::string& fragmentShaderFilePath)
{
	GLuint vertexShader = CreateShaderFromFile(GL_VERTEX_SHADER, vertexShaderFilePath);
	GLuint fragmentShader = CreateShaderFromFile(GL_FRAGMENT_SHADER, fragmentShaderFilePath);

	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	glLinkProgram(program);

	glDetachShader(program, vertexShader);
	glDeleteShader(vertexShader);
	glDetachShader(program, fragmentShader);
	glDeleteShader(fragmentShader);

	// Check shader program link status
	GLint linkStatus;
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	if (linkStatus != GL_TRUE) {
		char infoLog[512];
		GLsizei infoLogLen = sizeof(infoLog);
		glGetProgramInfoLog(program, infoLogLen, &infoLogLen, infoLog);
		std::cerr << "program link error: " << infoLog << std::endl;
	}

	return program;
}

/// <summary>
/// Creates a shader based on the provided shader type and the path to the file containing the shader source.
/// </summary>
/// <param name="shaderType">Shader type</param>
/// <param name="shaderFilePath">Path to the file containing the shader source</param>
/// <returns>OpenGL handle to the created shader</returns>
GLuint CreateShaderFromFile(const GLuint& shaderType, const std::string& shaderFilePath)
{
	std::ifstream shaderFile(shaderFilePath);
	if (shaderFile.fail())
	{
		std::cerr << "Unable to open shader file: " << shaderFilePath << std::endl;
		return 0;
	}

	std::string shaderSource;
	std::string temp;
	while (std::getline(shaderFile, temp))
	{
		shaderSource += temp + "\n";
	}
	shaderFile.close();

	return CreateShaderFromSource(shaderType, shaderSource);
}

/// <summary>
/// Creates a shader based on the provided shader type and the string containing the shader source.
/// </summary>
/// <param name="shaderType">Shader type</param>
/// <param name="shaderSource">Shader source string</param>
/// <returns>OpenGL handle to the created shader</returns>
GLuint CreateShaderFromSource(const GLuint& shaderType, const std::string& shaderSource)
{
	GLuint shader = glCreateShader(shaderType);

	const char* shaderSourceCStr = shaderSource.c_str();
	GLint shaderSourceLen = static_cast<GLint>(shaderSource.length());
	glShaderSource(shader, 1, &shaderSourceCStr, &shaderSourceLen);
	glCompileShader(shader);

	// Check compilation status
	GLint compileStatus;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus == GL_FALSE)
	{
		char infoLog[512];
		GLsizei infoLogLen = sizeof(infoLog);
		glGetShaderInfoLog(shader, infoLogLen, &infoLogLen, infoLog);
		std::cerr << "shader compilation error: " << infoLog << std::endl;
	}

	return shader;
}

/// <summary>
/// Function for handling the event when the size of the framebuffer changed.
/// </summary>
/// <param name="window">Reference to the window</param>
/// <param name="width">New width</param>
/// <param name="height">New height</param>
void FramebufferSizeChangedCallback(GLFWwindow* window, int width, int height)
{
	// Whenever the size of the framebuffer changed (due to window resizing, etc.),
	// update the dimensions of the region to the new size
	glViewport(0, 0, width, height);
}
