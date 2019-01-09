#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <GL/glew.h>

#include <GLFW/glfw3.h>
GLFWwindow *window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "common/shader.hpp"
#include "common/texture.hpp"
#include "common/controls.hpp"
#include "common/myobjloader.hpp"

int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "Playground", NULL, NULL);
	if (window == NULL)
	{
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// 加载OBJ信息
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ("suzanne.obj", vertices, uvs, normals);
	if (!res)
	{
		printf("load object file answered fail, exit!\n");
		exit(-1);
	}

	// Initialize GLEW
	// 请注意，我们在初始化GLEW之前设置glewExperimental变量的值为GL_TRUE，这样做能让GLEW在管理OpenGL的函数指针时更多地使用现代化的技术.
	// 如果把它设置为GL_FALSE的话可能会在使用OpenGL的核心模式时出现一些问题。
	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// initial settings
	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	//如果一个段比之前的某一个更接近相机则接受它
	glDepthFunc(GL_LESS);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	GLuint vertexbuffer;
	// 申请显存
	glGenBuffers(1, &vertexbuffer);
	// 绑定该缓存并把数据传过去
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	// 加载法向量并复制到缓存区
	GLuint normalBuffer;
	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normals.size(), &normals[0], GL_STATIC_DRAW);

	// 加载纹理
	GLuint texture = loadDDS("uvmap.DDS");
	GLuint uvBuffer;
	glGenBuffers(1, &uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * uvs.size(), &uvs[0], GL_STATIC_DRAW);

	GLuint programID = LoadShaders("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");

	// 加入MVP进行摄影机透视
	// 物体放在原点不动
	mat4 modelMatrix = mat4(1.0f);
	GLuint mvp_id = glGetUniformLocation(programID, "MVP");
	GLuint model_id = glGetUniformLocation(programID, "M");
	GLuint view_id = glGetUniformLocation(programID, "V");

	// 准备光源信息
	GLuint lightId = glGetUniformLocation(programID, "LightPosition_worldspace");
	glm::vec3 lightPos;

	GLuint sampler_id = glGetUniformLocation(programID, "textureSampler");
	mat4 MVP, viewMatrix;

	do
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(programID);

		computeMatricesFromInputs();

		// 将矩阵传到显卡buffer中
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		// appint the texture sampler
		// 默认情况0号就是激活的，这里只是显式的调用一次
		// 纹理单元的编号可以通过着色器访问到
		// 可以看见这里设定了uniform告知着色器使用那个Texture Unit
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		// 纹理的采样处理
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);
		glUniform1i(sampler_id, 0);

		// 计算新的MVP矩阵传给着色器
		// TODO: 加入键盘鼠标读入，计算新的P, V矩阵
		viewMatrix = getViewMatrix();
		MVP = getProjectionMatrix() * viewMatrix * modelMatrix;
		glUniformMatrix4fv(mvp_id, 1, GL_FALSE, &MVP[0][0]);

		// 将模型和视角矩阵也传给显卡，因为着色器需要模型表面的向量和视线方向
		glUniformMatrix4fv(model_id, 1, GL_FALSE, &modelMatrix[0][0]);
		glUniformMatrix4fv(view_id, 1, GL_FALSE, &viewMatrix[0][0]);

		// 处理光源信息
		lightPos = glm::vec3(4, 4, 4); // 一个静态光源
		glUniform3f(lightId, lightPos.x, lightPos.y, lightPos.z);

		// 调用绘制命令
		glDrawArrays(GL_TRIANGLES, 0, 3 * 12);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0);

	//clean up
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvBuffer);
	glDeleteBuffers(1, &normalBuffer);
	glDeleteTextures(1, &texture);
	glDeleteVertexArrays(1, &VertexArrayID);
	//glDeleteProgram(programID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}