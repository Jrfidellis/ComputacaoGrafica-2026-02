#version 400 core

// Unico fragment shader do jogo.
//
// A cor final e a textura multiplicada pelo tint. Com uma textura branca de
// 1x1 pixel, o tint sozinho pinta um quad solido, e por isso as barras do HUD,
// os pontos do radar, as balas e os contornos de hitbox usam este mesmo
// shader, sem nenhum programa extra.

in vec2 vUV;

uniform sampler2D tex;
uniform vec4 tint;

out vec4 color;

void main()
{
	vec4 amostra = texture(tex, vUV);
	color = amostra * tint;

	// Descarta o que for praticamente transparente. Com GL_NEAREST o alpha e
	// 0 ou 1, mas isto protege as bordas se o filtro mudar.
	if (color.a < 0.01)
		discard;
}
