#include "game/collision.h"

#include <cmath>
#include <cstdio>

bool aabbOverlap(const AABB &a, const AABB &b)
{
	// Duas caixas alinhadas se tocam quando se sobrepoem nos DOIS eixos.
	// Basta um eixo separado para nao haver colisao.
	if (std::fabs(a.center.x - b.center.x) > (a.half.x + b.half.x))
		return false;
	if (std::fabs(a.center.y - b.center.y) > (a.half.y + b.half.y))
		return false;
	return true;
}

static int checa(const char *nome, bool obtido, bool esperado)
{
	bool ok = (obtido == esperado);
	std::printf("  [%s] %-34s esperado=%d obtido=%d\n",
				ok ? "PASS" : "FAIL", nome, esperado ? 1 : 0, obtido ? 1 : 0);
	return ok ? 0 : 1;
}

int collisionSelfTest()
{
	std::printf("[selftest] aabbOverlap\n");
	int falhas = 0;

	AABB a = makeAABB(Vec2(0.0f, 0.0f), 10.0f, 10.0f);

	falhas += checa("identicas",
					aabbOverlap(a, makeAABB(Vec2(0.0f, 0.0f), 10.0f, 10.0f)), true);

	falhas += checa("sobrepostas parcialmente",
					aabbOverlap(a, makeAABB(Vec2(15.0f, 0.0f), 10.0f, 10.0f)), true);

	falhas += checa("separadas no eixo X",
					aabbOverlap(a, makeAABB(Vec2(21.0f, 0.0f), 10.0f, 10.0f)), false);

	falhas += checa("separadas no eixo Y",
					aabbOverlap(a, makeAABB(Vec2(0.0f, 21.0f), 10.0f, 10.0f)), false);

	// Separada em Y mesmo com X totalmente sobreposto: pega a implementacao
	// errada que testa apenas um dos eixos.
	falhas += checa("X sobreposto mas Y separado",
					aabbOverlap(a, makeAABB(Vec2(0.0f, 40.0f), 10.0f, 10.0f)), false);

	// Uma caixa inteiramente dentro da outra ainda e colisao.
	falhas += checa("uma dentro da outra",
					aabbOverlap(a, makeAABB(Vec2(2.0f, 2.0f), 1.0f, 1.0f)), true);

	std::printf("[selftest] aabbOverlap: %d falha(s)\n", falhas);
	return falhas;
}
