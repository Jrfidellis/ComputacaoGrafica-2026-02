#include <iostream>
#include <string>
#include <assert.h>

/*
 * Lista 1 - Exercício 3
 *
 * Triângulo formado pelos vértices P1, P2 e P3, respectivamente com as cores
 * vermelho, verde e azul.
 *
 * a) Configuração dos buffers (VBO, VAO)
 *
 *    Cada vértice passa a ter DOIS atributos: posição (x, y, z) e cor (r, g, b).
 *    Uma configuração possível é usar UM ÚNICO VBO com os atributos intercalados
 *    (interleaved), ou seja, 6 floats por vértice:
 *
 *        | x  y  z | r  g  b |  <- P1 (vermelho)
 *        | x  y  z | r  g  b |  <- P2 (verde)
 *        | x  y  z | r  g  b |  <- P3 (azul)
 *
 *    O VAO guarda dois "ponteiros de atributo" apontando para esse mesmo VBO:
 *
 *        atributo 0 (posição): 3 floats, stride = 6 * sizeof(float), offset = 0
 *        atributo 1 (cor):     3 floats, stride = 6 * sizeof(float), offset = 3 * sizeof(float)
 *
 *    O stride diz quantos bytes pular para chegar no mesmo atributo do próximo
 *    vértice; o offset diz em que byte o atributo começa dentro de cada vértice.
 *
 *    Alternativa: dois VBOs separados (um só com posições, outro só com cores),
 *    cada um com stride = 3 * sizeof(float) e offset 0, ambos registrados no
 *    mesmo VAO nas locations 0 e 1. O resultado no shader é idêntico.
 *
 * b) Identificação no vertex shader
 *
 *    Cada atributo é uma variável de entrada ("in") com o layout location
 *    correspondente ao índice usado em glVertexAttribPointer:
 *
 *        layout (location = 0) in vec3 position;
 *        layout (location = 1) in vec3 color;
 *
 *    A cor não é usada pelo vertex shader para posicionar nada; ela é apenas
 *    repassada para o fragment shader por uma variável "out". O rasterizador
 *    interpola essa cor entre os três vértices, por isso o interior do
 *    triângulo aparece com um gradiente vermelho -> verde -> azul.
 */

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader();
int setupGeometry();

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 800, HEIGHT = 600;

// Código fonte do Vertex Shader (em GLSL)
// Agora recebe 2 atributos por vértice: posição (location 0) e cor (location 1).
// A cor é repassada ao fragment shader pela variável de saída vertexColor.
const GLchar *vertexShaderSource = R"glsl(
 #version 400
 layout (location = 0) in vec3 position;
 layout (location = 1) in vec3 color;

 out vec4 vertexColor;

 void main()
 {
	 gl_Position = vec4(position.x, position.y, position.z, 1.0);
	 vertexColor = vec4(color, 1.0);
 }
 )glsl";

// Código fonte do Fragment Shader (em GLSL)
// Não usa mais uniform: a cor chega interpolada do vertex shader.
const GLchar *fragmentShaderSource = R"glsl(
 #version 400
 in vec4 vertexColor;
 out vec4 color;
 void main()
 {
	 color = vertexColor;
 }
 )glsl";

// Função MAIN
int main()
{
	// Inicialização da GLFW
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Criação da janela GLFW
	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Exercicio 3 - Triangulo com cores por vertice", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "Falha ao criar a janela GLFW" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);

	// GLAD: carrega todos os ponteiros de funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Falha ao inicializar GLAD" << std::endl;
		return -1;
	}

	// Obtendo as informações de versão
	const GLubyte *renderer = glGetString(GL_RENDERER);
	const GLubyte *version = glGetString(GL_VERSION);
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// Compilando e buildando o programa de shader
	GLuint shaderID = setupShader();

	// Gerando os buffers com a geometria e as cores do triângulo
	GLuint VAO = setupGeometry();

	glUseProgram(shaderID);

	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // cor de fundo
		glClear(GL_COLOR_BUFFER_BIT);

		glPointSize(20);

		glBindVertexArray(VAO); // Conectando ao buffer de geometria

		// Triângulo preenchido: a cor de cada pixel é interpolada entre P1, P2 e P3
		glDrawArrays(GL_TRIANGLES, 0, 3);

		// Pontos sobre os vértices, para evidenciar a cor "pura" de cada um
		// (P1 vermelho, P2 verde, P3 azul). Usa o mesmo VAO e o mesmo shader.
		glDrawArrays(GL_POINTS, 0, 3);

		glBindVertexArray(0);

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
	// Pede pra OpenGL desalocar os buffers
	glDeleteVertexArrays(1, &VAO);
	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

// Função de callback de teclado
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

// Compila e builda o programa de shader (idêntico ao dos exercícios anteriores)
int setupShader()
{
	// Vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// Checando erros de compilação (exibição via log no terminal)
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}
	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// Checando erros de compilação (exibição via log no terminal)
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}
	// Linkando os shaders e criando o identificador do programa de shader
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// Checando por erros de linkagem
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
				  << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

// Cria os buffers que armazenam a geometria E as cores do triângulo
// 1 VBO com os atributos intercalados (posição + cor), VAO com 2 ponteiros para atributo
// A função retorna o identificador do VAO
int setupGeometry()
{
	// Cada linha é um vértice completo: 3 floats de posição seguidos de 3 floats de cor.
	// As coordenadas foram estimadas a partir da figura do enunciado.
	GLfloat vertices[] = {
		//  x      y     z      r     g     b
		 0.0f,  0.6f, 0.0f,  1.0f, 0.0f, 0.0f, // P1 - vermelho
		-0.6f, -0.4f, 0.0f,  0.0f, 1.0f, 0.0f, // P2 - verde
		 0.6f, -0.25f, 0.0f, 0.0f, 0.0f, 1.0f, // P3 - azul
	};

	GLuint VBO, VAO;
	// Geração do identificador do VBO
	glGenBuffers(1, &VBO);
	// Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// Envia os dados do array de floats para o buffer da OpenGL
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);
	// Vincula (bind) o VAO primeiro, e em seguida conecta e seta o(s) buffer(s) de vértices
	// e os ponteiros para os atributos
	glBindVertexArray(VAO);

	// Tamanho em bytes de um vértice completo (posição + cor)
	const GLsizei stride = 6 * sizeof(GLfloat);

	// Atributo 0: posição (x, y, z)
	//   3 valores, tipo float, não normalizado, stride de 6 floats, começa no byte 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid *)0);
	glEnableVertexAttribArray(0);

	// Atributo 1: cor (r, g, b)
	//   3 valores, tipo float, não normalizado, stride de 6 floats,
	//   começa depois dos 3 floats da posição (offset de 3 * sizeof(GLfloat) bytes)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// A chamada para glVertexAttribPointer registrou o VBO como o buffer vinculado
	// ao atributo, então podemos desvincular com segurança
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (boa prática para evitar alterações acidentais)
	glBindVertexArray(0);

	return VAO;
}
