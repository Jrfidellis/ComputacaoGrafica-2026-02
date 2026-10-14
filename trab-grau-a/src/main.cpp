// Voo Rasante -- jogo 2D de aviao visto de cima
// Trabalho Grau A, Processamento Grafico: Fundamentos, Unisinos 2026/2
//
// Este e o UNICO arquivo do projeto que enxerga os dois lados da fronteira.
// A camada de jogo (src/game) nao compila contra OpenGL: o alvo game_core do
// CMake nao tem a GLAD nem a GLFW no include path, entao a separacao entre
// regras do jogo e renderizacao e verificada pelo compilador.
//
// Fluxo de um frame:
//   platform/input  ->  InputState   ->  game/world  ->  const World&  ->  render

#include "core/config.h"
#include "game/collision.h"
#include "game/flight.h"
#include "game/world.h"
#include "platform/input.h"
#include "platform/window.h"
#include "render/renderer.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>

// Verificacao da convencao de angulo, rodada antes de abrir a janela.
//
// Esta e a checagem mais valiosa do projeto. Todos os erros de convencao de
// angulo (graus no lugar de radianos, (cos,sin) no lugar de (sin,-cos), sinal
// trocado no Y) produzem um aviao que voa de um jeito "quase certo", sem
// nenhuma mensagem de erro. Aqui eles aparecem como texto no console.
static int verificarConvencaoDeAngulo()
{
	struct Caso
	{
		float graus;
		float x, y;
		const char *descricao;
	};

	const Caso casos[] = {
		{0.0f, 0.0f, -1.0f, "nariz para cima"},
		{90.0f, 1.0f, 0.0f, "nariz para a direita"},
		{180.0f, 0.0f, 1.0f, "nariz para baixo"},
		{270.0f, -1.0f, 0.0f, "nariz para a esquerda"},
	};

	std::printf("[selftest] convencao de angulo: forward = (sin a, -cos a)\n");
	int falhas = 0;

	for (const Caso &c : casos)
	{
		float rad = c.graus * 3.14159265358979f / 180.0f;
		Vec2 f = forwardFromAngle(rad);
		bool ok = std::fabs(f.x - c.x) < 0.001f && std::fabs(f.y - c.y) < 0.001f;
		if (!ok)
			falhas++;

		std::printf("  [%s] %3.0f graus -> (%+.2f, %+.2f)  esperado (%+.2f, %+.2f)  %s\n",
					ok ? "PASS" : "FAIL", c.graus, f.x, f.y, c.x, c.y, c.descricao);
	}

	std::printf("[selftest] convencao de angulo: %d falha(s)\n", falhas);
	return falhas;
}

int main(int argc, char **argv)
{
	bool selftest = false;
	for (int i = 1; i < argc; i++)
	{
		if (std::strcmp(argv[i], "--selftest") == 0)
			selftest = true;
	}

	const std::string raiz = projectRoot();
	std::cout << "[projeto] raiz: " << raiz << std::endl;

	// --- verificacoes que nao precisam de janela ---
	int falhas = 0;
	falhas += verificarConvencaoDeAngulo();
	falhas += collisionSelfTest();
	if (falhas > 0)
		std::cerr << "[selftest] ATENCAO: " << falhas << " verificacao(oes) falharam" << std::endl;

	Config cfg;
	loadConfig(raiz + "/config/game.cfg", cfg);

	GLFWwindow *janela = createWindow(cfg.windowWidth, cfg.windowHeight, "Voo Rasante");
	if (!janela)
		return 1;

	Renderer renderer;
	if (!initRenderer(renderer, raiz, cfg.windowWidth, cfg.windowHeight))
	{
		std::cerr << "[main] falha ao inicializar o renderizador" << std::endl;
		glfwTerminate();
		return 1;
	}

	World mundo;
	initWorld(mundo, cfg);
	std::cout << "[mundo] " << mundo.mapa.largura << "x" << mundo.mapa.altura
			  << " tiles, mundo de " << worldSize(mundo.mapa) << " unidades, "
			  << mundo.inimigosVivos << " tanques" << std::endl;

	std::cout << "\nControles:\n"
			  << "  setas ou WASD   girar o nariz e ajustar a potencia\n"
			  << "  ESPACO          metralhadora (municao infinita)\n"
			  << "  B               bomba (cai onde o aviao estava)\n"
			  << "  H R F12 ESC     hitboxes, reiniciar, screenshot, sair\n\n";

	InputReader leitor;

	double prev_s = glfwGetTime();
	double contadorTitulo = 0.0;
	double contadorConsole = 0.0;
	bool f12Anterior = false;
	int frames = 0;

	while (!glfwWindowShouldClose(janela))
	{
		// --- delta time, calculado UMA vez e usado por todo mundo ---
		double agora = glfwGetTime();
		float dt = (float)(agora - prev_s);
		prev_s = agora;

		// O primeiro frame inclui o tempo de criacao da janela e pode passar
		// de 0,3 s, o que teleportaria o aviao. O limite evita isso e tambem
		// protege contra a janela ser arrastada ou o app ficar suspenso.
		if (dt > 0.05f)
			dt = 0.05f;

		glfwPollEvents();

		glfwGetFramebufferSize(janela, &renderer.fbW, &renderer.fbH);

		if (glfwGetKey(janela, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(janela, GLFW_TRUE);

		// --- 1. entrada ---
		InputState input = readInput(leitor, janela);

		// --- 2. simulacao. Nenhuma chamada de OpenGL acontece aqui dentro ---
		updateWorld(mundo, input, dt);

		// --- 3. desenho. O mundo entra como const: nao da para alterar o jogo ---
		renderFrame(renderer, mundo);

		glfwSwapBuffers(janela);
		frames++;

		// Screenshot em F12, com deteccao de borda
		bool f12 = glfwGetKey(janela, GLFW_KEY_F12) == GLFW_PRESS;
		if (f12 && !f12Anterior)
			saveScreenshot(janela, raiz + "/screenshot.png");
		f12Anterior = f12;

		// Titulo da janela: FPS, estado do aviao e situacao do jogo.
		// O heading no titulo e o que torna visivel, durante o jogo, se a
		// convencao de angulo esta sendo aplicada de forma coerente.
		contadorTitulo -= dt;
		if (contadorTitulo <= 0.0)
		{
			char tmp[320];
			float graus = mundo.jogador.angle * 180.0f / 3.14159265f;

			const char *estado = mundo.gameOver ? (mundo.venceu ? "VITORIA (R reinicia)"
																: "DERROTA (R reinicia)")
												: "";

			Vec2 f = forwardFromAngle(mundo.jogador.angle);
			std::snprintf(tmp, sizeof(tmp),
						  "Voo Rasante | FPS %.0f | dir %+.0f graus (%+.2f,%+.2f) | vel %.0f | "
						  "pontos %d | bombas %d | tanques %d %s",
						  dt > 0.0f ? 1.0f / dt : 0.0f, graus, f.x, f.y,
						  mundo.jogador.speed, mundo.score, mundo.jogador.bombs,
						  mundo.inimigosVivos, estado);
			glfwSetWindowTitle(janela, tmp);
			contadorTitulo = 0.12;
		}

		// Diagnostico no console uma vez por segundo. O numero de tiles pega
		// culling esquecido; os contadores de entidades pegam vazamento na
		// remocao por swap-and-pop.
		contadorConsole -= dt;
		if (contadorConsole <= 0.0)
		{
			std::printf("[frame] tiles=%d quads=%d entidades=%zu tanques=%d pontos=%d\n",
						renderer.tilesDesenhados, renderer.quadsDesenhados,
						mundo.entidades.size(), mundo.inimigosVivos, mundo.score);
			std::fflush(stdout);
			contadorConsole = 1.0;
		}

		// No modo de teste, renderiza um frame, grava a imagem e sai.
		if (selftest && frames >= 2)
		{
			saveScreenshot(janela, raiz + "/screenshot.png");
			break;
		}
	}

	destroyRenderer(renderer);
	glfwDestroyWindow(janela);
	glfwTerminate();

	std::cout << "[main] encerrado. Pontuacao final: " << mundo.score << std::endl;
	return falhas > 0 ? 2 : 0;
}
