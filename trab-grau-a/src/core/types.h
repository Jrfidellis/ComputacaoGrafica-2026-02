#pragma once

// Tipos basicos compartilhados entre a camada de jogo e a de renderizacao.
// Este header NAO pode incluir nada de OpenGL. A GLM e permitida porque e
// matematica pura, nao fala com a placa de video.

#include <glm/glm.hpp>

using Vec2 = glm::vec2;

// Tipo de cada entidade do mundo. O switch por este enum e o unico lugar que
// precisa mudar para entrar um tipo novo.
enum class EntityKind
{
	GroundEnemy,
	Bullet,
	Bomb,
	Explosion
};

// De quem e o tiro. Evita a bala do jogador matar o proprio jogador.
enum class Team
{
	Player,
	Enemy
};

// Tudo que a camada de jogo sabe sobre o teclado e o mouse. A plataforma
// preenche, o jogo consome. Nenhum codigo de gameplay chama a GLFW.
struct InputState
{
	// Acoes continuas: valem enquanto a tecla esta pressionada
	bool turnLeft = false;
	bool turnRight = false;
	bool throttleUp = false;
	bool throttleDown = false;
	bool fireGun = false;

	// Acoes de borda: valem so no frame em que a tecla foi pressionada
	bool dropBomb = false;
	bool toggleHitboxes = false;
	bool restart = false;
};

// Caixa de colisao alinhada aos eixos, descrita pelo centro e pelas metades.
struct AABB
{
	Vec2 center;
	Vec2 half;
};
