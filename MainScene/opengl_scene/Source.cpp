#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Vertices.h"
#include "Shader.h"
#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include <vector>
#include <iostream>
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
unsigned int loadCubemap(std::vector<std::string> faces);
void renderWall();

const unsigned int SCR_WIDTH = 1400;
const unsigned int SCR_HEIGHT = 950;
// Camera settings
Camera camera(glm::vec3(0.0f, 1.0f, 3.0f));
bool firstMouse = true;
float lastX =  SCR_WIDTH / 2.0f;
float lastY =  SCR_HEIGHT / 2.0f;

//timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Light position
glm::vec3 lightPos(1.2f, 1.0f, 3.0f);

std::vector<glm::vec3> windows
{
	glm::vec3(-1.5f, 0.0f, -0.48f),
	glm::vec3(1.5f, 0.0f, 0.51f),
	glm::vec3(0.0f, 0.0f, 0.7f),
	glm::vec3(-0.3f, 0.0f, -2.3f),
	glm::vec3(0.5f, 0.0f, -0.6f)
};
// Post-effects
float negative = 0.0f;
float grayscale = 0.0f;
// normal mapping VAO
unsigned int wallVAO = 0;
unsigned int wallVBO;

int main()
{
	// Initialising glfw, glad and window
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "openGL project", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	//capturing the mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	// Configuring the global OpenGL state
	glEnable(GL_DEPTH_TEST);

	// Configuring blending for semi-transparent windows
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// Compiling and linking the shader programs
	Shader phongShader("phong.vs", "phong.fs");
	Shader lightShader("light.vs", "light.fs");
	Shader blendShader("blend.vs", "blend.fs");
	Shader screenShader("screen.vs", "screen.fs");
	Shader normalShader("normal_mapping.vs", "normal_mapping.fs");
	Shader parallaxShader("parallax.vs", "parallax.fs");
	Shader skyboxShader("skybox.vs", "skybox.fs");
	Shader reflectShader("reflect.vs", "reflect.fs");
	Shader refractShader("reflect.vs", "refract.fs");
	// cubeVAO and VBO
	unsigned int cubeVBO, cubeVAO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	// lightVAO, vbo is the same
	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

	//planeVAO
	unsigned int planeVBO, planeVAO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	//billboardVAO
	unsigned int billboardVBO, billboardVAO;
	glGenVertexArrays(1, &billboardVAO);
	glGenBuffers(1, &billboardVBO);
	glBindVertexArray(billboardVAO);
	glBindBuffer(GL_ARRAY_BUFFER, billboardVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(billboardVertices), &billboardVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	// screen quad VAO
	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	//skybox VAO
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	
	// Loading the textures(diffuse and specular maps);
	unsigned int diffuseMap = loadTexture("Resources/container2.png");
	unsigned int specularMap = loadTexture("Resources/container2_specular.png");
	unsigned int floorTexture = loadTexture("Resources/floor.jpg");
	unsigned int billboardTexture = loadTexture("Resources/blending_transparent_window.png");
	unsigned int wallTexture = loadTexture("Resources/wall-4-granite-DIFFUSE.jpg");
	unsigned int wallNormal = loadTexture("Resources/wall-4-granite-NORMAL.jpg");
	unsigned int bricksTexture = loadTexture("Resources/leaf.jpg");
	unsigned int bricksNormal = loadTexture("Resources/leaf_norm.jpg");
	unsigned int bricksHeight = loadTexture("Resources/leaf_disp.jpg");
	unsigned int woodTexture = loadTexture("Resources/wood.png");
	unsigned int woodNormal = loadTexture("Resources/toy_box_normal.png");
	unsigned int woodHeight = loadTexture("Resources/toy_box_disp.png");

	/*std::vector<std::string> faces
	{
		"Resources/skybox/right.jpg",
		"Resources/skybox/left.jpg",
		"Resources/skybox/top.jpg",
		"Resources/skybox/bottom.jpg",
		"Resources/skybox/front.jpg",
		"Resources/skybox/back.jpg"
	}; */

	std::vector<std::string> faces
	{
		"Resources/skybox/right1.jpg",
		"Resources/skybox/left1.jpg",
		"Resources/skybox/top1.jpg",
		"Resources/skybox/bottom1.jpg",
		"Resources/skybox/front1.jpg",
		"Resources/skybox/back1.jpg"
	}; 
	unsigned int cubemapTexture = loadCubemap(faces);


	// Framebuffer configuration
	unsigned int framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	// Texture for framebuffer
	unsigned int textureColorbuffer;
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
	// Renderbuffer object for depth and stencil tesing
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); // single rbo for both depth and stencil testing
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); //attaching the rbo
	// Checking if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete !" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Render loop
	while (!glfwWindowShouldClose(window))
	{
		// Timing
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//sorting the billboards
		std::map<float, glm::vec3> sorted;
		for (unsigned int i = 0; i < windows.size(); i++)
		{
			float distance = glm::length(camera.Position - windows[i]);
			sorted[distance] = windows[i];
		}

		processInput(window);
		// Rendering
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glEnable(GL_DEPTH_TEST);


		glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// ======= Drawing the scene with phong shading
		phongShader.use();
		phongShader.setVec3("viewPos", camera.Position);
		//light properties
		phongShader.setVec3("light.position", lightPos);
		phongShader.setVec3("light.ambient", 0.35f, 0.35f, 0.35f);
		phongShader.setVec3("light.diffuse", 0.6f, 0.6f, 0.6f); 
		phongShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		phongShader.setFloat("light.constant", 1.0f);
		phongShader.setFloat("light.linear", 0.045f);
		phongShader.setFloat("light.quadratic", 0.0075f);
		//material properties
		phongShader.setInt("material.diffuse", 0);
		phongShader.setInt("material.specular", 1);
		phongShader.setFloat("material.shininess", 64.0f);
		//setting the view, projection matrices
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		phongShader.setMat4("projection", projection);
		phongShader.setMat4("view", view);
		// The cube
		glm::mat4 model = glm::mat4(1.0f);
		phongShader.setMat4("model", model);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specularMap);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		// The plane
		phongShader.setVec3("light.ambient", 0.45f, 0.45f, 0.45f);
		phongShader.setVec3("light.diffuse", 0.7f, 0.7f, 0.7f); 
		phongShader.setInt("material.specular", 0);
		phongShader.setFloat("material.shininess", 32.0f);
		model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
		model = glm::scale(model, glm::vec3(2.0f));
		phongShader.setMat4("model", model);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, floorTexture);
		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// The brickwall normal mapping
		normalShader.use();
		normalShader.setVec3("viewPos", camera.Position);
		//light properties
		normalShader.setVec3("light.position", lightPos);
		normalShader.setVec3("light.ambient", 0.35f, 0.35f, 0.35f);
		normalShader.setVec3("light.diffuse", 0.6f, 0.6f, 0.6f);
		normalShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		normalShader.setFloat("light.constant", 1.0f);
		normalShader.setFloat("light.linear", 0.045f);
		normalShader.setFloat("light.quadratic", 0.0075f);
		//material properties
		normalShader.setInt("material.diffuse", 0);
		normalShader.setInt("material.specular", 1);
		normalShader.setInt("material.normalMap", 2);
		normalShader.setFloat("material.shininess", 64.0f);
		normalShader.setMat4("projection", projection);
		normalShader.setMat4("view", view);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(3.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		normalShader.setMat4("model", model);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, wallTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, wallTexture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, wallNormal);
		renderWall();
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(5.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		normalShader.setMat4("model", model);
		renderWall();
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(4.0f, 0.0f, 1.0f));
		model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		normalShader.setMat4("model", model);
		renderWall();
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(4.0f, 0.0f, -1.0f));
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		normalShader.setMat4("model", model);
		renderWall();
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(4.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		normalShader.setMat4("model", model);
		renderWall();

		// Red bricks parallax mapping
		parallaxShader.use();
		parallaxShader.setVec3("viewPosition", camera.Position);
		//light properties
		parallaxShader.setVec3("light.position", lightPos);
		parallaxShader.setVec3("light.ambient", 0.35f, 0.35f, 0.35f);
		parallaxShader.setVec3("light.diffuse", 0.6f, 0.6f, 0.6f);
		parallaxShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		parallaxShader.setFloat("light.constant", 1.0f);
		parallaxShader.setFloat("light.linear", 0.045f);
		parallaxShader.setFloat("light.quadratic", 0.0075f);
		//material properties
		parallaxShader.setInt("material.diffuse", 0);
		parallaxShader.setInt("material.specular", 1);
		parallaxShader.setInt("material.normalMap", 2);
		parallaxShader.setInt("material.depthMap", 3);
		parallaxShader.setFloat("height_scale", 0.1f);
		parallaxShader.setFloat("material.shininess", 64.0f);
		parallaxShader.setMat4("projection", projection);
		parallaxShader.setMat4("view", view);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-4.5f, 0.5f, 0.0f));
		model = glm::rotate(model, glm::radians(50.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		parallaxShader.setMat4("model", model);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, bricksTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, bricksTexture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, bricksNormal);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, bricksHeight);
		renderWall();
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-2.5f, 0.5f, -1.5f));
		model = glm::rotate(model, glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, woodTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, woodTexture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, woodNormal);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, woodHeight);
		parallaxShader.setMat4("model", model);
		parallaxShader.setFloat("height_scale", 0.2f);
		renderWall();

		// reflecting cube
		reflectShader.use();
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(2.0f, 3.0f, 0.0f));
		reflectShader.setMat4("model", model);
		reflectShader.setMat4("projection", projection);
		reflectShader.setMat4("view", view);
		reflectShader.setVec3("cameraPos", camera.Position);
		reflectShader.setInt("skybox", 0);
		glBindVertexArray(cubeVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		// refracting cube
		refractShader.use();
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-2.0f, 3.0f, 0.0f));
		refractShader.use();
		refractShader.setMat4("model", model);
		refractShader.setMat4("projection", projection);
		refractShader.setMat4("view", view);
		refractShader.setVec3("cameraPos", camera.Position);
		refractShader.setInt("skybox", 0);
		glBindVertexArray(cubeVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);



		// The cubemap
		glDepthFunc(GL_LEQUAL);
		skyboxShader.use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);
		skyboxShader.setInt("skybox", 0);
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);



		// ======= Drawing the light source
		lightShader.use();
		lightShader.setMat4("projection", projection);
		view = camera.GetViewMatrix();
		lightShader.setMat4("view", view);
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f));
		
		lightShader.setMat4("model", model);
		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// The billboard
		blendShader.use();
		blendShader.setVec3("up", camera.Up);
		blendShader.setVec3("right", camera.Right);
		blendShader.setMat4("projection", projection);
		blendShader.setMat4("view", view);
		model = glm::mat4(1.0f);
		blendShader.setMat4("model", model);
		glBindVertexArray(billboardVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, billboardTexture);
		
		for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, it->second);
			blendShader.setMat4("model", model);
			glBindVertexArray(billboardVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		

		// Back to default framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		screenShader.use();
		screenShader.setFloat("negative", negative);
		screenShader.setFloat("grayscale", grayscale);
		glBindVertexArray(quadVAO);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
		glDrawArrays(GL_TRIANGLES, 0, 6);




		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteBuffers(1, &cubeVBO);

	glfwTerminate();
	return 0;

}



void renderWall()
{
	if (wallVAO == 0)
	{
		//positions 
		glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
		glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
		glm::vec3 pos3(1.0f, -1.0f, 0.0f);
		glm::vec3 pos4(1.0f, 1.0f, 0.0f);
		// texture coordinates
		glm::vec2 uv1(0.0f, 1.0f);
		glm::vec2 uv2(0.0f, 0.0f);
		glm::vec2 uv3(1.0f, 0.0f);
		glm::vec2 uv4(1.0f, 1.0f);
		// normal vector
		glm::vec3 nm(0.0f, 0.0f, 1.0f);

		// calculate tangent and bitangent vectors for both triangles
		glm::vec3 tangent1, bitangent1;
		glm::vec3 tangent2, bitangent2;
		// 1st triangle
		glm::vec3 edge1 = pos2 - pos1;
		glm::vec3 edge2 = pos3 - pos1;
		glm::vec2 deltaUV1 = uv2 - uv1;
		glm::vec2 deltaUV2 = uv3 - uv1;
		GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent1 = glm::normalize(tangent1);

		bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent1 = glm::normalize(bitangent1);
		// 2nd triangle
		edge1 = pos3 - pos1;
		edge2 = pos4 - pos1;
		deltaUV1 = uv3 - uv1;
		deltaUV2 = uv4 - uv1;
		f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
		tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent2 = glm::normalize(tangent2);
		bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent2 = glm::normalize(bitangent2);

		float wallVertices[] = {
			// positions            // normal         // texcoords  // tangent                          // bitangent
			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
			pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
		};
		// configure plane VAO
		glGenVertexArrays(1, &wallVAO);
		glGenBuffers(1, &wallVBO);
		glBindVertexArray(wallVAO);
		glBindBuffer(GL_ARRAY_BUFFER, wallVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(wallVertices), &wallVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
	}
	glBindVertexArray(wallVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void processInput(GLFWwindow *window)
{
	// Closing window
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
	// Camera movement
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	// Light movement
	float lightSpeed = 5.0f;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		lightPos += glm::vec3(0.0f, 0.0f, -1.0f) * deltaTime * lightSpeed;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		lightPos += glm::vec3(0.0f, 0.0f, 1.0f) * deltaTime * lightSpeed;
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		lightPos += glm::vec3(-1.0f, 0.0f, 0.0f) * deltaTime * lightSpeed;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		lightPos += glm::vec3(1.0f, 0.0f, 0.0f) * deltaTime * lightSpeed;
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
		lightPos += glm::vec3(0.0f, 1.0f, 0.0f) * deltaTime * lightSpeed;
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
		lightPos += glm::vec3(0.0f, -1.0f, 0.0f) * deltaTime * lightSpeed;
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
	{
		negative += 1.0;
		if (negative > 250.0)
			negative = 0.0;
	}
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
	{
		grayscale += 1.0;
		if (grayscale > 250.0)
			grayscale = 0.0;
	}
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

// loads a cubemap texture from 6 texture faces
// 
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}
