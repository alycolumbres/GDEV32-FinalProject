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

glm::vec3 cameraPosition = glm::vec3(0.0f, 20.0f, 80.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

glm::vec3 directionalLightPosition = glm::vec3(50.0f, 60.0f, -100.0f);

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
    
    // Loop to calculate normal vertices per face
    for( int i=0; i < (sizeof(cubeVertices)/sizeof(cubeVertices[0])); i+=6 )
    {
        glm::vec3 v1( cubeVertices[i].x, cubeVertices[i].y, cubeVertices[i].z );
        glm::vec3 v2( cubeVertices[i+1].x, cubeVertices[i+1].y, cubeVertices[i+1].z );
        glm::vec3 v3( cubeVertices[i+2].x, cubeVertices[i+2].y, cubeVertices[i+2].z );
        
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
            cubeVertices[j].nx = crossProd.x;
            cubeVertices[j].ny = crossProd.y;
            cubeVertices[j].nz = crossProd.z;
        }
    }

    
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
	pyramidVertices[8] = { 0.0f, 1.0f, 0.0f,	255, 255, 255,		0.5f, 1.0f }; 

	// right 
	pyramidVertices[9] = { 1.0f, 0.0f, -1.0f,	255, 255, 255,		0.0f, 0.0f };
	pyramidVertices[10] = { 1.0f, 0.0f, 1.0f,	255, 255, 255,		1.0f, 0.0f };
	pyramidVertices[11] = { 0.0f, 1.0f, 0.0f,	255, 255, 255,		0.5f, 1.0f };

	// front
	pyramidVertices[12] = { -1.0f, 0.0f, -1.0f,	255, 255, 255,		0.0f, 0.0f };
	pyramidVertices[13] = { 1.0f, 0.0f, -1.0f,	255, 255, 255,		1.0f, 0.0f };
	pyramidVertices[14] = { 0.0f, 1.0f, 0.0f,	255, 255, 255,		0.5f, 1.0f };

	// left
	pyramidVertices[15] = { -1.0f, 0.0f, 1.0f,	255, 255, 255,		0.0f, 0.0f };
	pyramidVertices[16] = { -1.0f, 0.0f, -1.0f,	255, 255, 255,		1.0f, 0.0f };
	pyramidVertices[17] = { 0.0f, 1.0f, 0.0f,	255, 255, 255,		0.5f, 1.0f };
    
    // Loop to calculate normal vertices per face
    for( int i=0; i < (sizeof(pyramidVertices)/sizeof(pyramidVertices[0])); i+=6 )
    {
        glm::vec3 v1( pyramidVertices[i].x, pyramidVertices[i].y, pyramidVertices[i].z );
        glm::vec3 v2( pyramidVertices[i+1].x, pyramidVertices[i+1].y, pyramidVertices[i+1].z );
        glm::vec3 v3( pyramidVertices[i+2].x, pyramidVertices[i+2].y, pyramidVertices[i+2].z );
        
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
            pyramidVertices[j].nx = crossProd.x;
            pyramidVertices[j].ny = crossProd.y;
            pyramidVertices[j].nz = crossProd.z;
        }
    }

    
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

    // back hexagon
    hexVertices[8] = { 0.0f, 0.0f, 2.0f,	255, 255, 255,		0.5f, 0.5f };
    hexVertices[9] = { 0.5f, 1.0f, 2.0f,	255, 255, 255,		0.75f, 1.0f };
    hexVertices[10] = { 1.0f, 0.0f, 2.0f,	255, 255, 255,		1.0f, 0.5f };
    hexVertices[11] = { 0.5f, -1.0f, 2.0f,	255, 255, 255,		0.75f, 0.0f };
    hexVertices[12] = { -0.5f, -1.0f, 2.0f,	255, 255, 255,		0.25f, 0.0f };
    hexVertices[13] = { -1.0f, 0.0f, 2.0f,	255, 255, 255,		0.0f, 0.5f };
    hexVertices[14] = { -0.5f, 1.0f, 2.0f,	255, 255, 255,		0.25f, 1.0f };
    hexVertices[15] = { 0.5f, 1.0f, 2.0f,	255, 255, 255,		0.75f, 1.0f };

    // top right
    hexVertices[16] = { 1.0f, 0.0f, 0.0f,	255, 255, 255,		1.0f, 0.25f };
    hexVertices[17] = { 0.5f, 1.0f, 2.0f,	255, 255, 255,		0.0f, 0.75f };
    hexVertices[18] = { 0.5f, 1.0f, 0.0f,	255, 255, 255,		1.0f, 0.75f };
    hexVertices[19] = { 1.0f, 0.0f, 0.0f,	255, 255, 255,		1.0f, 0.25f };
    hexVertices[20] = { 0.5f, 1.0f, 2.0f,	255, 255, 255,		0.0f, 0.75f };
    hexVertices[21] = { 1.0f, 0.0f, 2.0f,	255, 255, 255,		0.0f, 0.25f };

    // top
    hexVertices[22] = { 0.5f, 1.0f, 2.0f,	255, 255, 255,		0.25f, 0.15f };
    hexVertices[23] = { -0.5f, 1.0f, 0.0f,	255, 255, 255,		0.25f, 0.15f };
    hexVertices[24] = { 0.5f, 1.0f, 0.0f,	255, 255, 255,		0.25f, 0.15f };
    hexVertices[25] = { -0.5f, 1.0f, 0.0f,	255, 255, 255,		0.25f, 0.15f };
    hexVertices[26] = { -0.5f, 1.0f, 2.0f,	255, 255, 255,		0.25f, 0.15f };
    hexVertices[27] = { 0.5f, 1.0f, 2.0f,	255, 255, 255,		0.25f, 0.15f };

    // top left
    hexVertices[28] = { -1.0f, 0.0f, 2.0f,	255, 255, 255,		1.0f, 0.25f };
    hexVertices[29] = { -0.5f, 1.0f, 0.0f,	255, 255, 255,		0.0f, 0.75f };
    hexVertices[30] = { -0.5f, 1.0f, 2.0f,	255, 255, 255,		1.0f, 0.75f };
    hexVertices[31] = { -1.0f, 0.0f, 2.0f,	255, 255, 255,		1.0f, 0.25f };
    hexVertices[32] = { -0.5f, 1.0f, 0.0f,	255, 255, 255,		0.0f, 0.75f };
    hexVertices[33] = { -1.0f, 0.0f, 0.0f,	255, 255, 255,		0.0f, 0.25f };

    // bottom left
    hexVertices[34] = { -0.5f, -1.0f, 2.0f,	255, 255, 255,		0.25f, 0.15f }; // bottom right
    hexVertices[35] = { -1.0f, 0.0f, 0.0f,	255, 255, 255,		0.25f, 0.15f }; // top left
    hexVertices[36] = { -1.0f, 0.0f, 2.0f,	255, 255, 255,		0.25f, 0.15f }; // top right
    hexVertices[37] = { -0.5f, -1.0f, 2.0f,	255, 255, 255,		0.25f, 0.15f }; // bottom right
    hexVertices[38] = { -1.0f, 0.0f, 0.0f,	255, 255, 255,		0.25f, 0.15f }; // top left
    hexVertices[39] = { -0.5f, -1.0f, 0.0f,	255, 255, 255,		0.25f, 0.15f }; // bottom left

    // bottom
    hexVertices[40] = { 0.5f, -1.0f, 2.0f,	255, 255, 255,		1.0f, 0.75f }; // bottom right
    hexVertices[41] = { -0.5f, -1.0f, 0.0f,	255, 255, 255,		0.0f, 0.25f }; // top left
    hexVertices[42] = { 0.5f, -1.0f, 0.0f,	255, 255, 255,		0.0f, 0.75f }; // top right
    hexVertices[43] = { -0.5f, -1.0f, 0.0f,	255, 255, 255,		0.0f, 0.25f };
    hexVertices[44] = { -0.5f, -1.0f, 2.0f,	255, 255, 255,		1.0f, 0.25f };
    hexVertices[45] = { 0.5f, -1.0f, 2.0f,	255, 255, 255,		1.0f, 0.75f };

    // bottom right
    hexVertices[46] = { 0.5f, -1.0f, 0.0f,	255, 255, 255,		0.25f, 0.15f }; // bottom right
    hexVertices[47] = { 1.0f, 0.0f, 2.0f,	255, 255, 255,		0.25f, 0.15f }; // top left
    hexVertices[48] = { 1.0f, 0.0f, 0.0f,	255, 255, 255,		0.25f, 0.15f }; // top right
    hexVertices[49] = { 0.5f, -1.0f, 0.0f,	255, 255, 255,		0.25f, 0.15f };
    hexVertices[50] = { 1.0f, 0.0f, 2.0f,	255, 255, 255,		0.25f, 0.15f };
    hexVertices[51] = { 0.5f, -1.0f, 2.0f,	255, 255, 255,		0.25f, 0.15f };
    
    // Loop to calculate normal vertices per face
    for( int i=0; i < (sizeof(hexVertices)/sizeof(hexVertices[0])); i+=6 )
    {
        glm::vec3 v1( hexVertices[i].x, hexVertices[i].y, hexVertices[i].z );
        glm::vec3 v2( hexVertices[i+1].x, hexVertices[i+1].y, hexVertices[i+1].z );
        glm::vec3 v3( hexVertices[i+2].x, hexVertices[i+2].y, hexVertices[i+2].z );
        
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
            hexVertices[j].nx = crossProd.x;
            hexVertices[j].ny = crossProd.y;
            hexVertices[j].nz = crossProd.z;
        }
    }

    
    
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
    
    
    
    // FRAME BUFFER OBJECT

    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    GLuint depthTex;
    glGenTextures(1, &depthTex);
    glBindTexture(GL_TEXTURE_2D, depthTex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
    glDrawBuffer(GL_NONE);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Error! Framebuffer not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    
	// Create a shader program
	GLuint mainProgram = CreateShaderProgram("main.vsh", "main.fsh");
    GLuint depthProgram = CreateShaderProgram("depth.vsh", "depth.fsh");
    
    glm::vec3 floorNormal = glm::vec3(floorVertices[6].nx, floorVertices[6].ny, floorVertices[6].nz);
    GLint floorNormalUniformLocation = glGetUniformLocation(mainProgram, "floorNormal");
    glUniform3fv(floorNormalUniformLocation, 1, glm::value_ptr(floorNormal));

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
    GLuint floorTex;
    glGenTextures(1, &floorTex);

    // --- Load our image using stb_image ---

    // Im image-space (pixels), (0, 0) is the upper-left corner of the image
    // However, in u-v coordinates, (0, 0) is the lower-left corner of the image
    // This means that the image will appear upside-down when we use the image data as is
    // This function tells stbi to flip the image vertically so that it is not upside-down when we use it
    stbi_set_flip_vertically_on_load(true);
    
    // 'imageWidth' and imageHeight will contain the width and height of the loaded image respectively
    int imageWidth, imageHeight, numChannels;

    // Read the image data and store it in an unsigned char array
    unsigned char* imageData = stbi_load("sand texture.jpeg", &imageWidth, &imageHeight, &numChannels, 0);

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
    

	// Create a variable that will contain the ID for our texture,
	// and use glGenTextures() to generate the texture itself
	GLuint pyramidTex;
	glGenTextures(1, &pyramidTex);

	// --- Load our image using stb_image ---

	// Im image-space (pixels), (0, 0) is the upper-left corner of the image
	// However, in u-v coordinates, (0, 0) is the lower-left corner of the image
	// This means that the image will appear upside-down when we use the image data as is
	// This function tells stbi to flip the image vertically so that it is not upside-down when we use it
	stbi_set_flip_vertically_on_load(true);

	// Read the image data and store it in an unsigned char array
	imageData = stbi_load("pyramid texture.jpeg", &imageWidth, &imageHeight, &numChannels, 0);

	// Make sure that we actually loaded the image before uploading the data to the GPU
	if (imageData != nullptr)
	{
		// Our texture is 2D, so we bind our texture to the GL_TEXTURE_2D target
		glBindTexture(GL_TEXTURE_2D, pyramidTex);

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
    GLuint pillarTex;
    glGenTextures(1, &pillarTex);

    // --- Load our image using stb_image ---

    // Im image-space (pixels), (0, 0) is the upper-left corner of the image
    // However, in u-v coordinates, (0, 0) is the lower-left corner of the image
    // This means that the image will appear upside-down when we use the image data as is
    // This function tells stbi to flip the image vertically so that it is not upside-down when we use it
    stbi_set_flip_vertically_on_load(true);

    // Read the image data and store it in an unsigned char array
    imageData = stbi_load("pillar texture.jpg", &imageWidth, &imageHeight, &numChannels, 0);

    // Make sure that we actually loaded the image before uploading the data to the GPU
    if (imageData != nullptr)
    {
        // Our texture is 2D, so we bind our texture to the GL_TEXTURE_2D target
        glBindTexture(GL_TEXTURE_2D, pillarTex);

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

		
        
        // --- SHADOW MAPPING ---
        glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -50.0f, 20.0f, -100.0f, 100.0f);
        
        glm::vec3 directionalLightDirection = glm::vec3(1.0f, -1.0f, 0.0f);
        GLint directionalLightDirectionUniformLocation = glGetUniformLocation(mainProgram, "directionalLightDirection");
        glUniform3fv(directionalLightDirectionUniformLocation, 1, glm::value_ptr(directionalLightDirection));

        glm::mat4 lightViewMatrix = glm::lookAt(directionalLightPosition, directionalLightPosition + directionalLightDirection, glm::vec3(0.0f, 1.0f, 0.0f));
        
        
        // FIRST PASS
        
        glUseProgram(depthProgram);
        
        GLint lightProjectionUniformLocation = glGetUniformLocation(depthProgram, "lightProjection");
        glUniformMatrix4fv(lightProjectionUniformLocation, 1, GL_FALSE, glm::value_ptr(lightProjection));

        GLint lightViewMatrixUniformLocation = glGetUniformLocation(depthProgram, "lightViewMatrix");
        glUniformMatrix4fv(lightViewMatrixUniformLocation, 1, GL_FALSE, glm::value_ptr(lightViewMatrix));

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        // Clear the color and depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glViewport(0, 0, 1024, 1024);
        
        // FLOOR

        // Use the vertex array object that we created
        glBindVertexArray(vaoFloor);

        // Bind our texture to texture unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTex);

        // Make our sampler in the fragment shader use texture unit 0
        GLint texUniformLocation = glGetUniformLocation(depthProgram, "tex");
        glUniform1i(texUniformLocation, 0);

        glm::mat4 floorModelMatrix = glm::mat4(1.0f);
        floorModelMatrix = glm::translate(floorModelMatrix, glm::vec3(0.0f, 40.0f, -50.0f));
        floorModelMatrix = glm::scale(floorModelMatrix, glm::vec3(100.0f, 50.0f, 150.0f));
        floorModelMatrix = glm::rotate(floorModelMatrix, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        
        glm::mat4 viewMatrix = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
        glm::mat4 projectionMatrix = glm::perspective(glm::radians(fov), windowWidth / windowHeight, 0.1f, 500.0f);
        glm::mat4 finalMatrix = projectionMatrix * viewMatrix * floorModelMatrix;

        GLint matUniformLocation = glGetUniformLocation(depthProgram, "mat");
        glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
        
        GLint modelUniformLocation = glGetUniformLocation(depthProgram, "model");
        glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, glm::value_ptr(floorModelMatrix));
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        

        // MIDDLE PYRAMID

        glBindVertexArray(vaoPyramid);
        glBindTexture(GL_TEXTURE_2D, pyramidTex);

        glm::mat4 pyramidMiddleModelMatrix = glm::mat4(1.0f);
        pyramidMiddleModelMatrix = glm::translate(pyramidMiddleModelMatrix, glm::vec3(0.0f, -10.0f, -80.0f));
        pyramidMiddleModelMatrix = glm::scale(pyramidMiddleModelMatrix, glm::vec3(30.0f, 40.0f, 30.0f));
        
        finalMatrix = projectionMatrix * viewMatrix * pyramidMiddleModelMatrix;

        matUniformLocation = glGetUniformLocation(depthProgram, "mat");
        glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
        glDrawArrays(GL_TRIANGLES, 0, 18);
        

        // LEFT PYRAMID

        glBindVertexArray(vaoPyramid);
        glBindTexture(GL_TEXTURE_2D, pyramidTex);

        glm::mat4 pyramidLeftModelMatrix = glm::mat4(1.0f);
        pyramidLeftModelMatrix = glm::translate(pyramidLeftModelMatrix, glm::vec3(-40.0f, -10.0f, -150.0f));
        pyramidLeftModelMatrix = glm::scale(pyramidLeftModelMatrix, glm::vec3(30.0f, 40.0f, 30.0f));
         
        finalMatrix = projectionMatrix * viewMatrix * pyramidLeftModelMatrix;

        matUniformLocation = glGetUniformLocation(depthProgram, "mat");
        glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
        glDrawArrays(GL_TRIANGLES, 0, 18);
        
        
        // RIGHT PYRAMID

        glBindVertexArray(vaoPyramid);
        glBindTexture(GL_TEXTURE_2D, pyramidTex);
        
        glm::mat4 pyramidRightModelMatrix = glm::mat4(1.0f);
        pyramidRightModelMatrix = glm::translate(pyramidRightModelMatrix, glm::vec3(40.0f, -10.0f, -150.0f));
        pyramidRightModelMatrix = glm::scale(pyramidRightModelMatrix, glm::vec3(30.0f, 40.0f, 30.0f));
         
        finalMatrix = projectionMatrix * viewMatrix * pyramidRightModelMatrix;

        matUniformLocation = glGetUniformLocation(depthProgram, "mat");
        glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
        glDrawArrays(GL_TRIANGLES, 0, 18);
        

        // LEFT TALL PILLAR 1

        glBindVertexArray(vaoHex);
        glBindTexture(GL_TEXTURE_2D, pillarTex);

        glm::mat4 pillarModelMatrix = glm::mat4(1.0f);
        pillarModelMatrix = glm::translate(pillarModelMatrix, glm::vec3(-30.0f, 15.0f, 70.0f));
        pillarModelMatrix = glm::scale(pillarModelMatrix, glm::vec3(2.0f, 15.0f, 2.0f));
        pillarModelMatrix = glm::rotate(pillarModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        finalMatrix = projectionMatrix * viewMatrix * pillarModelMatrix;

        matUniformLocation = glGetUniformLocation(depthProgram, "mat");
        glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));

        glDrawArrays(GL_TRIANGLE_FAN, 0, 8);
        glDrawArrays(GL_TRIANGLE_FAN, 8, 8);
        glDrawArrays(GL_TRIANGLES, 16, 36);

        
        // LEFT SHORT PILLAR 1

        pillarModelMatrix = glm::mat4(1.0f);
        pillarModelMatrix = glm::translate(pillarModelMatrix, glm::vec3(-50.0f, 10.0f, 70.0f));
        pillarModelMatrix = glm::scale(pillarModelMatrix, glm::vec3(2.0f, 10.0f, 2.0f));
        pillarModelMatrix = glm::rotate(pillarModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        finalMatrix = projectionMatrix * viewMatrix * pillarModelMatrix;

        matUniformLocation = glGetUniformLocation(depthProgram, "mat");
        glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));

        glDrawArrays(GL_TRIANGLE_FAN, 0, 8);
        glDrawArrays(GL_TRIANGLE_FAN, 8, 8);
        glDrawArrays(GL_TRIANGLES, 16, 36);
        

        // LEFT SHORT PILLAR 2

        pillarModelMatrix = glm::mat4(1.0f);
        pillarModelMatrix = glm::translate(pillarModelMatrix, glm::vec3(-70.0f, 10.0f, 70.0f));
        pillarModelMatrix = glm::scale(pillarModelMatrix, glm::vec3(2.0f, 10.0f, 2.0f));
        pillarModelMatrix = glm::rotate(pillarModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        finalMatrix = projectionMatrix * viewMatrix * pillarModelMatrix;

        matUniformLocation = glGetUniformLocation(depthProgram, "mat");
        glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));

        glDrawArrays(GL_TRIANGLE_FAN, 0, 8);
        glDrawArrays(GL_TRIANGLE_FAN, 8, 8);
        glDrawArrays(GL_TRIANGLES, 16, 36);
        

        // RIGHT TALL PILLAR 1

        pillarModelMatrix = glm::mat4(1.0f);
        pillarModelMatrix = glm::translate(pillarModelMatrix, glm::vec3(30.0f, 15.0f, 70.0f));
        pillarModelMatrix = glm::scale(pillarModelMatrix, glm::vec3(2.0f, 15.0f, 2.0f));
        pillarModelMatrix = glm::rotate(pillarModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        finalMatrix = projectionMatrix * viewMatrix * pillarModelMatrix;

        matUniformLocation = glGetUniformLocation(depthProgram, "mat");
        glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));

        glDrawArrays(GL_TRIANGLE_FAN, 0, 8);
        glDrawArrays(GL_TRIANGLE_FAN, 8, 8);
        glDrawArrays(GL_TRIANGLES, 16, 36);
        

        // RIGHT SHORT PILLAR 1

        pillarModelMatrix = glm::mat4(1.0f);
        pillarModelMatrix = glm::translate(pillarModelMatrix, glm::vec3(50.0f, 10.0f, 70.0f));
        pillarModelMatrix = glm::scale(pillarModelMatrix, glm::vec3(2.0f, 10.0f, 2.0f));
        pillarModelMatrix = glm::rotate(pillarModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        finalMatrix = projectionMatrix * viewMatrix * pillarModelMatrix;

        matUniformLocation = glGetUniformLocation(depthProgram, "mat");
        glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));

        glDrawArrays(GL_TRIANGLE_FAN, 0, 8);
        glDrawArrays(GL_TRIANGLE_FAN, 8, 8);
        glDrawArrays(GL_TRIANGLES, 16, 36);
        

        // RIGHT SHORT PILLAR 2

        pillarModelMatrix = glm::mat4(1.0f);
        pillarModelMatrix = glm::translate(pillarModelMatrix, glm::vec3(70.0f, 10.0f, 70.0f));
        pillarModelMatrix = glm::scale(pillarModelMatrix, glm::vec3(2.0f, 10.0f, 2.0f));
        pillarModelMatrix = glm::rotate(pillarModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        finalMatrix = projectionMatrix * viewMatrix * pillarModelMatrix;

        matUniformLocation = glGetUniformLocation(depthProgram, "mat");
        glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));

        glDrawArrays(GL_TRIANGLE_FAN, 0, 8);
        glDrawArrays(GL_TRIANGLE_FAN, 8, 8);
        glDrawArrays(GL_TRIANGLES, 16, 36);
        

        // LEFT TALL PILLAR 2

        pillarModelMatrix = glm::mat4(1.0f);
        pillarModelMatrix = glm::translate(pillarModelMatrix, glm::vec3(-10.0f, 15.0f, 70.0f));
        pillarModelMatrix = glm::scale(pillarModelMatrix, glm::vec3(2.0f, 15.0f, 2.0f));
        pillarModelMatrix = glm::rotate(pillarModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        finalMatrix = projectionMatrix * viewMatrix * pillarModelMatrix;

        matUniformLocation = glGetUniformLocation(depthProgram, "mat");
        glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));

        glDrawArrays(GL_TRIANGLE_FAN, 0, 8);
        glDrawArrays(GL_TRIANGLE_FAN, 8, 8);
        glDrawArrays(GL_TRIANGLES, 16, 36);
        

        // RIGHT TALL PILLAR 2

        pillarModelMatrix = glm::mat4(1.0f);
        pillarModelMatrix = glm::translate(pillarModelMatrix, glm::vec3(10.0f, 15.0f, 70.0f));
        pillarModelMatrix = glm::scale(pillarModelMatrix, glm::vec3(2.0f, 15.0f, 2.0f));
        pillarModelMatrix = glm::rotate(pillarModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        finalMatrix = projectionMatrix * viewMatrix * pillarModelMatrix;

        matUniformLocation = glGetUniformLocation(depthProgram, "mat");
        glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));

        glDrawArrays(GL_TRIANGLE_FAN, 0, 8);
        glDrawArrays(GL_TRIANGLE_FAN, 8, 8);
        glDrawArrays(GL_TRIANGLES, 16, 36);
        

        // CUBE ON TOP OF LEFT PILLARS

        glBindVertexArray(vaoCube);
        glBindTexture(GL_TEXTURE_2D, pillarTex);

        glm::mat4 cubeModelMatrix = glm::mat4(1.0f);
        cubeModelMatrix = glm::translate(cubeModelMatrix, glm::vec3(-43.0f, 11.0f, 0.0f));
        cubeModelMatrix = glm::scale(cubeModelMatrix, glm::vec3(31.0f, 1.0f, 2.1f));

        finalMatrix = projectionMatrix * viewMatrix * cubeModelMatrix;

        matUniformLocation = glGetUniformLocation(depthProgram, "mat");
        glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        

        // CUBE ON TOP OF RIGHT PILLARS

        glBindVertexArray(vaoCube);
        glBindTexture(GL_TEXTURE_2D, pillarTex);

        cubeModelMatrix = glm::mat4(1.0f);
        cubeModelMatrix = glm::translate(cubeModelMatrix, glm::vec3(43.0f, 11.0f, 0.0f));
        cubeModelMatrix = glm::scale(cubeModelMatrix, glm::vec3(31.0f, 1.0f, 2.1f));

        finalMatrix = projectionMatrix * viewMatrix * cubeModelMatrix;

        matUniformLocation = glGetUniformLocation(depthProgram, "mat");
        glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        

        // CUBE ON TOP OF MIDDLE PILLARS

        glBindVertexArray(vaoCube);
        glBindTexture(GL_TEXTURE_2D, pillarTex);

        cubeModelMatrix = glm::mat4(1.0f);
        cubeModelMatrix = glm::translate(cubeModelMatrix, glm::vec3(0.0f, 15.0f, 0.0f));
        cubeModelMatrix = glm::scale(cubeModelMatrix, glm::vec3(32.0f, 1.0f, 2.1f));

        finalMatrix = projectionMatrix * viewMatrix * cubeModelMatrix;

        matUniformLocation = glGetUniformLocation(depthProgram, "mat");
        glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        // "Unuse" the vertex array object
        glBindVertexArray(0);
        
        
        
        // SECOND PASS
        
        // Use the shader program that we created
        glUseProgram(mainProgram);
        
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glActiveTexture(depthTex);
        glBindTexture(GL_TEXTURE_2D, depthTex);
        
        GLint depthTexUniformLocation = glGetUniformLocation(mainProgram, "depthTex");
        glUniform1i(depthTexUniformLocation, 0);
        
        lightProjectionUniformLocation = glGetUniformLocation(mainProgram, "lightProjection");
        glUniformMatrix4fv(lightProjectionUniformLocation, 1, GL_FALSE, glm::value_ptr(lightProjection));

        lightViewMatrixUniformLocation = glGetUniformLocation(mainProgram, "lightViewMatrix");
        glUniformMatrix4fv(lightViewMatrixUniformLocation, 1, GL_FALSE, glm::value_ptr(lightViewMatrix));
        
        int framebufferWidth, framebufferHeight;
        glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
        glViewport(0, 0, framebufferWidth, framebufferHeight);
        
        
        // --- LIGHTING ---
        
        // Eye position
        glm::vec3 eyePosition = cameraPosition;
        GLint eyePositionUniformLocation = glGetUniformLocation(mainProgram, "eyePosition");
        glUniform3fv(eyePositionUniformLocation, 1, glm::value_ptr(eyePosition));
        
        
        // Ambient intensity FOR DIRECTIONAL LIGHT
        float ambientDirectionalIntensity = 0.8f;
        GLint ambientDirectionalIntensityUniformLocation = glGetUniformLocation(mainProgram, "ambientDirectionalIntensity");
        glUniform1f(ambientDirectionalIntensityUniformLocation, ambientDirectionalIntensity);
        
        // Ambient component FOR DIRECTIONAL LIGHT
        glm::vec3 ambientDirectionalComponent = glm::vec3(1.0f, 0.9f, 0.8f);
        GLint ambientDirectionalComponentUniformLocation = glGetUniformLocation(mainProgram, "ambientDirectionalComponent");
        glUniform3fv(ambientDirectionalComponentUniformLocation, 1, glm::value_ptr(ambientDirectionalComponent));
        
        
        // Diffuse intensity
        float diffuseIntensity = 0.8f;
        GLint diffuseIntensityUniformLocation = glGetUniformLocation(mainProgram, "diffuseIntensity");
        glUniform1f(diffuseIntensityUniformLocation, diffuseIntensity);
        
        // Diffuse component
        glm::vec3 diffuseComponent = glm::vec3(0.8f, 0.8f, 0.8f);
        GLint diffuseComponentUniformLocation = glGetUniformLocation(mainProgram, "diffuseComponent");
        glUniform3fv(diffuseComponentUniformLocation, 1, glm::value_ptr(diffuseComponent));
        
        
        // Specular intensity
        float specularIntensity = 5.0f;
        GLint specularIntensityUniformLocation = glGetUniformLocation(mainProgram, "specularIntensity");
        glUniform1f(specularIntensityUniformLocation, specularIntensity);
        
        // Specular component
        glm::vec3 specularComponent = glm::vec3(0.4f, 0.4f, 0.4f);
        GLint specularComponentUniformLocation = glGetUniformLocation(mainProgram, "specularComponent");
        glUniform3fv(specularComponentUniformLocation, 1, glm::value_ptr(specularComponent));
        
        
        // Shininess
        float shininess = 64.0f;
        GLint shininessUniformLocation = glGetUniformLocation(mainProgram, "shininess");
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
		texUniformLocation = glGetUniformLocation(mainProgram, "tex");
		glUniform1i(texUniformLocation, 0);

		floorModelMatrix = glm::mat4(1.0f);
		floorModelMatrix = glm::translate(floorModelMatrix, glm::vec3(0.0f, 40.0f, -50.0f));
		floorModelMatrix = glm::scale(floorModelMatrix, glm::vec3(100.0f, 50.0f, 150.0f));
		floorModelMatrix = glm::rotate(floorModelMatrix, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        
		viewMatrix = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
		projectionMatrix = glm::perspective(glm::radians(fov), windowWidth / windowHeight, 0.1f, 500.0f);
		finalMatrix = projectionMatrix * viewMatrix * floorModelMatrix;

		matUniformLocation = glGetUniformLocation(mainProgram, "mat");
		glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
        
        modelUniformLocation = glGetUniformLocation(mainProgram, "model");
        glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, glm::value_ptr(floorModelMatrix));
        
		glDrawArrays(GL_TRIANGLES, 0, 6);
        

		// MIDDLE PYRAMID

		glBindVertexArray(vaoPyramid);
		glBindTexture(GL_TEXTURE_2D, pyramidTex);

		pyramidMiddleModelMatrix = glm::mat4(1.0f);
        pyramidMiddleModelMatrix = glm::translate(pyramidMiddleModelMatrix, glm::vec3(0.0f, -10.0f, -80.0f));
        pyramidMiddleModelMatrix = glm::scale(pyramidMiddleModelMatrix, glm::vec3(30.0f, 40.0f, 30.0f));
        
		finalMatrix = projectionMatrix * viewMatrix * pyramidMiddleModelMatrix;

		matUniformLocation = glGetUniformLocation(mainProgram, "mat");
		glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
		glDrawArrays(GL_TRIANGLES, 0, 18);
        

		// LEFT PYRAMID

         glBindVertexArray(vaoPyramid);
         glBindTexture(GL_TEXTURE_2D, pyramidTex);

         pyramidLeftModelMatrix = glm::mat4(1.0f);
         pyramidLeftModelMatrix = glm::translate(pyramidLeftModelMatrix, glm::vec3(-40.0f, -10.0f, -150.0f));
         pyramidLeftModelMatrix = glm::scale(pyramidLeftModelMatrix, glm::vec3(30.0f, 40.0f, 30.0f));
         
         finalMatrix = projectionMatrix * viewMatrix * pyramidLeftModelMatrix;

         matUniformLocation = glGetUniformLocation(mainProgram, "mat");
         glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
         glDrawArrays(GL_TRIANGLES, 0, 18);
        
        
        // RIGHT PYRAMID

         glBindVertexArray(vaoPyramid);
         glBindTexture(GL_TEXTURE_2D, pyramidTex);

         pyramidRightModelMatrix = glm::mat4(1.0f);
         pyramidRightModelMatrix = glm::translate(pyramidRightModelMatrix, glm::vec3(40.0f, -10.0f, -150.0f));
         pyramidRightModelMatrix = glm::scale(pyramidRightModelMatrix, glm::vec3(30.0f, 40.0f, 30.0f));
         
         finalMatrix = projectionMatrix * viewMatrix * pyramidRightModelMatrix;

         matUniformLocation = glGetUniformLocation(mainProgram, "mat");
         glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
         glDrawArrays(GL_TRIANGLES, 0, 18);

        
         // LEFT TALL PILLAR 1

         glBindVertexArray(vaoHex);
         glBindTexture(GL_TEXTURE_2D, pillarTex);

         pillarModelMatrix = glm::mat4(1.0f);
         pillarModelMatrix = glm::translate(pillarModelMatrix, glm::vec3(-30.0f, 15.0f, 0.0f));
         pillarModelMatrix = glm::scale(pillarModelMatrix, glm::vec3(2.0f, 15.0f, 2.0f));
         pillarModelMatrix = glm::rotate(pillarModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

         finalMatrix = projectionMatrix * viewMatrix * pillarModelMatrix;

         matUniformLocation = glGetUniformLocation(mainProgram, "mat");
         glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));

         glDrawArrays(GL_TRIANGLE_FAN, 0, 8);
         glDrawArrays(GL_TRIANGLE_FAN, 8, 8);
         glDrawArrays(GL_TRIANGLES, 16, 36);

        
         // LEFT SHORT PILLAR 1

         pillarModelMatrix = glm::mat4(1.0f);
         pillarModelMatrix = glm::translate(pillarModelMatrix, glm::vec3(-50.0f, 10.0f, 0.0f));
         pillarModelMatrix = glm::scale(pillarModelMatrix, glm::vec3(2.0f, 10.0f, 2.0f));
         pillarModelMatrix = glm::rotate(pillarModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

         finalMatrix = projectionMatrix * viewMatrix * pillarModelMatrix;

         matUniformLocation = glGetUniformLocation(mainProgram, "mat");
         glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));

         glDrawArrays(GL_TRIANGLE_FAN, 0, 8);
         glDrawArrays(GL_TRIANGLE_FAN, 8, 8);
         glDrawArrays(GL_TRIANGLES, 16, 36);

        
         // LEFT SHORT PILLAR 2

         pillarModelMatrix = glm::mat4(1.0f);
         pillarModelMatrix = glm::translate(pillarModelMatrix, glm::vec3(-70.0f, 10.0f, 0.0f));
         pillarModelMatrix = glm::scale(pillarModelMatrix, glm::vec3(2.0f, 10.0f, 2.0f));
         pillarModelMatrix = glm::rotate(pillarModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

         finalMatrix = projectionMatrix * viewMatrix * pillarModelMatrix;

         matUniformLocation = glGetUniformLocation(mainProgram, "mat");
         glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));

         glDrawArrays(GL_TRIANGLE_FAN, 0, 8);
         glDrawArrays(GL_TRIANGLE_FAN, 8, 8);
         glDrawArrays(GL_TRIANGLES, 16, 36);

        
         // RIGHT TALL PILLAR 1

         pillarModelMatrix = glm::mat4(1.0f);
         pillarModelMatrix = glm::translate(pillarModelMatrix, glm::vec3(30.0f, 15.0f, 0.0f));
         pillarModelMatrix = glm::scale(pillarModelMatrix, glm::vec3(2.0f, 15.0f, 2.0f));
         pillarModelMatrix = glm::rotate(pillarModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

         finalMatrix = projectionMatrix * viewMatrix * pillarModelMatrix;

         matUniformLocation = glGetUniformLocation(mainProgram, "mat");
         glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));

         glDrawArrays(GL_TRIANGLE_FAN, 0, 8);
         glDrawArrays(GL_TRIANGLE_FAN, 8, 8);
         glDrawArrays(GL_TRIANGLES, 16, 36);

        
         // RIGHT SHORT PILLAR 1

         pillarModelMatrix = glm::mat4(1.0f);
         pillarModelMatrix = glm::translate(pillarModelMatrix, glm::vec3(50.0f, 10.0f, 0.0f));
         pillarModelMatrix = glm::scale(pillarModelMatrix, glm::vec3(2.0f, 10.0f, 2.0f));
         pillarModelMatrix = glm::rotate(pillarModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

         finalMatrix = projectionMatrix * viewMatrix * pillarModelMatrix;

         matUniformLocation = glGetUniformLocation(mainProgram, "mat");
         glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));

         glDrawArrays(GL_TRIANGLE_FAN, 0, 8);
         glDrawArrays(GL_TRIANGLE_FAN, 8, 8);
         glDrawArrays(GL_TRIANGLES, 16, 36);

        
         // RIGHT SHORT PILLAR 2

         pillarModelMatrix = glm::mat4(1.0f);
         pillarModelMatrix = glm::translate(pillarModelMatrix, glm::vec3(70.0f, 10.0f, 0.0f));
         pillarModelMatrix = glm::scale(pillarModelMatrix, glm::vec3(2.0f, 10.0f, 2.0f));
         pillarModelMatrix = glm::rotate(pillarModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

         finalMatrix = projectionMatrix * viewMatrix * pillarModelMatrix;

         matUniformLocation = glGetUniformLocation(mainProgram, "mat");
         glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));

         glDrawArrays(GL_TRIANGLE_FAN, 0, 8);
         glDrawArrays(GL_TRIANGLE_FAN, 8, 8);
         glDrawArrays(GL_TRIANGLES, 16, 36);

        
         // LEFT TALL PILLAR 2

         pillarModelMatrix = glm::mat4(1.0f);
         pillarModelMatrix = glm::translate(pillarModelMatrix, glm::vec3(-10.0f, 15.0f, 0.0f));
         pillarModelMatrix = glm::scale(pillarModelMatrix, glm::vec3(2.0f, 15.0f, 2.0f));
         pillarModelMatrix = glm::rotate(pillarModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

         finalMatrix = projectionMatrix * viewMatrix * pillarModelMatrix;

         matUniformLocation = glGetUniformLocation(mainProgram, "mat");
         glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));

         glDrawArrays(GL_TRIANGLE_FAN, 0, 8);
         glDrawArrays(GL_TRIANGLE_FAN, 8, 8);
         glDrawArrays(GL_TRIANGLES, 16, 36);

        
         // RIGHT TALL PILLAR 2

         pillarModelMatrix = glm::mat4(1.0f);
         pillarModelMatrix = glm::translate(pillarModelMatrix, glm::vec3(10.0f, 15.0f, 0.0f));
         pillarModelMatrix = glm::scale(pillarModelMatrix, glm::vec3(2.0f, 15.0f, 2.0f));
         pillarModelMatrix = glm::rotate(pillarModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

         finalMatrix = projectionMatrix * viewMatrix * pillarModelMatrix;

         matUniformLocation = glGetUniformLocation(mainProgram, "mat");
         glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));

         glDrawArrays(GL_TRIANGLE_FAN, 0, 8);
         glDrawArrays(GL_TRIANGLE_FAN, 8, 8);
         glDrawArrays(GL_TRIANGLES, 16, 36);

        
         // CUBE ON TOP OF LEFT PILLARS

         glBindVertexArray(vaoCube);
         glBindTexture(GL_TEXTURE_2D, pillarTex);

         cubeModelMatrix = glm::mat4(1.0f);
         cubeModelMatrix = glm::translate(cubeModelMatrix, glm::vec3(-43.0f, 11.0f, 0.0f));
         cubeModelMatrix = glm::scale(cubeModelMatrix, glm::vec3(31.0f, 1.0f, 2.1f));

         finalMatrix = projectionMatrix * viewMatrix * cubeModelMatrix;

         matUniformLocation = glGetUniformLocation(mainProgram, "mat");
         glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
         glDrawArrays(GL_TRIANGLES, 0, 36);

        
         // CUBE ON TOP OF RIGHT PILLARS

         glBindVertexArray(vaoCube);
         glBindTexture(GL_TEXTURE_2D, pillarTex);

         cubeModelMatrix = glm::mat4(1.0f);
         cubeModelMatrix = glm::translate(cubeModelMatrix, glm::vec3(43.0f, 11.0f, 0.0f));
         cubeModelMatrix = glm::scale(cubeModelMatrix, glm::vec3(31.0f, 1.0f, 2.1f));

         finalMatrix = projectionMatrix * viewMatrix * cubeModelMatrix;

         matUniformLocation = glGetUniformLocation(mainProgram, "mat");
         glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
         glDrawArrays(GL_TRIANGLES, 0, 36);

        
         // CUBE ON TOP OF MIDDLE PILLARS

         glBindVertexArray(vaoCube);
         glBindTexture(GL_TEXTURE_2D, pillarTex);

         cubeModelMatrix = glm::mat4(1.0f);
         cubeModelMatrix = glm::translate(cubeModelMatrix, glm::vec3(0.0f, 15.0f, 0.0f));
         cubeModelMatrix = glm::scale(cubeModelMatrix, glm::vec3(32.0f, 1.0f, 2.1f));

         finalMatrix = projectionMatrix * viewMatrix * cubeModelMatrix;

         matUniformLocation = glGetUniformLocation(mainProgram, "mat");
         glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
         glDrawArrays(GL_TRIANGLES, 0, 36);
         

		/*// Moving Pyramid

		glm::mat4 pyramid3ModelMatrix = glm::mat4(1.0f);
		pyramid3ModelMatrix = glm::translate(pyramid3ModelMatrix, glm::vec3(0.0f, -6.0f, -5.0f));
		pyramid3ModelMatrix = glm::scale(pyramid3ModelMatrix, glm::vec3(2.5f, 2.5f, 2.5f));
		pyramid3ModelMatrix = glm::rotate(pyramid3ModelMatrix, glm::radians(45.0f * time), glm::vec3(0.0f, 1.0f, 0.0f));
		finalMatrix = projectionMatrix * viewMatrix * pyramid3ModelMatrix;

		matUniformLocation = glGetUniformLocation(program, "mat");
		glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
		//glDrawArrays(GL_TRIANGLES, 0, 18);*/
		

		/*// Moving Hexagonal Prism
		
		glBindVertexArray(vaoHex);
		glBindTexture(GL_TEXTURE_2D, tex2);

		glm::mat4 hexModelMatrix = glm::mat4(1.0f);
		hexModelMatrix = glm::translate(hexModelMatrix, glm::vec3(5.0f, 0.0f, -7.0f));
		hexModelMatrix = glm::scale(hexModelMatrix, glm::vec3(2.0f, 2.0f, 2.0f));
		hexModelMatrix = glm::rotate(hexModelMatrix, glm::radians(50.0f * time), glm::vec3(1.0f, 1.0f, 1.0f));
		finalMatrix = projectionMatrix * viewMatrix * hexModelMatrix;

		matUniformLocation = glGetUniformLocation(program, "mat");
		glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));

		glDrawArrays(GL_TRIANGLE_FAN, 0, 8);
		glDrawArrays(GL_TRIANGLE_FAN, 8, 8);
		glDrawArrays(GL_TRIANGLES, 16, 36);

		// Static Hexagonal Prism

		glm::mat4 hex2ModelMatrix = glm::mat4(1.0f);
		hex2ModelMatrix = glm::translate(hex2ModelMatrix, glm::vec3(0.0f, -6.0f, -5.0f));
		//hex2ModelMatrix = glm::scale(hex2ModelMatrix, glm::vec3(2.0f, 2.0f, 2.0f));
		//hex2ModelMatrix = glm::rotate(hex2ModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		finalMatrix = projectionMatrix * viewMatrix * hex2ModelMatrix;

		matUniformLocation = glGetUniformLocation(program, "mat");
		glUniformMatrix4fv(matUniformLocation, 1, GL_FALSE, glm::value_ptr(finalMatrix));
		
		glDrawArrays(GL_TRIANGLE_FAN, 0, 8);
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
	glDeleteProgram(mainProgram);

	// Delete the VBO that contains our vertices
    glDeleteBuffers(1, &vboFloor);
	glDeleteBuffers(1, &vboPyramid);
    //glDeleteBuffers(1, &vboCube);
	glDeleteBuffers(1, &vboHex);

	// Delete the vertex array object
    glDeleteVertexArrays(1, &vboFloor);
	glDeleteVertexArrays(1, &vaoPyramid);
    //glDeleteVertexArrays(1, &vaoCube);
	glDeleteVertexArrays(1, &vaoHex);

	// Remember to tell GLFW to clean itself up before exiting the application
	glfwTerminate();

	return 0;
}

void keyboardInput(GLFWwindow* window)
{
	float speed = 40.0 * deltaTime;

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
