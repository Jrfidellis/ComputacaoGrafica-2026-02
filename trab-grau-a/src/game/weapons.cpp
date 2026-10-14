#include "game/weapons.h"

#include "core/config.h"
#include "game/collision.h"
#include "game/entities.h"
#include "game/flight.h"

bool fireGun(Player &jogador, std::vector<Entity> &lista, const Config &cfg)
{
	if (!jogador.alive || jogador.gunCooldown > 0.0f)
		return false;

	// A bala nasce um pouco a frente do nariz, senao ela colide com o proprio
	// aviao no mesmo frame em que foi criada.
	Vec2 frente = forwardFromAngle(jogador.angle);
	Vec2 boca = jogador.pos + frente * 22.0f;

	lista.push_back(makeBullet(boca, jogador.angle, cfg.bulletSpeed, cfg.bulletLife, Team::Player));
	jogador.gunCooldown = cfg.gunCooldown;
	return true;
}

bool dropBomb(Player &jogador, std::vector<Entity> &lista, const Config &cfg,
			  Vec2 posLancamento)
{
	if (!jogador.alive || jogador.bombs <= 0)
		return false;

	// A bomba fica EXATAMENTE onde o aviao estava no instante do lancamento e
	// cai parada ali ate explodir.
	//
	// A velocidade e zero de proposito. Fisicamente uma bomba solta de um
	// aviao em movimento manteria a inercia e cairia bem a frente, mas ai o
	// ponto de impacto passaria a depender da velocidade no momento do
	// lancamento, o que torna a mira imprevisivel. Com velocidade zero a regra
	// vira "sobrevoe o alvo e solte", que o jogador aprende de primeira.
	Entity b = makeBomb(posLancamento, cfg.bombFallTime);
	b.vel = Vec2(0.0f, 0.0f);
	lista.push_back(b);

	jogador.bombs--;
	return true;
}

int detonateBombs(std::vector<Entity> &lista, const Config &cfg)
{
	int pontos = 0;
	float raio2 = cfg.bombRadius * cfg.bombRadius;

	// Guarda onde explodir. Nao da para empurrar na lista durante a iteracao:
	// push_back pode realocar o vetor e invalidar a referencia do laco.
	std::vector<Vec2> estouros;

	for (Entity &e : lista)
	{
		if (!e.alive || e.kind != EntityKind::Bomb)
			continue;
		if (e.timer > 0.0f)
			continue;

		estouros.push_back(e.pos);
		e.alive = false;
	}

	for (Vec2 centro : estouros)
	{
		// Dano em area. Aqui o circulo e a forma certa, diferente do resto do
		// jogo que usa caixa: a explosao se espalha igual em todas as direcoes.
		for (Entity &alvo : lista)
		{
			if (!alvo.alive)
				continue;
			if (alvo.kind != EntityKind::GroundEnemy)
				continue;
			if (dist2(alvo.pos, centro) > raio2)
				continue;

			alvo.alive = false;
			pontos += cfg.scorePerTank;
		}
	}

	for (Vec2 centro : estouros)
		lista.push_back(makeExplosion(centro, cfg.explosionTime));

	return pontos;
}
