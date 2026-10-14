#include "core/sprites.h"

#include <algorithm>
#include <glm/gtc/constants.hpp>

// Tabela unica de sprites do jogo.
//
// Detalhe importante da arte da Kenney: as 24 aeronaves de ships_packed.png
// apontam para CIMA, mas o canhao da torre do tanque (tile 30) e o tanque
// montado (tile 28) apontam para BAIXO. Essa diferenca de 180 graus fica
// isolada aqui no campo angleOffset. Nenhuma linha de gameplay precisa saber
// disso: para o jogo, angulo 0 sempre significa "nariz para cima".
//
// tile < 0 significa "sem textura": o renderizador usa a textura branca 1x1 e
// pinta o quad com a cor do tint. E o caso das balas.

const float PI = glm::pi<float>();

const SpriteDef SPRITES[SPR_COUNT] = {
	// atlas          tile frames largura altura angleOffset
	{AtlasId::Ships, 0, 1, 38.0f, 38.0f, 0.0f},	   // SPR_PLAYER      aviao azul
	{AtlasId::Tiles, 28, 1, 30.0f, 30.0f, PI},	   // SPR_TANK        casco + torre
	{AtlasId::Tiles, 29, 1, 30.0f, 30.0f, 0.0f},   // SPR_TANK_HULL   so o casco
	{AtlasId::Tiles, 30, 1, 30.0f, 30.0f, PI},	   // SPR_TANK_TURRET so a torre
	{AtlasId::Tiles, -1, 1, 5.0f, 12.0f, 0.0f},	   // SPR_BULLET      quad solido
	{AtlasId::Tiles, 12, 1, 16.0f, 16.0f, 0.0f},   // SPR_BOMB        bomba simples
	// A UNICA animacao do jogo: tres quadros consecutivos, 4, 5 e 6. O clarao
	// abre, alarga e se despedaca numa estrela.
	{AtlasId::Tiles, 4, 3, 46.0f, 46.0f, 0.0f},	   // SPR_EXPLOSION   tiles 4,5,6
};

int spriteFrameTile(const SpriteDef &def, float progresso)
{
	if (def.frames <= 1 || def.tile < 0)
		return def.tile;

	// Divide o progresso em frames faixas iguais. O clamp protege as pontas:
	// progresso exatamente 1.0 cairia no indice frames, que nao existe.
	int quadro = (int)(progresso * (float)def.frames);
	quadro = std::min(std::max(quadro, 0), def.frames - 1);
	return def.tile + quadro;
}

// Os digitos nao sao contiguos na folha: 0 a 4 ficam nos tiles 19-23 e
// 5 a 9 ficam nos tiles 31-35, porque a grade tem 12 colunas e a faixa de
// digitos ocupa as 5 ultimas colunas de duas linhas seguidas.
const int DIGIT_TILE[10] = {19, 20, 21, 22, 23, 31, 32, 33, 34, 35};
