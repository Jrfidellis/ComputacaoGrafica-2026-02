#pragma once

#include "core/types.h"
#include "render/gl.h"

// Traduz o teclado e o mouse da GLFW no InputState que a camada de jogo
// entende. Este e o unico lugar do programa onde glfwGetKey e chamado.
//
// POLLING contra CALLBACK, a decisao de projeto deste arquivo:
//
//   acoes CONTINUAS (girar, acelerar, metralhadora) usam glfwGetKey, que le
//   o estado atual da tecla a cada frame;
//
//   acoes de UMA VEZ (soltar bomba, ligar hitboxes, reiniciar) usam deteccao
//   de borda, comparando com o estado do frame anterior.
//
// Usar o callback de teclado para movimento produz o defeito classico: o
// aviao anda um passo, para por meio segundo ate o repeat do sistema entrar,
// e so entao se move continuamente.
//
// ESQUEMA DE CONTROLES
//
//   setas ou WASD   voo: girar o nariz e ajustar a potencia   (polling)
//   espaco          metralhadora, municao infinita            (polling)
//   B               bomba                                     (borda)
//   H, R, ESC       hitboxes, reiniciar, sair                 (borda)
struct InputReader
{
	bool bombaAnterior = false;
	bool hitboxAnterior = false;
	bool restartAnterior = false;
};

InputState readInput(InputReader &leitor, GLFWwindow *janela);
