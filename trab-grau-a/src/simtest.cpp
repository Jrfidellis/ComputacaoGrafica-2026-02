// Teste de simulacao SEM JANELA.
//
// Este arquivo so compila porque game_core nao depende de OpenGL: nenhuma
// linha aqui toca a GLFW ou a GLAD, e mesmo assim o jogo inteiro (voo, tiro,
// colisao, bombas, camera) roda e pode ser verificado automaticamente.
//
// E a demonstracao mais direta de que a fronteira entre regras de jogo e
// renderizacao e real, e nao apenas uma convencao de nome de pasta.
//
// Rodar:  ./build/simtest     (sai com codigo 1 se algum caso falhar)

#include "core/sprites.h"
#include "game/flight.h"
#include "game/world.h"

#include <cmath>
#include <cstdio>

static int falhas = 0;

static void checa(const char *nome, bool ok, const char *detalhe)
{
	std::printf("  [%s] %-42s %s\n", ok ? "PASS" : "FAIL", nome, detalhe);
	if (!ok)
		falhas++;
}

// Avanca a simulacao por um numero de passos fixos de 1/60 s.
static void avanca(World &w, const InputState &in, int passos)
{
	for (int i = 0; i < passos; i++)
		updateWorld(w, in, 1.0f / 60.0f);
}

// Posicao do primeiro tanque vivo. Devolve false se nao houver nenhum.
static bool primeiroTanque(const World &w, Vec2 &destino)
{
	for (const Entity &e : w.entidades)
	{
		if (e.alive && e.kind == EntityKind::GroundEnemy)
		{
			destino = e.pos;
			return true;
		}
	}
	return false;
}

// Um tanque no canto oposto do mapa, longe de tudo.
//
// Existe por um motivo sutil: updateWorld declara VITORIA quando nao sobra
// nenhum inimigo, e a partir dai retorna logo no inicio. Um mundo de teste
// sem inimigo nenhum congela no primeiro frame e todos os casos seguintes
// medem zero. Este boneco mantem a simulacao rodando sem interferir.
static void mantemVivo(World &w)
{
	Vec2 canto(worldSize(w.mapa) - 120.0f, worldSize(w.mapa) - 120.0f);
	w.entidades.push_back(makeTank(canto, 9999));
}

// Devolve o jogador ao estado inicial entre secoes, para a morte num caso
// nao contaminar o proximo.
static void revive(World &w, const Config &cfg)
{
	w.jogador.health = cfg.playerHealth;
	w.jogador.alive = true;
	w.jogador.invuln = 0.0f;
	w.gameOver = false;
	w.venceu = false;
}

static int contaBombas(const World &w)
{
	int n = 0;
	for (const Entity &e : w.entidades)
		if (e.alive && e.kind == EntityKind::Bomb)
			n++;
	return n;
}

int main()
{
	Config cfg;
	cfg.enemyCount = 12;
	// Os casos abaixo medem UMA coisa de cada vez num mundo controlado.

	World w;
	initWorld(w, cfg);

	char buf[160];
	const InputState vazio;

	std::printf("simtest: mundo de %.0f unidades, %d tanques, %d bombas\n\n",
				worldSize(w.mapa), w.inimigosVivos, w.jogador.bombs);

	// ---------------------------------------------------------------------
	std::printf("MODELO DE VOO\n");

	// O aviao esta sempre em movimento: sem nenhuma tecla ele avanca mesmo assim
	Vec2 p0 = w.jogador.pos;
	avanca(w, vazio, 60);
	float andou = glm::length(w.jogador.pos - p0);
	std::snprintf(buf, sizeof(buf), "andou %.1f, esperado ~%.0f", andou, cfg.speedStart);
	checa("voa para frente sem nenhuma tecla", std::fabs(andou - cfg.speedStart) < 5.0f, buf);

	// Girar: segurar a tecla por 1 s deve girar turnRate radianos
	float a0 = w.jogador.angle;
	InputState direita;
	direita.turnRight = true;
	avanca(w, direita, 60);
	float girou = std::fabs(wrapAngle(w.jogador.angle - a0));
	std::snprintf(buf, sizeof(buf), "%.2f rad, esperado ~%.2f", girou, cfg.turnRate);
	checa("girar 1 s gira turn_rate radianos", std::fabs(girou - cfg.turnRate) < 0.1f, buf);

	// A potencia e limitada: segurar acelerar nunca passa de speed_max
	InputState acelera;
	acelera.throttleUp = true;
	avanca(w, acelera, 600);
	std::snprintf(buf, sizeof(buf), "velocidade %.0f, teto %.0f", w.jogador.speed, cfg.speedMax);
	checa("potencia satura em speed_max", w.jogador.speed <= cfg.speedMax + 0.01f, buf);

	// ---------------------------------------------------------------------
	std::printf("\nMETRALHADORA\n");
	revive(w, cfg);

	// Recentraliza antes de testar: o teste de potencia acima deixou o aviao
	// preso na borda do mapa, e dali as balas saem do mundo e morrem no
	// primeiro frame, o que e o comportamento correto e nao o que se quer
	// medir aqui.
	w.jogador.pos = Vec2(worldSize(w.mapa) * 0.5f, worldSize(w.mapa) * 0.5f);
	w.jogador.angle = 0.0f;

	InputState tiro;
	tiro.fireGun = true;
	avanca(w, tiro, 30);
	int balas = 0;
	for (const Entity &e : w.entidades)
		if (e.alive && e.kind == EntityKind::Bullet)
			balas++;
	std::snprintf(buf, sizeof(buf), "%d balas vivas", balas);
	checa("segurar o tiro cria balas", balas > 0, buf);

	// Atirar de cima para baixo num tanque, a distancia
	Vec2 alvo;
	if (primeiroTanque(w, alvo))
	{
		int antes = w.inimigosVivos;
		int scoreAntes = w.score;
		w.jogador.pos = alvo - Vec2(0.0f, 120.0f); // 120 unidades acima
		w.jogador.angle = 3.14159265f;			   // nariz para baixo
		avanca(w, tiro, 120);
		std::snprintf(buf, sizeof(buf), "%d -> %d tanques, score %d -> %d",
					  antes, w.inimigosVivos, scoreAntes, w.score);
		checa("bala destroi tanque e pontua", w.inimigosVivos < antes && w.score > scoreAntes, buf);
	}

	// ---------------------------------------------------------------------
	std::printf("\nVOAR POR CIMA DE TANQUE NAO DA DANO\n");
	revive(w, cfg);

	// O aviao voa ACIMA dos veiculos de solo. Sobrepor as caixas vistas de
	// cima significa apenas estar passando por cima do alvo, que e justamente
	// o que o jogador precisa fazer para bombardear.
	if (primeiroTanque(w, alvo))
	{
		int vidaAntes = w.jogador.health;
		w.jogador.pos = alvo;		 // exatamente em cima
		w.jogador.speed = cfg.speedMin;
		w.jogador.invuln = 0.0f;	 // sem invencibilidade mascarando o teste

		// Mantem o aviao preso sobre o tanque por 1 segundo inteiro
		for (int i = 0; i < 60; i++)
		{
			w.jogador.pos = alvo;
			updateWorld(w, vazio, 1.0f / 60.0f);
		}

		std::snprintf(buf, sizeof(buf), "vida %d -> %d", vidaAntes, w.jogador.health);
		checa("1 s parado sobre o tanque nao tira vida", w.jogador.health == vidaAntes, buf);
	}

	// O tiro do tanque continua machucando: a unica fonte de dano agora
	{
		int vidaAntes = w.jogador.health;
		w.jogador.invuln = 0.0f;
		w.entidades.push_back(makeBullet(w.jogador.pos - Vec2(0.0f, 30.0f),
										 3.14159265f, 200.0f, 2.0f, Team::Enemy));
		avanca(w, vazio, 30);
		std::snprintf(buf, sizeof(buf), "vida %d -> %d", vidaAntes, w.jogador.health);
		checa("bala inimiga ainda tira vida", w.jogador.health < vidaAntes, buf);
	}

	// ---------------------------------------------------------------------
	std::printf("\nBOMBA CAI ONDE O AVIAO ESTAVA\n");
	revive(w, cfg);

	{
		w.jogador.pos = Vec2(1000.0f, 1000.0f);
		w.jogador.angle = 0.0f;
		w.jogador.speed = cfg.speedMax; // velocidade alta: se houver deriva, aparece
		Vec2 lancamento = w.jogador.pos;

		int bombasAntes = w.jogador.bombs;
		InputState solta;
		solta.dropBomb = true;
		updateWorld(w, solta, 1.0f / 60.0f);

		std::snprintf(buf, sizeof(buf), "estoque %d -> %d, %d bomba(s) no ar",
					  bombasAntes, w.jogador.bombs, contaBombas(w));
		checa("soltar consome uma bomba do estoque",
			  w.jogador.bombs == bombasAntes - 1 && contaBombas(w) == 1, buf);

		// Enquanto a bomba cai, o aviao se afasta. A bomba nao pode acompanhar.
		float maiorDesvio = 0.0f;
		for (int i = 0; i < 30; i++)
		{
			updateWorld(w, vazio, 1.0f / 60.0f);
			for (const Entity &e : w.entidades)
			{
				if (e.alive && e.kind == EntityKind::Bomb)
				{
					float d = glm::length(e.pos - lancamento);
					if (d > maiorDesvio)
						maiorDesvio = d;
				}
			}
		}
		float afastou = glm::length(w.jogador.pos - lancamento);
		std::snprintf(buf, sizeof(buf), "desvio %.3f enquanto o aviao andou %.0f",
					  maiorDesvio, afastou);
		checa("bomba fica parada no ponto do lancamento", maiorDesvio < 0.01f, buf);
	}

	// A bomba destroi tanques num raio ao redor do ponto onde foi solta
	if (primeiroTanque(w, alvo))
	{
		int antes = w.inimigosVivos;
		int bombasAntes = w.jogador.bombs;
		w.jogador.pos = alvo;
		w.jogador.speed = cfg.speedMax;

		InputState solta;
		solta.dropBomb = true;
		updateWorld(w, solta, 1.0f / 60.0f);

		// Afasta o aviao para provar que quem causa o dano e o ponto de
		// lancamento, e nao a posicao atual do aviao no momento da explosao
		w.jogador.pos = alvo + Vec2(900.0f, 900.0f);
		avanca(w, vazio, 80);

		std::snprintf(buf, sizeof(buf), "%d -> %d tanques, estoque %d -> %d",
					  antes, w.inimigosVivos, bombasAntes, w.jogador.bombs);
		checa("bomba explode no ponto do lancamento",
			  w.inimigosVivos < antes && w.jogador.bombs == bombasAntes - 1, buf);
	}

	// Estoque esgotado: soltar nao faz mais nada
	{
		w.jogador.bombs = 0;
		InputState solta;
		solta.dropBomb = true;
		int noAr = contaBombas(w);
		updateWorld(w, solta, 1.0f / 60.0f);
		std::snprintf(buf, sizeof(buf), "estoque 0, bombas no ar %d -> %d", noAr, contaBombas(w));
		checa("estoque zerado nao solta bomba", contaBombas(w) <= noAr, buf);
		w.jogador.bombs = cfg.bombCount;
	}


	// ---------------------------------------------------------------------
	// ---------------------------------------------------------------------
	std::printf("\nANIMACAO DE SPRITE DA EXPLOSAO\n");

	{
		// A explosao e a unica animacao do jogo: tres tiles CONSECUTIVOS da
		// folha, 4, 5 e 6. spriteFrameTile divide o progresso de 0 a 1 em
		// faixas iguais e devolve o tile daquele quadro.
		const SpriteDef &def = SPRITES[SPR_EXPLOSION];

		std::snprintf(buf, sizeof(buf), "primeiro tile %d, %d quadros", def.tile, def.frames);
		checa("a explosao tem 3 quadros a partir do tile 4",
			  def.tile == 4 && def.frames == 3, buf);

		int q0 = spriteFrameTile(def, 0.0f);
		int q1 = spriteFrameTile(def, 0.5f);
		int q2 = spriteFrameTile(def, 0.99f);
		std::snprintf(buf, sizeof(buf), "0.0 -> %d, 0.5 -> %d, 0.99 -> %d", q0, q1, q2);
		checa("o progresso percorre os tres quadros em ordem",
			  q0 == 4 && q1 == 5 && q2 == 6, buf);

		// A ponta exata 1.0 nao pode estourar para um tile que nao existe
		int qFim = spriteFrameTile(def, 1.0f);
		std::snprintf(buf, sizeof(buf), "progresso 1.0 -> tile %d", qFim);
		checa("progresso 1.0 fica no ultimo quadro", qFim == 6, buf);

		// Um sprite de quadro unico ignora o progresso
		int qTanque = spriteFrameTile(SPRITES[SPR_TANK_HULL], 0.8f);
		std::snprintf(buf, sizeof(buf), "casco do tanque -> tile %d", qTanque);
		checa("sprite de quadro unico ignora o progresso",
			  qTanque == SPRITES[SPR_TANK_HULL].tile, buf);
	}

	{
		// A explosao precisa existir tempo suficiente para a animacao ser
		// vista: se ela morresse em um frame, os tres quadros nunca
		// apareceriam na tela.
		World t;
		Config c = cfg;
		c.enemyCount = 0;
		initWorld(t, c);
		mantemVivo(t);
		t.jogador.pos = Vec2(1000.0f, 1000.0f);

		InputState solta;
		solta.dropBomb = true;
		updateWorld(t, solta, 1.0f / 60.0f);

		// Espera a bomba explodir
		for (int i = 0; i < 120; i++)
		{
			updateWorld(t, vazio, 1.0f / 60.0f);
			bool temExplosao = false;
			for (const Entity &e : t.entidades)
				if (e.alive && e.kind == EntityKind::Explosion)
					temExplosao = true;
			if (temExplosao)
				break;
		}

		// Conta quantos frames a explosao sobrevive
		int frames = 0;
		for (int i = 0; i < 200; i++)
		{
			bool viva = false;
			for (const Entity &e : t.entidades)
				if (e.alive && e.kind == EntityKind::Explosion)
					viva = true;
			if (!viva)
				break;
			frames++;
			updateWorld(t, vazio, 1.0f / 60.0f);
		}

		std::snprintf(buf, sizeof(buf), "explosao dura %d frames a 60 fps", frames);
		checa("a explosao dura mais que os 3 quadros", frames >= 3 * 3, buf);
	}

	// ---------------------------------------------------------------------
	std::printf("\nREGRESSAO CORRIGIDA\n");

	{
		// Antes o arrasto era um fator por FRAME e nao por segundo, entao o
		// alcance dependia da taxa de quadros. O flare saiu do jogo, mas a
		// bomba usa a mesma integracao e a licao vale: tudo que decai com o
		// tempo precisa de dt.
		auto distanciaDaBomba = [&](float dt) {
			World t;
			Config c = cfg;
			c.enemyCount = 0;
			initWorld(t, c);
			mantemVivo(t);
			t.jogador.pos = Vec2(1000.0f, 1000.0f);
			t.jogador.angle = 0.0f;
			t.jogador.speed = c.speedMax;
			Vec2 inicio = t.jogador.pos;
			int passos = (int)(0.5f / dt);
			for (int i = 0; i < passos; i++)
				updateWorld(t, vazio, dt);
			return glm::length(t.jogador.pos - inicio);
		};
		float a60 = distanciaDaBomba(1.0f / 60.0f);
		float a30 = distanciaDaBomba(1.0f / 30.0f);
		float dif = (a60 > 0.1f) ? 100.0f * std::fabs(a30 - a60) / a60 : 999.0f;
		std::snprintf(buf, sizeof(buf), "60fps %.1f u, 30fps %.1f u, diferenca %.1f%%",
					  a60, a30, dif);
		checa("o voo independe da taxa de quadros", dif < 5.0f, buf);
	}

	// ---------------------------------------------------------------------
	std::printf("\nCAMERA\n");

	revive(w, cfg);
	w.jogador.pos = Vec2(10.0f, 10.0f);
	updateWorld(w, vazio, 1.0f / 60.0f);
	std::snprintf(buf, sizeof(buf), "topLeft = (%.0f, %.0f)", w.camera.topLeft.x, w.camera.topLeft.y);
	checa("camera trava no canto do mapa",
		  w.camera.topLeft.x == 0.0f && w.camera.topLeft.y == 0.0f, buf);

	float lado = worldSize(w.mapa);
	revive(w, cfg);
	w.jogador.pos = Vec2(lado - 10.0f, lado - 10.0f);
	updateWorld(w, vazio, 1.0f / 60.0f);
	float maxX = lado - cfg.windowWidth;
	std::snprintf(buf, sizeof(buf), "topLeft.x = %.0f, maximo %.0f", w.camera.topLeft.x, maxX);
	checa("camera trava na borda oposta", std::fabs(w.camera.topLeft.x - maxX) < 0.5f, buf);

	// ---------------------------------------------------------------------
	std::printf("\n%s: %d falha(s)\n", falhas == 0 ? "TUDO OK" : "ATENCAO", falhas);
	return falhas == 0 ? 0 : 1;
}
