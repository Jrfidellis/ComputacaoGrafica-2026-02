#pragma once

#include "core/types.h"

#include <algorithm>

// A camera guarda o canto superior esquerdo da regiao visivel, em coordenadas
// de mundo. O renderizador transforma isso numa matriz de view com
// translate(-cam). Esta struct nao sabe o que e uma matriz.
struct Camera2D
{
	Vec2 topLeft = Vec2(0.0f);
};

// Centraliza a camera no alvo e depois trava nos limites do mundo.
//
// O travamento e a razao pela qual o aviao NAO fica exatamente no centro
// quando chega perto da borda do mapa. Isso e intencional: sem o travamento o
// jogador veria o vazio fora do mapa. A faixa de agua na borda existe
// justamente para que esse comportamento pareca proposital.
inline Vec2 cameraFollow(Vec2 alvo, float telaW, float telaH, float mundoW, float mundoH)
{
	Vec2 canto = alvo - Vec2(telaW * 0.5f, telaH * 0.5f);

	float maxX = std::max(0.0f, mundoW - telaW);
	float maxY = std::max(0.0f, mundoH - telaH);

	canto.x = std::min(std::max(canto.x, 0.0f), maxX);
	canto.y = std::min(std::max(canto.y, 0.0f), maxY);
	return canto;
}
