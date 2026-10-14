#pragma once

#include "core/config.h"
#include "core/sprites.h"
#include "core/types.h"

#include <vector>

// Uma struct unica para tudo que existe no mundo, com uma tag de tipo.
//
// Tanques, balas, bombas e explosoes sao a MESMA struct, diferenciadas pelo
// campo kind. Em troca de alguns campos que nem todo tipo usa, a lista do
// mundo e um vetor so, e a integracao e a remocao sao escritas uma vez para
// todos. Adicionar um tipo novo custa um case a mais no switch.
struct Entity
{
	EntityKind kind = EntityKind::GroundEnemy;
	Team team = Team::Enemy;

	Vec2 pos = Vec2(0.0f);
	Vec2 vel = Vec2(0.0f);

	float angle = 0.0f;		  // radianos, 0 = nariz/frente para cima
	float turretAngle = 0.0f; // so o tanque usa
	float speed = 0.0f;

	int health = 1;
	float halfW = 8.0f; // metade da hitbox
	float halfH = 8.0f;

	int sprite = SPR_TANK;
	float scale = 1.0f; // multiplica o tamanho do sprite (bomba encolhe)

	float timer = 0.0f;	   // tempo de vida restante ou contagem regressiva
	float cooldown = 0.0f; // proximo tiro
	bool alive = true;
};

// O jogador NAO fica no vetor de entidades. Ele e um membro separado do World
// porque a remocao por swap-and-pop embaralha os indices, e um indice de
// jogador invalidado seria uma fonte infinita de bugs.
struct Player
{
	Vec2 pos = Vec2(0.0f);
	float angle = 0.0f;
	float speed = 160.0f;
	int health = 5;
	int bombs = 20;
	float gunCooldown = 0.0f;
	float invuln = 0.0f; // segundos de invencibilidade apos levar dano
	bool alive = true;

	float halfW = 11.0f; // hitbox menor que o sprite de 38, de proposito:
	float halfH = 11.0f; // perdoar o jogador e melhor do que ser preciso

};

// Fabricas de entidades. Mantem os valores iniciais num lugar so.
Entity makeTank(Vec2 pos, int health);
Entity makeBullet(Vec2 pos, float angle, float speed, float vida, Team team);
Entity makeBomb(Vec2 pos, float tempoQueda);
Entity makeExplosion(Vec2 pos, float duracao);

// Integra posicao, decrementa temporizadores e mata o que venceu o tempo.
// Recebe a Config porque o arrasto do flare e um valor ajustavel.
void integrateEntities(std::vector<Entity> &lista, const Config &cfg,
					   float dt, float mundoTam);

// Remove os mortos com swap-and-pop. So pode rodar no FIM do update, nunca
// durante uma iteracao sobre a lista.
void removeDead(std::vector<Entity> &lista);
