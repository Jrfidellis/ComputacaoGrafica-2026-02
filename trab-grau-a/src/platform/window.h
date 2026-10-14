#pragma once

#include "render/gl.h"

#include <string>

// Cria a janela, ativa o contexto, carrega a GLAD e imprime as informacoes de
// versao. Devolve nullptr em caso de falha.
GLFWwindow *createWindow(int largura, int altura, const std::string &titulo);

// Raiz do projeto, gravada pelo CMake em tempo de configuracao.
// Resolve os caminhos de assets e shaders independente do diretorio de onde o
// jogo foi aberto: terminal, VS Code ou Finder dao o mesmo resultado.
std::string projectRoot();

// Salva a tela atual num PNG. Usa o tamanho do FRAMEBUFFER, e nao o da janela,
// por causa das telas Retina, onde os dois sao diferentes.
bool saveScreenshot(GLFWwindow *janela, const std::string &caminho);
