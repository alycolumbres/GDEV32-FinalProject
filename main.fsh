#version 330

// FRAGMENT SHADER

// UV-coordinate of the fragment (interpolated by the rasterization stage)
in vec2 outUV;

// Color of the fragment received from the vertex shader (interpolated by the rasterization stage)
in vec3 outColor;

// Final color of the fragment that will be rendered on the screen
out vec4 color;

in vec3 fragPosition;
in vec3 fragNormal;

// Texture unit of the texture
uniform sampler2D tex;

uniform vec3 pointLightPosition, ambientPointComponent, ambientDirectionalComponent, diffuseComponent, specularComponent, eyePosition;
uniform float ambientPointIntensity, ambientDirectionalIntensity, diffuseIntensity, specularIntensity, shininess;

void main()
{
	// Get pixel color of the texture at the current UV coordinate
	// and output it as our final fragment color
	vec4 fragColor = texture(tex, outUV);
    
    // AMBIENT
        
    // For directional light
    vec3 directionalLightAmbient = ambientDirectionalIntensity * ambientDirectionalComponent;
    
    
    // DIFFUSE
    vec3 norm = normalize(fragNormal);
    
    // For directional light
    vec3 directionalLightDir = normalize( vec3(0, -1, 0) );
    float directionalLightDiff = max(dot(norm, -directionalLightDir), 0.0f);
    vec3 directionalLightDiffuse = directionalLightDiff * (diffuseComponent * diffuseIntensity);
    
    
    // SPECULAR
    vec3 viewDir = normalize(eyePosition - fragPosition);
    
    // For directional light
    vec3 directionalLightReflectDir = reflect(directionalLightDir, norm);
    float directionalLightSpec = pow(max(dot(directionalLightReflectDir, viewDir), 0.0), shininess);
    vec3 directionalLightSpecular = directionalLightSpec * (specularComponent * specularIntensity);
    
    
    // Final
    vec3 finalColor = (directionalLightAmbient + directionalLightDiffuse + directionalLightSpecular) * vec3(fragColor);
    //vec3 finalColor = vec3(fragColor);
    color = vec4(finalColor, 1.f);
}
