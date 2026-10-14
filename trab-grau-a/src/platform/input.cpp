#include "platform/input.h"

static bool tecla(GLFWwindow *j, int codigo)
{
	return glfwGetKey(j, codigo) == GLFW_PRESS;
}

InputState readInput(InputReader &leitor, GLFWwindow *janela)
{
	InputState in;

	// --- acoes continuas: lidas por polling a cada frame ---
	in.turnLeft = tecla(janela, GLFW_KEY_A) || tecla(janela, GLFW_KEY_LEFT);
	in.turnRight = tecla(janela, GLFW_KEY_D) || tecla(janela, GLFW_KEY_RIGHT);
	in.throttleUp = tecla(janela, GLFW_KEY_W) || tecla(janela, GLFW_KEY_UP);
	in.throttleDown = tecla(janela, GLFW_KEY_S) || tecla(janela, GLFW_KEY_DOWN);

	// A metralhadora aceita a barra de espaco ou o botao esquerdo do mouse
	in.fireGun = tecla(janela, GLFW_KEY_SPACE) ||
				 glfwGetMouseButton(janela, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

	// --- acoes de uma vez: deteccao de borda ---
	// Sem isto, segurar a tecla gastaria todo o estoque em meio segundo.
	bool bombaAgora = tecla(janela, GLFW_KEY_B) ||
					  glfwGetMouseButton(janela, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
	in.dropBomb = bombaAgora && !leitor.bombaAnterior;
	leitor.bombaAnterior = bombaAgora;

	bool hitboxAgora = tecla(janela, GLFW_KEY_H);
	in.toggleHitboxes = hitboxAgora && !leitor.hitboxAnterior;
	leitor.hitboxAnterior = hitboxAgora;

	bool restartAgora = tecla(janela, GLFW_KEY_R) || tecla(janela, GLFW_KEY_ENTER);
	in.restart = restartAgora && !leitor.restartAnterior;
	leitor.restartAnterior = restartAgora;

	return in;
}
