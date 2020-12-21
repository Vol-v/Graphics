#version 330 core
out vec4 FragColor;

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	sampler2D normalMap;
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
in mat3 TBN;
in vec3 FragPos;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

void main()
{
	// normal map
	vec3 norm = texture(material.normalMap, TexCoords).rgb;
	norm = normalize(norm * 2.0 - 1.0);
	norm = normalize(TBN * norm);
	
	// --- Phong shading in world space
	//ambient
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
	//diffuse	
	
	vec3 lightDir = normalize(light.position - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * light.diffuse * vec3(texture(material.diffuse, TexCoords));
	//specular
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = spec * light.specular * vec3(texture(material.specular, TexCoords));
	//attenuation
	float distance = length(light.position - FragPos);
	float attenuation = 1.0/(light.constant + light.linear * distance + light.quadratic * (distance * distance));

	ambient  *= attenuation; 
	diffuse  *= attenuation;
	specular *= attenuation;   


	vec3 result = (ambient + diffuse + specular);
	FragColor = vec4(result, 1.0);
}