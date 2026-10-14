#pragma once

#include "core/types.h"

struct Renderer;
struct World;

// HUD desenhado em espaco de tela, ou seja com view = identidade.
// Pontuacao, vida, estoque de bombas e as mensagens de fim de jogo.
void drawHud(Renderer &r, const World &w);

// Desenha um numero inteiro usando os tiles de digito da folha.
// Nao precisa de FreeType nem de fonte embutida: os digitos 0 a 9 ja estao
// no tiles_packed.png, nos tiles 19-23 e 31-35.
// Devolve a largura total desenhada, em pixels.
float drawNumber(Renderer &r, int valor, Vec2 posEsquerda, float alturaDigito);
