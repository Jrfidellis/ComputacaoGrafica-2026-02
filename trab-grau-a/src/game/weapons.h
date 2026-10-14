#pragma once

#include "core/types.h"

#include <vector>

struct Entity;
struct Player;
struct Config;

// Metralhadora: municao infinita, limitada apenas pelo tempo de recarga.
// Devolve true se um tiro saiu neste frame.
bool fireGun(Player &jogador, std::vector<Entity> &lista, const Config &cfg);

// Bomba: estoque limitado, comeca em cfg.bombCount.
//
// posLancamento e a posicao que o aviao ocupava no INICIO do frame, antes de
// updatePlayer avancar o voo. E ela, e nao a posicao ja avancada, que marca
// onde a bomba cai: a tecla foi apertada olhando o quadro anterior, entao e
// ali que o jogador espera o impacto. A diferenca e um frame de voo, ate 5
// unidades na velocidade maxima.
bool dropBomb(Player &jogador, std::vector<Entity> &lista, const Config &cfg,
			  Vec2 posLancamento);

// Processa as bombas cujo tempo acabou: cria a explosao e aplica dano em area.
// Devolve quantos pontos o jogador ganhou com as destruicoes.
int detonateBombs(std::vector<Entity> &lista, const Config &cfg);
