#pragma once

#include "render/gl.h"

#include <string>

// Carrega, compila e linka um par de shaders a partir de arquivos em disco.
// Devolve 0 em caso de falha, depois de imprimir o log no console.
GLuint loadShaderProgram(const std::string &caminhoVert, const std::string &caminhoFrag);
