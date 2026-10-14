#pragma once

// Tabela de sprites: liga um identificador logico ("o tanque") ao indice de
// tile dentro de uma das duas folhas, ao tamanho em unidades de mundo e a
// correcao de angulo da arte.
//
// Esta tabela vive na camada core, sem OpenGL, porque e so dado. Quem traduz
// indice de tile em coordenada de textura e o render/atlas.cpp.

// Qual das duas folhas de sprite usar.
enum class AtlasId
{
	Ships, // ships_packed.png : 128x192, grade 4x6 de 32x32
	Tiles  // tiles_packed.png : 192x160, grade 12x10 de 16x16
};

struct SpriteDef
{
	AtlasId atlas;
	int tile;			// PRIMEIRO quadro: linha * colunas + coluna
	int frames;			// quantos quadros CONSECUTIVOS a animacao usa
	float sizeW, sizeH; // tamanho desenhado, em unidades de mundo
	float angleOffset;	// radianos somados ao angulo da entidade
};

// ANIMACAO POR SPRITESHEET
//
// Os quadros de uma animacao sao tiles CONSECUTIVOS na folha, entao a
// animacao inteira cabe em dois numeros: o primeiro tile e a quantidade de
// quadros. Nao ha lista de quadros, nem struct de animacao, nem temporizador
// proprio.
//
// Quem avanca a animacao e o PROGRESSO do objeto, um valor de 0 a 1 que a
// propria entidade ja tem. A explosao, por exemplo, usa o quanto falta do seu
// tempo de vida. Isso faz cada explosao tocar a sua propria animacao do
// comeco, em vez de todas piscarem juntas num relogio global.
//
// progresso 0.0 -> primeiro quadro
// progresso 1.0 -> ultimo quadro
int spriteFrameTile(const SpriteDef &def, float progresso);

// Identificadores logicos usados pelo resto do codigo.
enum SpriteId
{
	SPR_PLAYER = 0,
	SPR_TANK,
	SPR_TANK_HULL,
	SPR_TANK_TURRET,
	SPR_BULLET,
	SPR_BOMB,
	SPR_EXPLOSION,
	SPR_COUNT
};

extern const SpriteDef SPRITES[SPR_COUNT];

// Os dez digitos vivem na folha de tiles, espalhados em duas faixas:
// 0 a 4 nos tiles 19-23 e 5 a 9 nos tiles 31-35. Esta tabela esconde isso.
extern const int DIGIT_TILE[10];

// Faixa de tiles de terreno (totalmente opacos) dentro de tiles_packed.png.
const int TERRAIN_FIRST = 36;
const int TERRAIN_LAST = 119;
