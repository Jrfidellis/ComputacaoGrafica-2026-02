#include "game/entities.h"

#include "game/flight.h"

#include <cmath>

Entity makeTank(Vec2 pos, int health)
{
	Entity e;
	e.kind = EntityKind::GroundEnemy;
	e.team = Team::Enemy;
	e.pos = pos;
	e.health = health;
	e.sprite = SPR_TANK_HULL;
	e.halfW = 12.0f;
	e.halfH = 12.0f;
	e.angle = 0.0f;
	e.turretAngle = 0.0f;
	return e;
}

Entity makeBullet(Vec2 pos, float angle, float speed, float vida, Team team)
{
	Entity e;
	e.kind = EntityKind::Bullet;
	e.team = team;
	e.pos = pos;
	e.angle = angle;
	e.speed = speed;
	e.vel = forwardFromAngle(angle) * speed;
	e.timer = vida;
	e.sprite = SPR_BULLET;
	e.halfW = 3.0f;
	e.halfH = 5.0f;
	e.health = 1;
	return e;
}

Entity makeBomb(Vec2 pos, float tempoQueda)
{
	Entity e;
	e.kind = EntityKind::Bomb;
	e.team = Team::Player;
	e.pos = pos;
	e.timer = tempoQueda;
	e.sprite = SPR_BOMB;
	e.halfW = 6.0f;
	e.halfH = 6.0f;
	e.scale = 1.0f;
	return e;
}

Entity makeExplosion(Vec2 pos, float duracao)
{
	Entity e;
	e.kind = EntityKind::Explosion;
	e.team = Team::Player;
	e.pos = pos;
	e.timer = duracao;
	e.sprite = SPR_EXPLOSION;
	e.halfW = 0.0f; // explosao nao colide: o dano ja foi aplicado na detonacao
	e.halfH = 0.0f;
	// timer corre de "duracao" ate 0, e e ele que controla QUAL QUADRO da
	// animacao de sprite aparece. Ver drawEntities em render/layers.cpp.
	return e;
}

void integrateEntities(std::vector<Entity> &lista, const Config &cfg,
					   float dt, float mundoTam)
{
	for (Entity &e : lista)
	{
		if (!e.alive)
			continue;

		e.pos += e.vel * dt;

		if (e.cooldown > 0.0f)
			e.cooldown -= dt;

		switch (e.kind)
		{
		case EntityKind::Bullet:
			// Bala morre por tempo de vida ou ao sair do mundo
			e.timer -= dt;
			if (e.timer <= 0.0f)
				e.alive = false;
			if (e.pos.x < 0.0f || e.pos.y < 0.0f || e.pos.x > mundoTam || e.pos.y > mundoTam)
				e.alive = false;
			break;

		case EntityKind::Bomb:
			// A bomba encolhe enquanto cai. E o unico lugar do jogo onde o S
			// da matriz model = T * R * S e animado.
			e.timer -= dt;
			e.scale = 0.35f + 0.65f * (e.timer > 0.0f ? e.timer : 0.0f);
			break;

		case EntityKind::Explosion:
			e.timer -= dt;
			if (e.timer <= 0.0f)
				e.alive = false;
			break;

		default:
			break;
		}
	}
}

void removeDead(std::vector<Entity> &lista)
{
	size_t i = 0;
	while (i < lista.size())
	{
		if (lista[i].alive)
		{
			i++;
		}
		else
		{
			lista[i] = lista.back();
			lista.pop_back();
			// Nao incrementa i: o elemento que veio do fim ainda nao foi visto
		}
	}
}
