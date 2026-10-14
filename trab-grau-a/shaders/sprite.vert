#version 400 core

// Unico vertex shader do jogo.
//
// A geometria enviada e SEMPRE o mesmo quad unitario centrado na origem,
// de (-0.5,-0.5) a (0.5,0.5). Posicao, giro e tamanho de cada objeto vem da
// matriz model. Qual pedaco da folha de sprite usar vem de uvOffset/uvScale.
// Isso permite desenhar o mapa, os avioes, o HUD e o radar com um unico VAO.

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;

uniform mat4 projection; // mundo -> tela, enviada uma vez
uniform mat4 view;       // camera, enviada uma vez por grupo de desenho
uniform mat4 model;      // objeto, enviada a cada desenho

uniform vec2 uvOffset;   // canto superior esquerdo do tile dentro da folha
uniform vec2 uvScale;    // tamanho do tile dentro da folha

out vec2 vUV;

void main()
{
	vUV = aUV * uvScale + uvOffset;
	gl_Position = projection * view * model * vec4(aPos, 0.0, 1.0);
}
