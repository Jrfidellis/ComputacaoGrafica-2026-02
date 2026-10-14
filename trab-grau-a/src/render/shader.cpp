#include "render/shader.h"

#include <fstream>
#include <iostream>
#include <sstream>

void glCheck(const char *rotulo)
{
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
		std::cerr << "[gl] erro 0x" << std::hex << err << std::dec << " em " << rotulo << std::endl;
}

static std::string lerArquivo(const std::string &caminho, bool &ok)
{
	std::ifstream f(caminho);
	if (!f.is_open())
	{
		std::cerr << "[shader] nao abriu " << caminho << std::endl;
		ok = false;
		return "";
	}
	std::stringstream ss;
	ss << f.rdbuf();
	ok = true;
	return ss.str();
}

static GLuint compilar(GLenum tipo, const std::string &fonte, const std::string &nome)
{
	GLuint id = glCreateShader(tipo);
	const GLchar *ptr = fonte.c_str();
	glShaderSource(id, 1, &ptr, nullptr);
	glCompileShader(id);

	GLint ok = GL_FALSE;
	glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
	if (!ok)
	{
		GLchar log[1024];
		glGetShaderInfoLog(id, sizeof(log), nullptr, log);
		std::cerr << "[shader] FALHA ao compilar " << nome << "\n"
				  << log << std::endl;
		glDeleteShader(id);
		return 0;
	}
	return id;
}

GLuint loadShaderProgram(const std::string &caminhoVert, const std::string &caminhoFrag)
{
	bool okV = false, okF = false;
	std::string fonteV = lerArquivo(caminhoVert, okV);
	std::string fonteF = lerArquivo(caminhoFrag, okF);
	if (!okV || !okF)
		return 0;

	GLuint vs = compilar(GL_VERTEX_SHADER, fonteV, caminhoVert);
	if (!vs)
		return 0;

	GLuint fs = compilar(GL_FRAGMENT_SHADER, fonteF, caminhoFrag);
	if (!fs)
	{
		glDeleteShader(vs);
		return 0;
	}

	GLuint prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);

	GLint ok = GL_FALSE;
	glGetProgramiv(prog, GL_LINK_STATUS, &ok);
	if (!ok)
	{
		GLchar log[1024];
		glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
		std::cerr << "[shader] FALHA ao linkar\n"
				  << log << std::endl;
		glDeleteProgram(prog);
		prog = 0;
	}

	glDeleteShader(vs);
	glDeleteShader(fs);

	if (prog)
		std::cout << "[shader] programa " << prog << " pronto" << std::endl;
	return prog;
}
