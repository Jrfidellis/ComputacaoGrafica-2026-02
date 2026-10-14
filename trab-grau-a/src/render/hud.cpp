#include "render/hud.h"

#include "game/world.h"
#include "render/renderer.h"

#include <algorithm>
#include <cmath>
#include <vector>

float drawNumber(Renderer &r, int valor, Vec2 posEsquerda, float alturaDigito)
{
	if (valor < 0)
		valor = 0;

	// Separa os digitos do menos significativo para o mais significativo
	std::vector<int> digitos;
	if (valor == 0)
	{
		digitos.push_back(0);
	}
	else
	{
		while (valor > 0)
		{
			digitos.push_back(valor % 10);
			valor /= 10;
		}
	}

	float largura = alturaDigito * 0.62f; // avanco entre digitos
	float x = posEsquerda.x;

	// Desenha do mais significativo para o menos, invertendo a ordem
	for (int i = (int)digitos.size() - 1; i >= 0; i--)
	{
		int tile = DIGIT_TILE[digitos[i]];
		drawTile(r, AtlasId::Tiles, tile,
				 Vec2(x + alturaDigito * 0.5f, posEsquerda.y),
				 0.0f, alturaDigito, alturaDigito);
		x += largura;
	}

	return x - posEsquerda.x;
}

// Barra de fundo mais barra de preenchimento, ambas quads solidos.
static void drawBar(Renderer &r, Vec2 canto, float largura, float altura,
					float fracao, const glm::vec4 &cor)
{
	fracao = std::min(std::max(fracao, 0.0f), 1.0f);

	Vec2 centroFundo(canto.x + largura * 0.5f, canto.y + altura * 0.5f);
	drawSolid(r, centroFundo, largura, altura, glm::vec4(0.0f, 0.0f, 0.0f, 0.45f));

	float cheia = largura * fracao;
	if (cheia > 0.0f)
	{
		Vec2 centro(canto.x + cheia * 0.5f, canto.y + altura * 0.5f);
		drawSolid(r, centro, cheia, altura, cor);
	}
}

void drawHud(Renderer &r, const World &w)
{
	const float margem = 14.0f;
	const float alturaDigito = 22.0f;

	// --- Pontuacao, canto superior esquerdo ---
	drawNumber(r, w.score, Vec2(margem, margem + alturaDigito * 0.5f), alturaDigito);

	// --- Vida: coracoes como barra, canto superior direito ---
	float fracaoVida = (w.cfg.playerHealth > 0)
						   ? (float)w.jogador.health / (float)w.cfg.playerHealth
						   : 0.0f;
	float barraW = 150.0f;
	drawBar(r, Vec2(r.telaW - margem - barraW, margem), barraW, 16.0f,
			fracaoVida, glm::vec4(0.95f, 0.35f, 0.35f, 0.95f));

	// --- Bombas: icone mais o numero, abaixo da vida ---
	float xArma = r.telaW - margem - barraW + 10.0f;
	float yArma = margem + 38.0f;
	drawTile(r, AtlasId::Tiles, SPRITES[SPR_BOMB].tile, Vec2(xArma, yArma), 0.0f, 20.0f, 20.0f);
	drawNumber(r, w.jogador.bombs, Vec2(xArma + 18.0f, yArma), 20.0f);

	// --- Tanques restantes, ao lado da pontuacao ---
	float yInim = margem + alturaDigito + 22.0f;
	drawTile(r, AtlasId::Tiles, SPRITES[SPR_TANK].tile,
			 Vec2(margem + 12.0f, yInim), 0.0f, 22.0f, 22.0f);
	drawNumber(r, w.inimigosVivos, Vec2(margem + 28.0f, yInim), 20.0f);

	// --- Fim de jogo: faixa escura atravessando a tela ---
	if (w.gameOver)
	{
		drawSolid(r, Vec2(r.telaW * 0.5f, r.telaH * 0.5f),
				  (float)r.telaW, 96.0f, glm::vec4(0.0f, 0.0f, 0.0f, 0.72f));

		glm::vec4 cor = w.venceu ? glm::vec4(0.4f, 1.0f, 0.5f, 1.0f)
								 : glm::vec4(1.0f, 0.4f, 0.4f, 1.0f);

		// Sem fonte para texto longo: a faixa colorida mais a pontuacao
		// grande comunicam o resultado. O titulo da janela escreve por extenso.
		drawSolid(r, Vec2(r.telaW * 0.5f, r.telaH * 0.5f - 34.0f),
				  220.0f, 6.0f, cor);
		drawNumber(r, w.score, Vec2(r.telaW * 0.5f - 70.0f, r.telaH * 0.5f + 6.0f), 44.0f);
	}
}
