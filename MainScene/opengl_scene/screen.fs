#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float negative;
uniform float grayscale;
void main()
{
	vec3 color;
	if (negative > 125.0)
	{
		color = vec3(1.0 - texture(screenTexture, TexCoords));
	}
	else
	{
		color = vec3(texture(screenTexture, TexCoords));
	}
	if (grayscale > 125.0)
	{
		FragColor = texture(screenTexture, TexCoords);
		float average = 0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b;
		FragColor = vec4(average, average, average, 1.0);
	}
	else 
	{
		FragColor = vec4(color, 1.0);
	}
}