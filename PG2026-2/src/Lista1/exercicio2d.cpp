#include <iostream>
#include <string>
#include <assert.h>
#include <vector>
#include <cmath>

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

// Quantidade de segmentos usados para aproximar o arco (a "casca") da fatia
// Quanto maior, mais "redondo" o arco fica
const int N_SEGMENTS = 32;

// Ângulo de abertura da fatia de pizza (em graus)
const float SLICE_ANGLE_DEG = 60.0f;

// Total de vértices do triangle fan: centro (ponta da fatia) + N_SEGMENTS + 1 pontos do arco
const int N_VERTICES = N_SEGMENTS + 2;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar *vertexShaderSource = R"glsl(
 #version 400
 layout (location = 0) in vec3 position;
 void main()
 {
	 gl_Position = vec4(position.x, position.y, position.z, 1.0);
 }
 )glsl";

// Código fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar *fragmentShaderSource = R"glsl(
 #version 400
 uniform vec4 inputColor;
 out vec4 color;
 void main()
 {
	 color = inputColor;
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
	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Fatia de Pizza - Equacao Parametrica", nullptr, nullptr);
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

	// Gerando os buffers com a geometria do círculo
	GLuint VAO = setupGeometry();

	// Localização da variável uniform que guarda a cor
	GLint colorLoc = glGetUniformLocation(shaderID, "inputColor");

	glUseProgram(shaderID);

	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // cor de fundo
		glClear(GL_COLOR_BUFFER_BIT);

		glBindVertexArray(VAO); // Conectando ao buffer de geometria

		// Fatia preenchida em cor de queijo
		glUniform4f(colorLoc, 0.95f, 0.75f, 0.25f, 1.0f);
		glDrawArrays(GL_TRIANGLE_FAN, 0, N_VERTICES);

		// Contorno em marrom (inclui o centro, que é a ponta da fatia)
		glUniform4f(colorLoc, 0.45f, 0.25f, 0.05f, 1.0f);
		glDrawArrays(GL_LINE_LOOP, 0, N_VERTICES);

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

// Cria os buffers que armazenam a geometria da fatia de pizza
// Os vértices são gerados pela equação paramétrica do círculo:
//   x = cx + r * cos(θ)
//   y = cy + r * sin(θ)
// mas θ varia apenas dentro do ângulo da fatia (SLICE_ANGLE_DEG),
// centrado em 90° para a fatia apontar para cima
// Os pontos são organizados como um GL_TRIANGLE_FAN: o primeiro vértice é o
// centro (a ponta da fatia) e os demais percorrem o arco (a "casca")
int setupGeometry()
{
	const float PI = 3.14159265358979f;
	const float cx = 0.0f;	 // ponta da fatia (x)
	const float cy = -0.4f;	 // ponta da fatia (y)
	const float radius = 0.8f;

	// A janela é 800x600, então 1 unidade em x cobre mais pixels que em y.
	// Multiplicamos o x pelo fator height/width para o círculo não virar elipse.
	const float aspectCorrection = (float)HEIGHT / (float)WIDTH;

	vector<GLfloat> vertices;

	// Vértice central (necessário para o triangle fan)
	vertices.push_back(cx);
	vertices.push_back(cy);
	vertices.push_back(0.0f);

	// O arco cobre apenas o ângulo da fatia, centrado em 90° (apontando para cima)
	const float sliceHalf = (SLICE_ANGLE_DEG * PI / 180.0f) / 2.0f;
	const float thetaStart = PI / 2.0f - sliceHalf;
	const float thetaEnd = PI / 2.0f + sliceHalf;

	for (int i = 0; i <= N_SEGMENTS; i++)
	{
		float theta = thetaStart + (thetaEnd - thetaStart) * (float)i / (float)N_SEGMENTS;
		float x = cx + radius * cos(theta) * aspectCorrection;
		float y = cy + radius * sin(theta);
		vertices.push_back(x);
		vertices.push_back(y);
		vertices.push_back(0.0f);
	}

	GLuint VBO, VAO;
	// Geração do identificador do VBO
	glGenBuffers(1, &VBO);
	// Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// Envia os dados do vetor de floats para o buffer da OpenGL
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

	// Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	// Atributo 0: posição (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return VAO;
}
