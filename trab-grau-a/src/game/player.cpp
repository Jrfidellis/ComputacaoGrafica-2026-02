#include "game/player.h"

#include "core/config.h"
#include "core/types.h"
#include "game/entities.h"
#include "game/flight.h"
#include "game/tilemap.h"

#include <algorithm>

void updatePlayer(Player &jogador, const InputState &input, const Config &cfg,
				  const TileMap &mapa, float dt)
{
	if (!jogador.alive)
		return;

	// Invencibilidade temporaria depois de levar dano
	if (jogador.invuln > 0.0f)
		jogador.invuln -= dt;

	if (jogador.gunCooldown > 0.0f)
		jogador.gunCooldown -= dt;

	// Giro do nariz. Positivo e horario na tela, ver a convencao em flight.h.
	if (input.turnLeft)
		jogador.angle -= cfg.turnRate * dt;
	if (input.turnRight)
		jogador.angle += cfg.turnRate * dt;
	jogador.angle = wrapAngle(jogador.angle);

	// Potencia. O aviao NUNCA para: a velocidade so varia entre min e max.
	// E o que diferencia o modelo de voo de um movimento livre em 8 direcoes.
	if (input.throttleUp)
		jogador.speed += cfg.accel * dt;
	if (input.throttleDown)
		jogador.speed -= cfg.accel * dt;
	jogador.speed = std::min(std::max(jogador.speed, cfg.speedMin), cfg.speedMax);

	// Avanca sempre para a frente
	jogador.pos = stepAirplane(jogador.pos, jogador.angle, jogador.speed, dt);

	// Trava nos limites do mundo, com uma margem para o aviao nao entrar
	// inteiro na faixa de agua da borda.
	float lado = worldSize(mapa);
	float margem = mapa.borda * mapa.tileSize * 0.5f;
	jogador.pos.x = std::min(std::max(jogador.pos.x, margem), lado - margem);
	jogador.pos.y = std::min(std::max(jogador.pos.y, margem), lado - margem);
}
