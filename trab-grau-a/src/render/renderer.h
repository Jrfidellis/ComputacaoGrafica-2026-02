#pragma once

#include "core/sprites.h"
#include "core/types.h"
#include "render/atlas.h"
#include "render/gl.h"

#include <glm/glm.hpp>
#include <string>

struct World;

// Estado do renderizador. Tudo que e mutavel durante o desenho mora aqui, e
// nao no World, justamente para que as funcoes de desenho possam receber o
// mundo como const World&.
struct Renderer
{
	GLuint prog = 0;
	GLuint vao = 0, vbo = 0;

	// Localizacoes dos uniforms, buscadas uma vez na inicializacao
	GLint uProjection = -1, uView = -1, uModel = -1;
	GLint uUvOffset = -1, uUvScale = -1, uTint = -1, uTex = -1;

	Atlas ships;
	Atlas tiles;
	GLuint white = 0;
	GLuint cloud = 0;

	glm::mat4 projection = glm::mat4(1.0f);

	// telaW/telaH sao as unidades LOGICAS, as mesmas da projecao ortografica.
	// fbW/fbH sao os PIXELS REAIS do framebuffer. Numa tela Retina os dois
	// diferem por um fator de 2, e a viewport precisa dos pixels reais, senao
	// o jogo e desenhado so num quarto da janela.
	int telaW = 800, telaH = 600;
	int fbW = 800, fbH = 600;

	// Contadores de diagnostico, zerados a cada frame
	int tilesDesenhados = 0;
	int quadsDesenhados = 0;
};

bool initRenderer(Renderer &r, const std::string &raizProjeto, int telaW, int telaH);
void destroyRenderer(Renderer &r);

// -- primitivas de desenho ---------------------------------------------------
//
// Disciplina de uniforms, seguida em todo o codigo de desenho:
//   projection : enviada uma vez na inicializacao
//   view       : enviada uma vez por GRUPO de desenho (camada, HUD, radar)
//   model      : enviada a cada objeto
// Esquecer de reenviar o view ao trocar de grupo e a causa mais comum de
// "uma camada apareceu no lugar errado".

void setView(Renderer &r, const glm::mat4 &view);

// Monta a matriz do objeto. A ordem e translate * rotate * scale: na pratica
// o vertice e escalado primeiro, depois girado, depois movido. Inverter isso
// faz o objeto orbitar a origem do mundo em vez de girar no proprio eixo.
glm::mat4 makeModelMatrix(Vec2 pos, float anguloRad, float largura, float altura);

// Desenha um tile de uma das folhas.
void drawTile(Renderer &r, AtlasId atlas, int tile, Vec2 pos, float anguloRad,
			  float largura, float altura, const glm::vec4 &tint = glm::vec4(1.0f));

// Desenha um sprite da tabela SPRITES, ja aplicando o angleOffset da arte.
//
// progressoAnim vai de 0 a 1 e escolhe o quadro, para sprites que tem mais de
// um. Sprites de quadro unico ignoram o valor.
void drawSprite(Renderer &r, int spriteId, Vec2 pos, float anguloRad,
				float escala = 1.0f, const glm::vec4 &tint = glm::vec4(1.0f),
				float progressoAnim = 0.0f);

// Quad de cor solida, usando a textura branca de 1x1.
void drawSolid(Renderer &r, Vec2 pos, float largura, float altura,
			   const glm::vec4 &cor, float anguloRad = 0.0f);

// Quad com uma textura qualquer, usada inteira (UV de 0 a 1). E o caminho das
// nuvens, que nao vem de uma folha de sprites.
void drawTextured(Renderer &r, GLuint textura, Vec2 pos, float largura, float altura,
				  const glm::vec4 &tint, float anguloRad = 0.0f);

// Contorno de retangulo em GL_LINE_LOOP, para o debug de hitbox da tecla H.
void drawOutline(Renderer &r, Vec2 centro, float halfW, float halfH, const glm::vec4 &cor,
				 float espessura = 1.5f);

// Desenha o frame inteiro a partir do estado do mundo.
// O mundo entra como const: o renderizador nao tem como alterar o jogo.
void renderFrame(Renderer &r, const World &w);
