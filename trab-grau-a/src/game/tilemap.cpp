#include "game/tilemap.h"

#include "core/sprites.h"

#include <cmath>
#include <random>

// Tiles escolhidos na folha tiles_packed.png apos medir a pureza de cor de
// cada um dos 84 tiles opacos:
//   50  = grama lisa  (94% de uma cor so)
//   56  = areia lisa  (94%)
//   42  = agua/gelo   (100%, cor unica) -> borda do mapa
// Os demais sao variacoes com pequena decoracao, usados esparsamente para o
// terreno nao ficar liso demais.
static const int TILE_GRAMA = 50;
static const int TILE_AREIA = 56;
static const int TILE_AGUA = 42;
static const int DECOR_GRAMA[] = {52, 53, 100, 101};
static const int DECOR_AREIA[] = {58, 59, 106, 107};

void generateMap(TileMap &mapa, int ladoEmTiles, float tileSize, int borda, unsigned int seed)
{
	mapa.largura = ladoEmTiles;
	mapa.altura = ladoEmTiles;
	mapa.tileSize = tileSize;
	mapa.borda = borda;
	mapa.tiles.assign((size_t)ladoEmTiles * ladoEmTiles, TILE_GRAMA);

	std::mt19937 rng(seed);
	std::uniform_real_distribution<float> sorteio(0.0f, 1.0f);
	std::uniform_int_distribution<int> coord(0, ladoEmTiles - 1);

	// Algumas manchas circulares de areia espalhadas pelo mapa
	int manchas = ladoEmTiles / 6;
	for (int m = 0; m < manchas; m++)
	{
		int cx = coord(rng);
		int cy = coord(rng);
		int raio = 3 + (int)(sorteio(rng) * 7.0f);

		for (int l = cy - raio; l <= cy + raio; l++)
		{
			for (int c = cx - raio; c <= cx + raio; c++)
			{
				if (c < 0 || l < 0 || c >= ladoEmTiles || l >= ladoEmTiles)
					continue;
				float dx = (float)(c - cx);
				float dy = (float)(l - cy);
				// Borda irregular: o raio efetivo varia um pouco por tile
				if (dx * dx + dy * dy <= raio * raio * (0.6f + 0.4f * sorteio(rng)))
					mapa.tiles[(size_t)l * ladoEmTiles + c] = TILE_AREIA;
			}
		}
	}

	// Decoracao esparsa, coerente com o terreno de baixo
	for (int l = 0; l < ladoEmTiles; l++)
	{
		for (int c = 0; c < ladoEmTiles; c++)
		{
			if (sorteio(rng) > 0.06f)
				continue;
			size_t i = (size_t)l * ladoEmTiles + c;
			int escolha = (int)(sorteio(rng) * 4.0f) & 3;
			mapa.tiles[i] = (mapa.tiles[i] == TILE_AREIA) ? DECOR_AREIA[escolha] : DECOR_GRAMA[escolha];
		}
	}

	// Faixa de agua na borda. A camera trava antes dela, entao o jogador ve a
	// agua e entende que o mapa acabou: o aviao sair do centro perto da borda
	// vira decisao de projeto visivel, e nao um bug.
	for (int l = 0; l < ladoEmTiles; l++)
	{
		for (int c = 0; c < ladoEmTiles; c++)
		{
			if (c < borda || l < borda || c >= ladoEmTiles - borda || l >= ladoEmTiles - borda)
				mapa.tiles[(size_t)l * ladoEmTiles + c] = TILE_AGUA;
		}
	}
}

int tileAt(const TileMap &mapa, int coluna, int linha)
{
	if (coluna < 0 || linha < 0 || coluna >= mapa.largura || linha >= mapa.altura)
		return TILE_AGUA;
	return mapa.tiles[(size_t)linha * mapa.largura + coluna];
}

bool isWater(const TileMap &mapa, int coluna, int linha)
{
	return tileAt(mapa, coluna, linha) == TILE_AGUA;
}
