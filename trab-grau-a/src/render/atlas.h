#pragma once

#include "render/gl.h"

#include <string>

// Uma folha de sprites carregada como textura, mais os numeros necessarios
// para recortar um tile dela.
struct Atlas
{
	GLuint tex = 0;
	int folhaW = 0, folhaH = 0; // tamanho da imagem em pixels
	int tileW = 0, tileH = 0;	// tamanho de um tile em pixels
	int colunas = 0;
};

// Carrega um PNG e cria a textura. Usa GL_NEAREST e nao gera mipmap.
//
// GL_NEAREST nao e so escolha estetica de pixel art. As duas folhas da Kenney
// sao PNG indexado por paleta cujo indice transparente tem RGB preto, entao
// GL_LINEAR interpolaria contra preto e deixaria um halo escuro em volta de
// cada sprite. Mipmap seria pior ainda: o nivel 1 mistura tiles vizinhos.
bool loadAtlas(Atlas &atlas, const std::string &caminho, int tileW, int tileH);

// Converte um indice de tile no retangulo de UV correspondente.
//
// O recuo de meio texel existe porque a folha e empacotada sem espaco entre
// os tiles: amostrar exatamente na divisa pode arredondar para o tile vizinho
// e aparecer uma tira de 1 pixel do sprite errado na borda. Por isso a escala
// usa (tile - 1) e nao (tile).
void tileUV(const Atlas &atlas, int indice, float offset[2], float escala[2]);

// Textura branca de 1x1. Combinada com o uniform tint, permite desenhar quads
// de cor solida (balas, barras do HUD, pontos do radar, contornos de hitbox)
// com o mesmo shader dos sprites.
GLuint createWhiteTexture();

// Textura de nuvem gerada em codigo: um disco branco com o alpha caindo
// suavemente do centro para a borda. Nao existe sprite de nuvem no pacote da
// Kenney, e um quad solido translucido aparece como um retangulo obvio na
// tela. Com esta textura a mesma chamada de desenho vira uma mancha macia.
GLuint createCloudTexture();
