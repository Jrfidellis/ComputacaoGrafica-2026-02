#include "render/renderer.h"

#include "game/world.h"
#include "render/hud.h"
#include "render/layers.h"
#include "render/radar.h"
#include "render/shader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

bool initRenderer(Renderer &r, const std::string &raiz, int telaW, int telaH)
{
	r.telaW = telaW;
	r.telaH = telaH;

	r.prog = loadShaderProgram(raiz + "/shaders/sprite.vert", raiz + "/shaders/sprite.frag");
	if (!r.prog)
		return false;

	r.uProjection = glGetUniformLocation(r.prog, "projection");
	r.uView = glGetUniformLocation(r.prog, "view");
	r.uModel = glGetUniformLocation(r.prog, "model");
	r.uUvOffset = glGetUniformLocation(r.prog, "uvOffset");
	r.uUvScale = glGetUniformLocation(r.prog, "uvScale");
	r.uTint = glGetUniformLocation(r.prog, "tint");
	r.uTex = glGetUniformLocation(r.prog, "tex");

	// O QUAD UNITARIO, centrado na origem.
	//
	// Centrado e nao de (0,0) a (1,1): se o canto fosse a origem, todo sprite
	// giraria em torno do proprio canto superior esquerdo em vez do centro.
	// Esta geometria e enviada uma unica vez e nunca mais muda. Todo o resto
	// do jogo e esta mesma malha com uma matriz model diferente.
	const GLfloat vertices[] = {
		// posicao        UV
		-0.5f, -0.5f, 0.0f, 0.0f,
		 0.5f, -0.5f, 1.0f, 0.0f,
		 0.5f,  0.5f, 1.0f, 1.0f,

		-0.5f, -0.5f, 0.0f, 0.0f,
		 0.5f,  0.5f, 1.0f, 1.0f,
		-0.5f,  0.5f, 0.0f, 1.0f,
	};

	glGenVertexArrays(1, &r.vao);
	glGenBuffers(1, &r.vbo);

	glBindVertexArray(r.vao);
	glBindBuffer(GL_ARRAY_BUFFER, r.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// VBO intercalado: 4 floats por vertice, sendo 2 de posicao e 2 de UV.
	const GLsizei stride = 4 * sizeof(GLfloat);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid *)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	if (!loadAtlas(r.ships, raiz + "/assets/ships_packed.png", 32, 32))
		return false;
	if (!loadAtlas(r.tiles, raiz + "/assets/tiles_packed.png", 16, 16))
		return false;
	r.white = createWhiteTexture();
	r.cloud = createCloudTexture();

	// Projecao ortografica em coordenadas de tela, com a origem no canto
	// superior esquerdo e o Y crescendo para baixo.
	// Os literais sao 0.0f e nao 0.0: com double a GLM devolve dmat4, e o
	// value_ptr passaria doubles para uma funcao que espera floats.
	r.projection = glm::ortho(0.0f, (float)telaW, (float)telaH, 0.0f, -1.0f, 1.0f);

	glUseProgram(r.prog);
	glUniformMatrix4fv(r.uProjection, 1, GL_FALSE, glm::value_ptr(r.projection));
	glUniform1i(r.uTex, 0);
	setView(r, glm::mat4(1.0f));

	// 2D puro: a ordem de desenho e quem decide o que fica por cima.
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glCheck("initRenderer");
	return true;
}

void destroyRenderer(Renderer &r)
{
	if (r.vao)
		glDeleteVertexArrays(1, &r.vao);
	if (r.vbo)
		glDeleteBuffers(1, &r.vbo);
	if (r.prog)
		glDeleteProgram(r.prog);
	if (r.ships.tex)
		glDeleteTextures(1, &r.ships.tex);
	if (r.tiles.tex)
		glDeleteTextures(1, &r.tiles.tex);
	if (r.white)
		glDeleteTextures(1, &r.white);
	if (r.cloud)
		glDeleteTextures(1, &r.cloud);
}

void setView(Renderer &r, const glm::mat4 &view)
{
	glUniformMatrix4fv(r.uView, 1, GL_FALSE, glm::value_ptr(view));
}

glm::mat4 makeModelMatrix(Vec2 pos, float anguloRad, float largura, float altura)
{
	glm::mat4 m(1.0f);
	m = glm::translate(m, glm::vec3(pos.x, pos.y, 0.0f));
	m = glm::rotate(m, anguloRad, glm::vec3(0.0f, 0.0f, 1.0f));
	m = glm::scale(m, glm::vec3(largura, altura, 1.0f));
	return m;
}

// Emite um quad. E o unico ponto do programa que chama glDrawArrays.
static void emitir(Renderer &r, GLuint textura, const float offset[2], const float escala[2],
				   const glm::mat4 &model, const glm::vec4 &tint, GLenum modo, GLint primeiro, GLsizei qtd)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textura);

	glUniformMatrix4fv(r.uModel, 1, GL_FALSE, glm::value_ptr(model));
	glUniform2f(r.uUvOffset, offset[0], offset[1]);
	glUniform2f(r.uUvScale, escala[0], escala[1]);
	glUniform4f(r.uTint, tint.r, tint.g, tint.b, tint.a);

	glBindVertexArray(r.vao);
	glDrawArrays(modo, primeiro, qtd);

	r.quadsDesenhados++;
}

void drawTile(Renderer &r, AtlasId atlasId, int tile, Vec2 pos, float anguloRad,
			  float largura, float altura, const glm::vec4 &tint)
{
	const Atlas &atlas = (atlasId == AtlasId::Ships) ? r.ships : r.tiles;

	float offset[2], escala[2];
	tileUV(atlas, tile, offset, escala);

	GLuint textura = (tile < 0) ? r.white : atlas.tex;
	emitir(r, textura, offset, escala,
		   makeModelMatrix(pos, anguloRad, largura, altura), tint, GL_TRIANGLES, 0, 6);
}

void drawSprite(Renderer &r, int spriteId, Vec2 pos, float anguloRad,
				float escala, const glm::vec4 &tint, float progressoAnim)
{
	const SpriteDef &def = SPRITES[spriteId];

	// spriteFrameTile escolhe o quadro da animacao. Para sprites de quadro
	// unico ele devolve o proprio tile, entao este caminho serve aos dois.
	int tile = spriteFrameTile(def, progressoAnim);

	// O angleOffset corrige a orientacao da ARTE (a torre do tanque aponta
	// para baixo, os avioes apontam para cima). O gameplay nunca ve isso.
	drawTile(r, def.atlas, tile, pos, anguloRad + def.angleOffset,
			 def.sizeW * escala, def.sizeH * escala, tint);
}

void drawSolid(Renderer &r, Vec2 pos, float largura, float altura,
			   const glm::vec4 &cor, float anguloRad)
{
	const float offset[2] = {0.0f, 0.0f};
	const float escala[2] = {1.0f, 1.0f};
	emitir(r, r.white, offset, escala,
		   makeModelMatrix(pos, anguloRad, largura, altura), cor, GL_TRIANGLES, 0, 6);
}

void drawTextured(Renderer &r, GLuint textura, Vec2 pos, float largura, float altura,
				  const glm::vec4 &tint, float anguloRad)
{
	const float offset[2] = {0.0f, 0.0f};
	const float escala[2] = {1.0f, 1.0f};
	emitir(r, textura, offset, escala,
		   makeModelMatrix(pos, anguloRad, largura, altura), tint, GL_TRIANGLES, 0, 6);
}

void drawOutline(Renderer &r, Vec2 centro, float halfW, float halfH, const glm::vec4 &cor,
				 float espessura)
{
	const float esp = espessura;
	drawSolid(r, Vec2(centro.x, centro.y - halfH), halfW * 2.0f, esp, cor);
	drawSolid(r, Vec2(centro.x, centro.y + halfH), halfW * 2.0f, esp, cor);
	drawSolid(r, Vec2(centro.x - halfW, centro.y), esp, halfH * 2.0f, cor);
	drawSolid(r, Vec2(centro.x + halfW, centro.y), esp, halfH * 2.0f, cor);
}

void renderFrame(Renderer &r, const World &w)
{
	r.tilesDesenhados = 0;
	r.quadsDesenhados = 0;

	glViewport(0, 0, r.fbW, r.fbH);
	glClearColor(0.07f, 0.10f, 0.16f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(r.prog);
	glUniformMatrix4fv(r.uProjection, 1, GL_FALSE, glm::value_ptr(r.projection));

	drawTerrain(r, w);
	drawLowClouds(r, w);
	drawEntities(r, w);
	drawCloudLayer(r, w);

	// A partir daqui tudo e espaco de tela: a camera volta para a origem.
	setView(r, glm::mat4(1.0f));
	drawHud(r, w);
	drawRadar(r, w);

	glBindVertexArray(0);
}
