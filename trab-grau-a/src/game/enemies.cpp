#include "game/enemies.h"

#include "core/config.h"
#include "game/collision.h"
#include "game/entities.h"
#include "game/flight.h"

// COMPORTAMENTO DO TANQUE INIMIGO
//
// O tanque nao anda. O que se move e a TORRE, que gira em direcao ao jogador
// a uma taxa limitada e atira quando esta apontada o suficiente.
//
// Isso e proposital e rende uma demonstracao direta do requisito de matriz de
// model: o tanque e desenhado em DUAS partes, o casco parado e a torre
// girando, cada uma com a sua propria matriz, usando o mesmo quad e a mesma
// textura. Um objeto, duas transformacoes.

void updateEnemies(std::vector<Entity> &lista, const Player &jogador,
				   const Config &cfg, float dt)
{
	float alcance2 = cfg.enemyRange * cfg.enemyRange;

	// Os tiros criados neste frame ficam de lado: dar push_back durante o laco
	// pode realocar o vetor e invalidar a referencia que o laco esta usando.
	std::vector<Entity> novosTiros;

	for (Entity &e : lista)
	{
		if (!e.alive || e.kind != EntityKind::GroundEnemy)
			continue;

		bool naMira = jogador.alive && dist2(e.pos, jogador.pos) <= alcance2;
		if (!naMira)
			continue;

		// A torre persegue o jogador, mas com taxa de giro limitada: ela nao
		// gruda instantaneamente, entao da para passar voando depressa e
		// escapar da mira.
		float desejado = headingTo(e.pos, jogador.pos);
		e.turretAngle = turnToward(e.turretAngle, desejado, cfg.turretTurnRate * dt);

		if (e.cooldown > 0.0f)
			continue;

		// So atira se a torre ja estiver apontada o suficiente. Sem esta
		// checagem o tanque acerta enquanto ainda esta girando, o que parece
		// injusto porque o jogador ve o cano apontando para outro lado.
		float erro = wrapAngle(desejado - e.turretAngle);
		if (erro < 0.25f && erro > -0.25f)
		{
			Vec2 boca = e.pos + forwardFromAngle(e.turretAngle) * 16.0f;
			novosTiros.push_back(makeBullet(boca, e.turretAngle,
											cfg.enemyBulletSpeed, 3.0f, Team::Enemy));
			e.cooldown = cfg.enemyFireCooldown;
		}
	}

	for (const Entity &t : novosTiros)
		lista.push_back(t);
}
