#version 330 core

in vec2 uv;
in vec3 vertexPosition_worldspace;
in vec3 eyeDirection_cameraspace;
in vec3 normalDirection_cameraspace;
in vec3 lightDirection_cameraspace;

// Ouput data
out vec3 color;

uniform sampler2D textureSampler;
uniform mat4 MV;
uniform vec3 lightPosition_worldspace;
uniform vec3 lightColor;
uniform float  lightPower;

void main()
{
	float distance = length(lightPosition_worldspace - vertexPosition_worldspace);
	vec3 materialDiffuseColor = texture(textureSampler, uv).rgb;
	vec3 materialAmbientLight = vec3(0.1, 0.1, 0.1) * materialDiffuseColor;
	vec3 materialSpecularColor = vec3(0.3, 0.3, 0.3);
	float cosTheta = clamp(dot(lightDirection_cameraspace, normalDirection_cameraspace), 0, 1);

	// phong BRDF model
	vec3 R = reflect(-lightDirection_cameraspace, normalDirection_cameraspace);
	float cosAlpha = clamp(dot(normalDirection_cameraspace, R), 0, 1);
	
	color = materialDiffuseColor * lightColor * lightPower * cosTheta / (distance * distance);
	color = color + materialAmbientLight;
	color = color + materialDiffuseColor * lightColor * lightPower * pow(cosAlpha, 5) / (distance * distance);
}