#pragma once

#include <string>

// Todos os numeros ajustaveis do jogo num lugar so, carregados de
// config/game.cfg. Os valores abaixo sao os padroes: se o arquivo nao existir
// ou faltar uma chave, o jogo roda assim mesmo.
struct Config
{
	// Janela
	int windowWidth = 800;
	int windowHeight = 600;

	// Mundo. E QUADRADO de proposito: o radar tambem e quadrado, e um mundo
	// 4:3 dentro de um radar 1:1 distorceria o mapa ou deixaria borda sobrando.
	int mapTiles = 96;		 // mapa de 96 x 96 tiles
	float tileSize = 32.0f;	 // cada tile ocupa 32 unidades de mundo
	int borderTiles = 2;	 // faixa de agua na borda, onde a camera trava
	unsigned int seed = 20262;

	// Voo do jogador
	float speedMin = 90.0f;	   // unidades por segundo
	float speedMax = 320.0f;
	float speedStart = 160.0f;
	float accel = 170.0f;	   // unidades por segundo ao quadrado
	float turnRate = 2.4f;	   // radianos por segundo (~137 graus/s)
	int playerHealth = 5;

	// Metralhadora
	float bulletSpeed = 620.0f;
	float bulletLife = 1.1f;   // segundos ate sumir
	float gunCooldown = 0.09f;
	int bulletDamage = 1;

	// Bombas
	int bombCount = 20;
	float bombFallTime = 0.75f; // tempo ate explodir
	float bombRadius = 90.0f;	// raio de dano da explosao
	float explosionTime = 0.35f;

	// Inimigos
	int enemyCount = 10;
	int enemyHealth = 3;
	float enemyFireCooldown = 2.2f;
	float enemyBulletSpeed = 220.0f;
	float enemyRange = 420.0f;	 // distancia em que o tanque comeca a atirar
	float turretTurnRate = 1.6f;

	// Pontuacao
	int scorePerTank = 100;

	// Radar
	float radarSize = 160.0f;
	float radarMargin = 12.0f;

	// Parallax: fator de rolagem de cada camada em relacao a camera.
	// Menor que 1 = parece distante, maior que 1 = parece perto do observador.
	float parallaxCloudsLow = 1.15f;
	float parallaxCloudsHigh = 1.60f;
	int cloudCount = 60;
};

// Le um arquivo "chave = valor". Linhas vazias e comecadas por # sao ignoradas.
// Devolve false se o arquivo nao abriu, mas cfg continua com os padroes.
bool loadConfig(const std::string &path, Config &cfg);
