#pragma once

#include <vector>

// Grade de tiles do terreno. Guarda apenas INDICES de tile: a traducao de
// indice para coordenada de textura acontece do outro lado da fronteira,
// em render/atlas.cpp. Este arquivo nao sabe o que e uma textura.
struct TileMap
{
	int largura = 0;  // em tiles
	int altura = 0;	  // em tiles
	float tileSize = 32.0f;
	int borda = 2;	  // espessura da faixa de agua, em tiles
	std::vector<int> tiles;
};

// Gera o mapa proceduralmente a partir de uma semente. Manchas de areia sobre
// um fundo de grama, decoracao esparsa e uma faixa de agua na borda.
void generateMap(TileMap &mapa, int ladoEmTiles, float tileSize, int borda, unsigned int seed);

// Indice do tile numa coluna/linha. Fora dos limites devolve agua.
int tileAt(const TileMap &mapa, int coluna, int linha);

// E agua? Usado para nao nascer inimigo em cima da agua.
bool isWater(const TileMap &mapa, int coluna, int linha);

// Tamanho do mundo em unidades (nao em tiles). O mundo e quadrado.
inline float worldSize(const TileMap &mapa)
{
	return mapa.largura * mapa.tileSize;
}
