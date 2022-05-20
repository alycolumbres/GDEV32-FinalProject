#version 330

// FRAGMENT SHADER

// UV-coordinate of the fragment (interpolated by the rasterization stage)
in vec2 outUV;

// Color of the fragment received from the vertex shader (interpolated by the rasterization stage)
in vec3 outColor;

// Final color of the fragment that will be rendered on the screen
out vec4 color;

in vec3 fragPosition;
in vec4 fragPositionLight;
in vec3 fragNormal;

// Texture unit of the texture
uniform sampler2D tex, depthTex;

uniform vec3 eyePosition, directionalLightDirection, ambientDirectionalComponent, diffuseComponent, specularComponent, floorNormal;

uniform float ambientDirectionalIntensity, diffuseIntensity, specularIntensity, shininess;

void main()
{
	// Get pixel color of the texture at the current UV coordinate
	// and output it as our final fragment color
	vec4 fragColor = texture(tex, outUV);
    
    // DIRECTIONAL LIGHT
    // AMBIENT
    vec3 directionalLightAmbient = ambientDirectionalIntensity * ambientDirectionalComponent;
    
    // DIFFUSE
    vec3 norm = normalize(fragNormal);
    
    vec3 directionalLightDirNormalized = normalize( directionalLightDirection );
    float directionalLightDiff = max(dot(norm, -directionalLightDirNormalized), 0.0f);
    vec3 directionalLightDiffuse = directionalLightDiff * (diffuseComponent * diffuseIntensity);
    
    // SPECULAR
    vec3 viewDir = normalize(eyePosition - fragPosition);
    
    vec3 directionalLightReflectDir = reflect(directionalLightDirNormalized, norm);
    float directionalLightSpec = pow(max(dot(directionalLightReflectDir, viewDir), 0.0), shininess);
    vec3 directionalLightSpecular = directionalLightSpec * (specularComponent * specularIntensity);
    
    // Shadow calculation
    vec3 fragLightNDC = vec3(fragPositionLight) / fragPositionLight.w;  // homogeneous to cartesian
    fragLightNDC = (fragLightNDC + 1) / 2;                              // [-1, 1] to [0, 1]
    float depthValue = texture(depthTex, fragLightNDC.xy).x;
    
    // Removing shadow acne
    float bias = max(0.05 * (1.0 - dot(floorNormal, directionalLightDirection)), 0.005);
    
    if(depthValue < fragLightNDC.z)
    {
        // Final
        vec3 finalColor = directionalLightAmbient * vec3(fragColor);
        color = vec4(finalColor, 1.f);
    }
    else
    {
        vec3 finalColor = (directionalLightAmbient +
                           directionalLightDiffuse +
                           directionalLightSpecular) * vec3(fragColor);
        color = vec4(finalColor, 1.f);
    }
}
