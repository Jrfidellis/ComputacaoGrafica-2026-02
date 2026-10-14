#pragma once

#include "core/types.h"

struct Renderer;
struct World;

// Radar quadrado no canto inferior esquerdo, estilo Ace Combat: mostra o mapa
// inteiro de uma vez, pontos vermelhos para os inimigos e uma seta para o
// jogador que gira conforme o nariz do aviao.
//
// O mapeamento mundo -> radar e feito na CPU, e nao com uma segunda matriz de
// view. A versao com matriz seria elegante (o mesmo objeto do mundo desenhado
// por outra view), mas a escala dessa matriz encolheria tambem o PONTO: um
// tanque de 30 unidades viraria um ponto de 1,5 pixel. Compensar isso exigiria
// embutir o inverso da escala em cada matriz model, o que e confuso de ler e
// pior de explicar. Tres linhas de conta resolvem melhor.
void drawRadar(Renderer &r, const World &w);

// Converte uma posicao de mundo na posicao correspondente dentro do painel.
Vec2 worldToRadar(const World &w, Vec2 mundo, Vec2 cantoRadar, float tamanhoRadar);
