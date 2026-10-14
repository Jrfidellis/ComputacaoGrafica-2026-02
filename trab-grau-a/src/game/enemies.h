#pragma once

#include <vector>

struct Entity;
struct Player;
struct Config;

// Atualiza os inimigos: gira a torre dos tanques em direcao ao jogador e
// dispara quando ela esta apontada.
void updateEnemies(std::vector<Entity> &lista, const Player &jogador,
				   const Config &cfg, float dt);
