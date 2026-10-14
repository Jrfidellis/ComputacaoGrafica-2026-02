#pragma once

// Unico lugar que define a ordem correta de inclusao: a GLAD precisa vir
// ANTES da GLFW, senao a GLFW puxa os headers de OpenGL do sistema e o
// compilador reclama de redefinicao.
//
// Nenhum arquivo de src/game/ inclui este header. O CMake garante isso: o
// alvo game_core nao tem vendor/ no include path, entao a tentativa falha na
// compilacao, e nao na revisao de codigo.

#include <glad/glad.h>

#include <GLFW/glfw3.h>

// Verifica glGetError() e imprime o rotulo se algo deu errado.
// No macOS nao existe glDebugMessageCallback (e recurso de GL 4.3 e a Apple
// parou na 4.1), entao esta e a unica instrumentacao disponivel.
void glCheck(const char *rotulo);
