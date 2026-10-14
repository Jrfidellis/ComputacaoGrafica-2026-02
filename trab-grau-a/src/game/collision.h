#pragma once

#include "core/types.h"

// Colisao por hitbox alinhada aos eixos (AABB).
//
// Por que AABB e nao um retangulo orientado: o aviao gira, entao o certo seria
// um OBB. Mas uma caixa alinhada e um pouco MENOR que o sprite perdoa mais o
// jogador, que e melhor para o jogo, e deixa uma unica funcao servir balas,
// tanques e o aviao. O raio da bomba e o unico caso onde circulo e o certo,
// e ali e um circulo mesmo, nao uma caixa.

// As duas caixas se tocam?
bool aabbOverlap(const AABB &a, const AABB &b);

// Monta uma AABB a partir de centro e metades.
inline AABB makeAABB(Vec2 centro, float halfW, float halfH)
{
	AABB box;
	box.center = centro;
	box.half = Vec2(halfW, halfH);
	return box;
}

// Distancia ao quadrado, usada pelo raio da bomba sem pagar a raiz quadrada.
inline float dist2(Vec2 a, Vec2 b)
{
	Vec2 d = a - b;
	return d.x * d.x + d.y * d.y;
}

// Seis casos de teste rodados no modo --selftest. Imprime PASS/FAIL e devolve
// o numero de falhas. E a unica parte do jogo testavel sem abrir janela.
int collisionSelfTest();
