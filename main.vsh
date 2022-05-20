#version 330

// VERTEX SHADER

// Vertex position
layout(location = 0) in vec3 vertexPosition;

// Vertex color
layout(location = 1) in vec3 vertexColor;

// Vertex UV coordinate
layout(location = 2) in vec2 vertexUV;

// Vertex normal
layout(location = 3) in vec3 vertexNormal;

// UV coordinate (will be passed to the fragment shader)
out vec2 outUV;

// Color (will be passed to the fragment shader)
out vec3 outColor;

out vec3 fragPosition;
out vec4 fragPositionLight;
out vec3 fragNormal;

uniform mat4 mat, model, lightProjection, lightViewMatrix;


void main()
{
	// Convert our vertex position to homogeneous coordinates by introducing the w-component.
	// Vertex positions are ... positions, so we specify the w-coordinate as 1.0.
    
    fragPosition = vec3(model * vec4(vertexPosition, 1.f));
    fragPositionLight = lightProjection * lightViewMatrix * model * vec4(vertexPosition, 1.f);
    fragNormal = mat3(transpose(inverse(model))) * vertexNormal;
    vec4 finalPosition = mat * vec4(vertexPosition, 1.0);

	// Give OpenGL the final position of our vertex
	gl_Position = finalPosition;

	outUV = vertexUV;
	outColor = vertexColor;
}
