#version 330

// depth map texture vertex shader

// Vertex position
layout(location = 0) in vec3 vertexPosition;

uniform mat4 lightProjection, lightViewMatrix, model;

void main()
{
	gl_Position = lightProjection * lightViewMatrix * model * vec4(vertexPosition, 1.0f);
}
