#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 viewPos;
in vec2 TexCoords;
in vec3 TangentLightPos;
in vec3 TangentViewPos;
in vec3 TangentFragPos;

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	sampler2D normalMap;
	sampler2D depthMap;
	float shininess;
};
struct Light {
    vec3 position;
	//vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

uniform Material material;
uniform Light light;
uniform float height_scale;
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir);

void main()
{
	

	//offset texture coordinates with ParallaxMapping
	vec3 TangentViewDir = normalize(TangentViewPos - TangentFragPos);
	vec2 texCoords = ParallaxMapping(TexCoords, TangentViewDir);
	if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
    discard;
	// then sample all textures with new texCoords
	vec3 norm = texture(material.normalMap, texCoords).rgb;
	norm = normalize(norm * 2.0 - 1.0);
	
	// --- Phong shading in world space
	//ambient
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, texCoords));
	//diffuse	
	
	vec3 lightDir = normalize(light.position - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * light.diffuse * vec3(texture(material.diffuse, texCoords));
	//specular
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = spec * light.specular * vec3(texture(material.specular, texCoords));
	//attenuation
	float distance = length(light.position - FragPos);
	float attenuation = 1.0/(light.constant + light.linear * distance + light.quadratic * (distance * distance));

	ambient  *= attenuation; 
	diffuse  *= attenuation;
	specular *= attenuation;   


	vec3 result = (ambient + diffuse + specular);
	FragColor = vec4(result, 1.0);
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
    // number of depth layers
    const float minLayers = 8.0;
	const float maxLayers = 32.0;
	float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy * height_scale; 
    vec2 deltaTexCoords = P / numLayers;
	// get initial values
	vec2  currentTexCoords     = texCoords;
	float currentDepthMapValue = texture(material.depthMap, currentTexCoords).r;
  
	while(currentLayerDepth < currentDepthMapValue)	
	{
		// shift texture coordinates along direction of P
		currentTexCoords -= deltaTexCoords;
		// get depthmap value at current texture coordinates
		currentDepthMapValue = texture(material.depthMap, currentTexCoords).r;  
		// get depth of next layer
		currentLayerDepth += layerDepth;  
	}
	// get texture coordinates before collision (reverse operations)
	vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

	// get depth after and before collision for linear interpolation
	float afterDepth  = currentDepthMapValue - currentLayerDepth;
	float beforeDepth = texture(material.depthMap, prevTexCoords).r - currentLayerDepth + layerDepth;
 
	// interpolation of texture coordinates
	float weight = afterDepth / (afterDepth - beforeDepth);
	vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

	return finalTexCoords;  
} 