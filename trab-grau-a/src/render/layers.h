#pragma once

struct Renderer;
struct World;

// As camadas do cenario, desenhadas nesta ordem.
//
// O parallax e uma linha por camada: cada uma usa uma matriz de view com um
// FATOR diferente aplicado ao deslocamento da camera.
//
//   view_camada = translate(-camera * fator)
//
// Num jogo visto de cima a camera esta no alto olhando para baixo, entao o
// CHAO e o ponto mais distante e nada pode rolar mais devagar que ele. As
// nuvens ficam entre a camera e o chao e por isso rolam mais rapido: quanto
// mais alta a nuvem, mais perto da camera e maior o fator.
//
//   chao          1.00
//   nuvem baixa   1.15
//   nuvem alta    1.60

// O terreno, 1:1 com a camera. Desenha apenas os tiles visiveis.
void drawTerrain(Renderer &r, const World &w);

// Nuvens baixas, entre o chao e o aviao. Desenhadas ANTES das entidades, logo
// o aviao passa por cima delas.
void drawLowClouds(Renderer &r, const World &w);

// Tanques, balas, bombas, explosoes e o jogador, mais os contornos de hitbox
// quando a tecla H esta ligada.
void drawEntities(Renderer &r, const World &w);

// Nuvens altas, desenhadas DEPOIS das entidades: passam por cima do aviao.
void drawCloudLayer(Renderer &r, const World &w);
