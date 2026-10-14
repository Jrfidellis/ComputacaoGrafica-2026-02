#include "render/atlas.h"

#include <cmath>
#include <iostream>

#include "stb_image.h"

bool loadAtlas(Atlas &atlas, const std::string &caminho, int tileW, int tileH)
{
	// NAO chamar stbi_set_flip_vertically_on_load.
	//
	// A stb entrega a imagem com a primeira linha no topo. A OpenGL, por
	// convencao, trata v=0 como a base da textura. Se invertermos no
	// carregamento, os indices de linha da folha passam a contar de baixo
	// para cima, o que e horrivel de usar. Nao invertendo, esse "de cabeca
	// para baixo" cancela exatamente com a projecao ortho(0,W,H,0), que ja
	// tem o Y crescendo para baixo. O resultado e um modelo mental unico:
	// UV e tela medem a partir do canto superior esquerdo.

	int w = 0, h = 0, canais = 0;
	unsigned char *pixels = stbi_load(caminho.c_str(), &w, &h, &canais, 4);
	if (!pixels)
	{
		std::cerr << "[atlas] FALHA ao carregar " << caminho
				  << ": " << stbi_failure_reason() << std::endl;
		return false;
	}

	std::cout << "[atlas] " << caminho << " -> " << w << "x" << h
			  << " (" << canais << " canais na origem, forcado para 4)" << std::endl;

	glGenTextures(1, &atlas.tex);
	glBindTexture(GL_TEXTURE_2D, atlas.tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(pixels);

	atlas.folhaW = w;
	atlas.folhaH = h;
	atlas.tileW = tileW;
	atlas.tileH = tileH;
	atlas.colunas = w / tileW;

	glCheck("loadAtlas");
	return true;
}

void tileUV(const Atlas &atlas, int indice, float offset[2], float escala[2])
{
	if (indice < 0 || atlas.colunas <= 0)
	{
		// Sem textura: usa a folha inteira. Quem chamar vai ligar a textura
		// branca de 1x1, entao o resultado e um quad de cor solida.
		offset[0] = 0.0f;
		offset[1] = 0.0f;
		escala[0] = 1.0f;
		escala[1] = 1.0f;
		return;
	}

	int coluna = indice % atlas.colunas;
	int linha = indice / atlas.colunas;

	float fW = (float)atlas.folhaW;
	float fH = (float)atlas.folhaH;

	// Meio texel para dentro em cada lado, e por isso a escala perde 1 pixel.
	offset[0] = (coluna * atlas.tileW + 0.5f) / fW;
	offset[1] = (linha * atlas.tileH + 0.5f) / fH;
	escala[0] = (atlas.tileW - 1.0f) / fW;
	escala[1] = (atlas.tileH - 1.0f) / fH;
}

GLuint createCloudTexture()
{
	const int N = 64;
	unsigned char pixels[N * N * 4];

	for (int y = 0; y < N; y++)
	{
		for (int x = 0; x < N; x++)
		{
			// Distancia normalizada do centro, de 0 no meio a 1 na borda
			float dx = (x + 0.5f) / N * 2.0f - 1.0f;
			float dy = (y + 0.5f) / N * 2.0f - 1.0f;
			float d = std::sqrt(dx * dx + dy * dy);

			// smoothstep invertido: opaco no centro, zero a partir da borda
			float a = 1.0f - d;
			if (a < 0.0f)
				a = 0.0f;
			a = a * a * (3.0f - 2.0f * a);

			size_t i = ((size_t)y * N + x) * 4;
			pixels[i + 0] = 255;
			pixels[i + 1] = 255;
			pixels[i + 2] = 255;
			pixels[i + 3] = (unsigned char)(a * 255.0f);
		}
	}

	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, N, N, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	// Aqui GL_LINEAR e o certo, ao contrario dos sprites: esta textura e um
	// degrade, nao pixel art, e a interpolacao e justamente o que suaviza a
	// borda. Nao ha risco de halo porque o RGB e branco em todos os texels.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glCheck("createCloudTexture");
	return tex;
}

GLuint createWhiteTexture()
{
	GLuint tex = 0;
	const unsigned char branco[4] = {255, 255, 255, 255};

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, branco);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	glCheck("createWhiteTexture");
	return tex;
}
