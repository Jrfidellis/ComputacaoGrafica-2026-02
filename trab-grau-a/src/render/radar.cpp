#include "render/radar.h"

#include "game/flight.h"
#include "game/world.h"
#include "render/renderer.h"

Vec2 worldToRadar(const World &w, Vec2 mundo, Vec2 cantoRadar, float tamanhoRadar)
{
	// O mundo e quadrado, entao uma unica escala serve para os dois eixos e o
	// mapa nao distorce dentro do painel quadrado.
	float escala = tamanhoRadar / worldSize(w.mapa);
	return cantoRadar + mundo * escala;
}

void drawRadar(Renderer &r, const World &w)
{
	const float tam = w.cfg.radarSize;
	const float margem = w.cfg.radarMargin;

	// Canto superior esquerdo do painel, no canto INFERIOR esquerdo da tela.
	// Lembrando que o Y cresce para baixo: o fundo da tela e y = telaH.
	Vec2 canto(margem, r.telaH - margem - tam);
	Vec2 centro(canto.x + tam * 0.5f, canto.y + tam * 0.5f);

	// Fundo e moldura
	drawSolid(r, centro, tam, tam, glm::vec4(0.03f, 0.09f, 0.07f, 0.78f));

	const glm::vec4 borda(0.35f, 0.85f, 0.55f, 0.85f);
	const float e = 2.0f;
	drawSolid(r, Vec2(centro.x, canto.y), tam, e, borda);
	drawSolid(r, Vec2(centro.x, canto.y + tam), tam, e, borda);
	drawSolid(r, Vec2(canto.x, centro.y), e, tam, borda);
	drawSolid(r, Vec2(canto.x + tam, centro.y), e, tam, borda);

	// Retangulo mostrando a area que a tela esta cobrindo agora
	{
		float escala = tam / worldSize(w.mapa);
		Vec2 vistaCanto = worldToRadar(w, w.camera.topLeft, canto, tam);
		float vw = r.telaW * escala;
		float vh = r.telaH * escala;
		Vec2 c(vistaCanto.x + vw * 0.5f, vistaCanto.y + vh * 0.5f);

		const glm::vec4 fraco(0.5f, 0.9f, 0.7f, 0.35f);
		drawSolid(r, Vec2(c.x, vistaCanto.y), vw, 1.0f, fraco);
		drawSolid(r, Vec2(c.x, vistaCanto.y + vh), vw, 1.0f, fraco);
		drawSolid(r, Vec2(vistaCanto.x, c.y), 1.0f, vh, fraco);
		drawSolid(r, Vec2(vistaCanto.x + vw, c.y), 1.0f, vh, fraco);
	}

	// Pontos vermelhos: um por inimigo vivo. O tamanho e FIXO em pixels de
	// tela, independente da escala do mapa, que e justamente o motivo de a
	// conversao ser feita aqui na CPU.
	for (const Entity &ent : w.entidades)
	{
		if (!ent.alive)
			continue;
		if (ent.kind != EntityKind::GroundEnemy)
			continue;

		Vec2 p = worldToRadar(w, ent.pos, canto, tam);
		drawSolid(r, p, 5.0f, 5.0f, glm::vec4(1.0f, 0.25f, 0.25f, 0.95f));
	}

	// Seta do jogador. Gira com o mesmo angulo do aviao, entao voar em circulo
	// faz a seta girar no mesmo sentido: e uma verificacao gratuita de que a
	// convencao de angulo esta aplicada de forma consistente nos dois lugares.
	if (w.jogador.alive)
	{
		Vec2 p = worldToRadar(w, w.jogador.pos, canto, tam);
		const glm::vec4 branco(1.0f, 1.0f, 1.0f, 1.0f);

		// Corpo da seta: um retangulo fino alinhado com o nariz
		drawSolid(r, p, 3.0f, 12.0f, branco, w.jogador.angle);

		// Ponta: um quadradinho deslocado na direcao do nariz
		Vec2 frente = forwardFromAngle(w.jogador.angle);
		drawSolid(r, p + frente * 6.0f, 6.0f, 6.0f, branco, w.jogador.angle);
	}
}
