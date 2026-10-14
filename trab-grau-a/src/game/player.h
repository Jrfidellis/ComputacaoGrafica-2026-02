#pragma once

struct Player;
struct Config;
struct InputState;
struct TileMap;

// Aplica o modelo de voo ao jogador: gira o nariz, ajusta a potencia e avanca.
// Le o InputState preenchido pela plataforma; nao conhece a GLFW.
void updatePlayer(Player &jogador, const InputState &input, const Config &cfg,
				  const TileMap &mapa, float dt);
