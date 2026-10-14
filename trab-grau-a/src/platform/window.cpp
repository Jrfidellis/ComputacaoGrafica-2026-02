#include "platform/window.h"

#include <iostream>
#include <vector>

#include "stb_image_write.h"

#ifndef PROJECT_ROOT
#define PROJECT_ROOT "."
#endif

std::string projectRoot()
{
	return std::string(PROJECT_ROOT);
}

GLFWwindow *createWindow(int largura, int altura, const std::string &titulo)
{
	if (!glfwInit())
	{
		std::cerr << "[janela] glfwInit falhou" << std::endl;
		return nullptr;
	}

	// Contexto 4.0 core com forward compatibility. No macOS essas quatro
	// linhas sao obrigatorias: sem elas o sistema entrega um contexto 2.1
	// legado, onde nem VAO nem "#version 400" existem.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *janela = glfwCreateWindow(largura, altura, titulo.c_str(), nullptr, nullptr);
	if (!janela)
	{
		std::cerr << "[janela] glfwCreateWindow falhou" << std::endl;
		glfwTerminate();
		return nullptr;
	}

	glfwMakeContextCurrent(janela);
	glfwSwapInterval(1);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "[janela] gladLoadGLLoader falhou" << std::endl;
		glfwDestroyWindow(janela);
		glfwTerminate();
		return nullptr;
	}

	std::cout << "[gl] Renderer : " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "[gl] Versao   : " << glGetString(GL_VERSION) << std::endl;
	std::cout << "[gl] GLSL     : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	if (!GLAD_GL_VERSION_3_3)
		std::cerr << "[gl] AVISO: contexto abaixo de 3.3, o trabalho exige 3.3+" << std::endl;

	return janela;
}

bool saveScreenshot(GLFWwindow *janela, const std::string &caminho)
{
	int fw = 0, fh = 0;
	glfwGetFramebufferSize(janela, &fw, &fh);
	if (fw <= 0 || fh <= 0)
		return false;

	std::vector<unsigned char> pixels((size_t)fw * fh * 4);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, fw, fh, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

	// glReadPixels entrega de baixo para cima; o PNG grava de cima para baixo.
	stbi_flip_vertically_on_write(1);
	int ok = stbi_write_png(caminho.c_str(), fw, fh, 4, pixels.data(), fw * 4);
	stbi_flip_vertically_on_write(0);

	if (ok)
		std::cout << "[screenshot] " << caminho << " (" << fw << "x" << fh << ")" << std::endl;
	else
		std::cerr << "[screenshot] falhou ao gravar " << caminho << std::endl;

	return ok != 0;
}
