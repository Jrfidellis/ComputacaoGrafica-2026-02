#pragma once

#include "core/config.h"
#include "core/types.h"
#include "game/camera2d.h"
#include "game/entities.h"
#include "game/tilemap.h"

#include <vector>

// Estado completo da simulacao. O renderizador recebe isto como const World&
// e nao tem como alterar nada: a fronteira e verificada pelo compilador.
struct World
{
	Config cfg;
	TileMap mapa;

	Player jogador;
	std::vector<Entity> entidades; // inimigos, balas, bombas, explosoes

	Camera2D camera;

	int score = 0;
	int inimigosVivos = 0;
	float tempoDeJogo = 0.0f;
	bool gameOver = false;
	bool venceu = false;

	// Posicoes das nuvens da camada de parallax. Ficam aqui porque sao estado
	// do mundo, nao do renderizador: a mesma lista alimenta qualquer desenho.
	std::vector<Vec2> nuvensBaixas;
	std::vector<Vec2> nuvensAltas;

	// Liga os contornos de hitbox na tecla H. E estado de jogo, nao de desenho,
	// porque o input que alterna isso chega pelo InputState.
	bool mostrarHitboxes = false;

	// Contadores so para diagnostico no console
	int tilesDesenhados = 0;
};

// Monta o mundo: gera o mapa, posiciona o jogador no centro, espalha tanques
// fora da agua e sorteia as nuvens.
void initWorld(World &w, const Config &cfg);

// Um passo de simulacao. NENHUMA chamada de OpenGL acontece aqui, nem em nada
// que esta funcao chame. Essa e a regra que separa jogo de renderizacao.
void updateWorld(World &w, const InputState &input, float dt);
