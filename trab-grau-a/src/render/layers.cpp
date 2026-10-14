#include "render/layers.h"

#include "game/world.h"
#include "render/renderer.h"

#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

// Matriz de view de uma camada: deslocamento da camera multiplicado pelo fator
// de parallax daquela camada.
static glm::mat4 viewDaCamada(const World &w, float fator)
{
	Vec2 cam = w.camera.topLeft * fator;
	return glm::translate(glm::mat4(1.0f), glm::vec3(-cam.x, -cam.y, 0.0f));
}

void drawTerrain(Renderer &r, const World &w)
{
	setView(r, viewDaCamada(w, 1.0f));

	const TileMap &m = w.mapa;
	float ts = m.tileSize;
	Vec2 cam = w.camera.topLeft;

	// CULLING: so os tiles dentro da janela sao desenhados.
	// Sem isto seriam 96 x 96 = 9216 desenhos por frame. Com isto sao cerca
	// de 26 x 20 = 520, o que custa menos de 2 ms num Mac com chip M.
	//
	// Cada tile e um desenho com sua propria matriz model. Daria para juntar
	// o mapa inteiro num VBO so e ser mais rapido, mas isso esconderia
	// exatamente o conceito que o trabalho pede para demonstrar.
	int c0 = std::max(0, (int)std::floor(cam.x / ts));
	int c1 = std::min(m.largura - 1, (int)std::ceil((cam.x + r.telaW) / ts));
	int l0 = std::max(0, (int)std::floor(cam.y / ts));
	int l1 = std::min(m.altura - 1, (int)std::ceil((cam.y + r.telaH) / ts));

	for (int l = l0; l <= l1; l++)
	{
		for (int c = c0; c <= c1; c++)
		{
			Vec2 pos((c + 0.5f) * ts, (l + 0.5f) * ts);
			// +1 no tamanho evita uma linha fina de fundo entre tiles vizinhos
			drawTile(r, AtlasId::Tiles, tileAt(m, c, l), pos, 0.0f, ts + 1.0f, ts + 1.0f);
			r.tilesDesenhados++;
		}
	}
}

void drawEntities(Renderer &r, const World &w)
{
	setView(r, viewDaCamada(w, 1.0f));

	// Inimigos primeiro, para o aviao e os tiros ficarem por cima
	for (const Entity &e : w.entidades)
	{
		if (!e.alive || e.kind != EntityKind::GroundEnemy)
			continue;

		// O tanque e desenhado em DUAS partes com duas matrizes model
		// diferentes: o casco parado e a torre girando para mirar o jogador.
		drawSprite(r, SPR_TANK_HULL, e.pos, e.angle);
		drawSprite(r, SPR_TANK_TURRET, e.pos, e.turretAngle);
	}

	for (const Entity &e : w.entidades)
	{
		if (!e.alive)
			continue;

		switch (e.kind)
		{
		case EntityKind::Bullet:
		{
			glm::vec4 cor = (e.team == Team::Player)
								? glm::vec4(1.0f, 0.95f, 0.45f, 1.0f)
								: glm::vec4(1.0f, 0.42f, 0.35f, 1.0f);
			const SpriteDef &def = SPRITES[SPR_BULLET];
			drawSolid(r, e.pos, def.sizeW, def.sizeH, cor, e.angle);
			break;
		}

		case EntityKind::Bomb:
			// A escala vem do estado da entidade e encolhe enquanto a bomba
			// cai. E o unico lugar onde o S da matriz T * R * S e animado.
			drawSprite(r, SPR_BOMB, e.pos, e.angle, e.scale);
			break;

		case EntityKind::Explosion:
		{
			// ANIMACAO POR SPRITESHEET, a unica do jogo.
			//
			// e.timer corre de explosionTime ate 0, entao "progresso" e o
			// quanto da explosao JA passou, de 0 a 1. Esse mesmo numero faz
			// tres coisas ao mesmo tempo:
			//
			//   1. escolhe o quadro: tiles 4, 5 e 6 da folha, nesta ordem
			//   2. aumenta a escala, o clarao cresce
			//   3. reduz o alpha, o clarao apaga
			//
			// Como o progresso vem do temporizador da PROPRIA entidade, cada
			// explosao toca a sua animacao do comeco. Se o quadro viesse de
			// um relogio global, todas piscariam juntas.
			float restante = e.timer > 0.0f ? e.timer : 0.0f;
			float progresso = 1.0f - restante / w.cfg.explosionTime;
			if (progresso < 0.0f)
				progresso = 0.0f;
			if (progresso > 1.0f)
				progresso = 1.0f;

			float escala = 0.7f + progresso * 1.4f;
			float alpha = 1.0f - progresso * progresso; // apaga no fim

			// Clarao macio por baixo, so para a explosao iluminar o terreno
			drawTextured(r, r.cloud, e.pos, 90.0f * escala, 90.0f * escala,
						 glm::vec4(1.0f, 0.65f, 0.15f, 0.45f * alpha));

			drawSprite(r, SPR_EXPLOSION, e.pos, 0.0f, escala,
					   glm::vec4(1.0f, 1.0f, 1.0f, alpha), progresso);
			break;
		}

		default:
			break;
		}
	}

	// O jogador por ultimo, sempre visivel por cima do resto.
	if (w.jogador.alive)
	{
		// Pisca enquanto esta invencivel apos levar dano
		bool piscar = w.jogador.invuln > 0.0f && std::fmod(w.jogador.invuln, 0.2f) > 0.1f;
		if (!piscar)
			drawSprite(r, SPR_PLAYER, w.jogador.pos, w.jogador.angle);
	}

	// Debug de hitbox na tecla H. Vale muito mais do que as 20 linhas que
	// custa: e a prova visual de que a colisao usa as caixas que diz usar.
	if (w.mostrarHitboxes)
	{
		const glm::vec4 verde(0.3f, 1.0f, 0.4f, 0.9f);
		const glm::vec4 vermelho(1.0f, 0.35f, 0.35f, 0.9f);
		const glm::vec4 amarelo(1.0f, 0.9f, 0.3f, 0.9f);

		for (const Entity &e : w.entidades)
		{
			if (!e.alive || e.halfW <= 0.0f)
				continue;
			const glm::vec4 &cor = (e.kind == EntityKind::Bullet) ? amarelo : vermelho;
			drawOutline(r, e.pos, e.halfW, e.halfH, cor);
		}

		if (w.jogador.alive)
			drawOutline(r, w.jogador.pos, w.jogador.halfW, w.jogador.halfH, verde);
	}
}

// Uma nuvem sao tres manchas macias sobrepostas, usando a textura radial
// gerada em codigo. O pacote da Kenney nao tem sprite de nuvem, e um quad
// solido translucido apareceria como um retangulo obvio na tela.
//
// Esta camada tambem e a demonstracao mais visivel do alpha blending.
static void desenhaNuvens(Renderer &r, const std::vector<Vec2> &lista,
						  float tamBase, float alpha)
{
	const glm::vec4 cor(1.0f, 1.0f, 1.0f, alpha);
	for (size_t i = 0; i < lista.size(); i++)
	{
		Vec2 p = lista[i];
		float tam = tamBase + (float)(i % 7) * 30.0f;
		drawTextured(r, r.cloud, p, tam, tam * 0.72f, cor);
		drawTextured(r, r.cloud, p + Vec2(tam * 0.30f, tam * 0.10f), tam * 0.80f, tam * 0.58f, cor);
		drawTextured(r, r.cloud, p + Vec2(-tam * 0.28f, tam * 0.08f), tam * 0.70f, tam * 0.52f, cor);
	}
}

void drawLowClouds(Renderer &r, const World &w)
{
	// Fator 1.15: so um pouco mais rapido que o chao, porque estas nuvens
	// estao logo acima dele. Desenhadas ANTES do aviao, que passa por cima.
	setView(r, viewDaCamada(w, w.cfg.parallaxCloudsLow));
	desenhaNuvens(r, w.nuvensBaixas, 110.0f, 0.13f);
}

void drawCloudLayer(Renderer &r, const World &w)
{
	// Fator 1.60: bem mais rapido, porque estas nuvens estao perto da camera.
	// Desenhadas DEPOIS do aviao, entao passam por cima dele.
	setView(r, viewDaCamada(w, w.cfg.parallaxCloudsHigh));
	desenhaNuvens(r, w.nuvensAltas, 150.0f, 0.17f);
}
