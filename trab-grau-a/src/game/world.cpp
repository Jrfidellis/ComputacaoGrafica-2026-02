#include "game/world.h"

#include "game/collision.h"
#include "game/enemies.h"
#include "game/flight.h"
#include "game/player.h"
#include "game/weapons.h"

#include <random>

void initWorld(World &w, const Config &cfg)
{
	w.cfg = cfg;

	generateMap(w.mapa, cfg.mapTiles, cfg.tileSize, cfg.borderTiles, cfg.seed);

	float lado = worldSize(w.mapa);

	w.jogador = Player();
	w.jogador.pos = Vec2(lado * 0.5f, lado * 0.5f);
	w.jogador.angle = 0.0f;
	w.jogador.speed = cfg.speedStart;
	w.jogador.health = cfg.playerHealth;
	w.jogador.bombs = cfg.bombCount;
	w.jogador.alive = true;

	w.entidades.clear();
	w.score = 0;
	w.tempoDeJogo = 0.0f;
	w.gameOver = false;
	w.venceu = false;

	std::mt19937 rng(cfg.seed + 7u);
	std::uniform_int_distribution<int> coluna(cfg.borderTiles + 1, cfg.mapTiles - cfg.borderTiles - 2);

	// Espalha tanques em terreno seco, longe do ponto inicial do jogador para
	// o jogo nao comecar com o aviao dentro de uma emboscada.
	int criados = 0;
	int tentativas = 0;
	while (criados < cfg.enemyCount && tentativas < cfg.enemyCount * 60)
	{
		tentativas++;

		int c = coluna(rng);
		int l = coluna(rng);
		if (isWater(w.mapa, c, l))
			continue;

		Vec2 pos((c + 0.5f) * cfg.tileSize, (l + 0.5f) * cfg.tileSize);
		if (dist2(pos, w.jogador.pos) < 420.0f * 420.0f)
			continue;

		w.entidades.push_back(makeTank(pos, cfg.enemyHealth));
		criados++;
	}
	w.inimigosVivos = criados;

	// Nuvens da camada de parallax, espalhadas sobre todo o mundo
	// Duas alturas de nuvem, cada uma com seu proprio fator de parallax.
	// Ao fazer uma curva da para ver as duas se separando uma da outra e do
	// chao, que e a demonstracao visivel do parallax.
	w.nuvensBaixas.clear();
	w.nuvensAltas.clear();
	std::uniform_real_distribution<float> pos01(0.0f, 1.0f);
	for (int i = 0; i < cfg.cloudCount; i++)
	{
		w.nuvensBaixas.push_back(Vec2(pos01(rng) * lado * 1.2f, pos01(rng) * lado * 1.2f));
		w.nuvensAltas.push_back(Vec2(pos01(rng) * lado * 1.7f, pos01(rng) * lado * 1.7f));
	}

	w.camera.topLeft = cameraFollow(w.jogador.pos,
									(float)cfg.windowWidth, (float)cfg.windowHeight,
									lado, lado);
}

// Colisao de todas as balas contra tudo que pode ser atingido.
static int resolverBalas(World &w)
{
	int pontos = 0;

	for (Entity &bala : w.entidades)
	{
		if (!bala.alive || bala.kind != EntityKind::Bullet)
			continue;

		AABB caixaBala = makeAABB(bala.pos, bala.halfW, bala.halfH);

		if (bala.team == Team::Player)
		{
			// Bala do jogador contra inimigos
			for (Entity &alvo : w.entidades)
			{
				if (!alvo.alive)
					continue;
				if (alvo.kind != EntityKind::GroundEnemy)
					continue;

				if (!aabbOverlap(caixaBala, makeAABB(alvo.pos, alvo.halfW, alvo.halfH)))
					continue;

				alvo.health -= w.cfg.bulletDamage;
				bala.alive = false;
				if (alvo.health <= 0)
				{
					alvo.alive = false;
					pontos += w.cfg.scorePerTank;
				}
				break;
			}
		}
		else
		{
			// Bala inimiga contra o jogador
			if (!w.jogador.alive || w.jogador.invuln > 0.0f)
				continue;

			AABB caixaJogador = makeAABB(w.jogador.pos, w.jogador.halfW, w.jogador.halfH);
			if (aabbOverlap(caixaBala, caixaJogador))
			{
				bala.alive = false;
				w.jogador.health--;
				w.jogador.invuln = 1.0f;
			}
		}
	}

	return pontos;
}

void updateWorld(World &w, const InputState &input, float dt)
{
	// Alternar hitboxes e reiniciar funcionam mesmo com o jogo acabado
	if (input.toggleHitboxes)
		w.mostrarHitboxes = !w.mostrarHitboxes;

	if (input.restart && w.gameOver)
	{
		Config guardada = w.cfg;
		initWorld(w, guardada);
		return;
	}

	if (w.gameOver)
		return;

	w.tempoDeJogo += dt;

	// Posicao do aviao no INICIO do frame, antes de o voo avancar. E ela que
	// marca onde a bomba cai: a tecla foi apertada com o jogador olhando o
	// quadro anterior, entao e esse ponto que ele espera acertar.
	Vec2 posAntesDoVoo = w.jogador.pos;

	// 1. Jogador
	updatePlayer(w.jogador, input, w.cfg, w.mapa, dt);

	// 2. Armas
	if (input.fireGun)
		fireGun(w.jogador, w.entidades, w.cfg);
	if (input.dropBomb)
		dropBomb(w.jogador, w.entidades, w.cfg, posAntesDoVoo);

	// 3. Inimigos
	updateEnemies(w.entidades, w.jogador, w.cfg, dt);

	// 4. Integracao: move tudo e vence temporizadores
	integrateEntities(w.entidades, w.cfg, dt, worldSize(w.mapa));

	// 5. Colisoes
	w.score += resolverBalas(w);
	w.score += detonateBombs(w.entidades, w.cfg);

	// 6. Limpeza. So agora, com nenhuma iteracao em andamento.
	removeDead(w.entidades);

	// 7. Contagem e condicoes de fim
	w.inimigosVivos = 0;
	for (const Entity &e : w.entidades)
		if (e.alive && e.kind == EntityKind::GroundEnemy)
			w.inimigosVivos++;

	if (w.jogador.health <= 0)
	{
		w.jogador.alive = false;
		w.gameOver = true;
		w.venceu = false;
	}
	else if (w.inimigosVivos == 0)
	{
		w.gameOver = true;
		w.venceu = true;
	}

	// 8. Camera segue o aviao e trava nas bordas
	float lado = worldSize(w.mapa);
	w.camera.topLeft = cameraFollow(w.jogador.pos,
									(float)w.cfg.windowWidth, (float)w.cfg.windowHeight,
									lado, lado);
}
