#pragma once

// Matematica do modelo de voo. Tudo inline porque sao funcoes de 2 ou 3 linhas
// usadas em varios lugares.
//
// CONVENCAO DE ANGULO, fixada aqui e valida no jogo inteiro:
//
//   angulo 0        -> nariz apontando para CIMA na tela
//   angulo positivo -> giro HORARIO
//
// O giro positivo e horario porque a projecao e ortho(0, 800, 600, 0), ou seja
// o eixo Y cresce para BAIXO. Derivacao: o vertice do nariz esta em (0, -0.5)
// no espaco local. A rotacao padrao leva (x, y) para
// (x*cos - y*sin, x*sin + y*cos), o que da (0.5*sin a, -0.5*cos a).
// Em a = +90 graus isso e (+0.5, 0), que e a direita da tela.
//
// Por isso o vetor para a frente e (sin a, -cos a), e NAO (cos a, sin a).

#include "core/types.h"

#include <cmath>
#include <glm/gtc/constants.hpp>

// Vetor unitario na direcao em que o nariz aponta.
inline Vec2 forwardFromAngle(float a)
{
	return Vec2(std::sin(a), -std::cos(a));
}

// Normaliza um angulo para a faixa (-PI, PI].
inline float wrapAngle(float a)
{
	const float DOIS_PI = glm::two_pi<float>();
	while (a > glm::pi<float>())
		a -= DOIS_PI;
	while (a <= -glm::pi<float>())
		a += DOIS_PI;
	return a;
}

// Angulo necessario para que o nariz aponte de "origem" para "alvo".
// E a inversa de forwardFromAngle: se forward = (sin a, -cos a), entao
// sin a = dx e cos a = -dy, logo a = atan2(dx, -dy).
inline float headingTo(Vec2 origem, Vec2 alvo)
{
	Vec2 d = alvo - origem;
	return std::atan2(d.x, -d.y);
}

// Gira "atual" em direcao a "alvo", no maximo maxPasso radianos.
// Escolhe sempre o lado mais curto do circulo.
inline float turnToward(float atual, float alvo, float maxPasso)
{
	float diff = wrapAngle(alvo - atual);
	if (diff > maxPasso)
		diff = maxPasso;
	if (diff < -maxPasso)
		diff = -maxPasso;
	return wrapAngle(atual + diff);
}

// Avanca uma posicao seguindo o nariz. Usado pelo jogador e, na v2, pelos
// avioes inimigos, que sao o mesmo modelo de voo com outro controle.
inline Vec2 stepAirplane(Vec2 pos, float angulo, float velocidade, float dt)
{
	return pos + forwardFromAngle(angulo) * velocidade * dt;
}
