#include "core/config.h"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

// Parser minimo de "chave = valor". Nao usa biblioteca de JSON de proposito:
// sao ~60 linhas, nao adiciona dependencia e e facil de explicar.

static std::string trim(const std::string &s)
{
	size_t a = s.find_first_not_of(" \t\r\n");
	if (a == std::string::npos)
		return "";
	size_t b = s.find_last_not_of(" \t\r\n");
	return s.substr(a, b - a + 1);
}

bool loadConfig(const std::string &path, Config &cfg)
{
	std::ifstream f(path);
	if (!f.is_open())
	{
		std::cerr << "[config] nao abriu " << path << ", usando os padroes" << std::endl;
		return false;
	}

	std::map<std::string, std::string> kv;
	std::string linha;
	while (std::getline(f, linha))
	{
		// Corta comentario de fim de linha
		size_t h = linha.find('#');
		if (h != std::string::npos)
			linha = linha.substr(0, h);

		size_t eq = linha.find('=');
		if (eq == std::string::npos)
			continue;

		std::string chave = trim(linha.substr(0, eq));
		std::string valor = trim(linha.substr(eq + 1));
		if (!chave.empty() && !valor.empty())
			kv[chave] = valor;
	}

	// Helpers de leitura: so sobrescrevem o padrao se a chave existir
	auto lerF = [&](const char *k, float &destino) {
		auto it = kv.find(k);
		if (it != kv.end())
			destino = std::stof(it->second);
	};
	auto lerI = [&](const char *k, int &destino) {
		auto it = kv.find(k);
		if (it != kv.end())
			destino = std::stoi(it->second);
	};
	auto lerU = [&](const char *k, unsigned int &destino) {
		auto it = kv.find(k);
		if (it != kv.end())
			destino = (unsigned int)std::stoul(it->second);
	};

	lerI("window_width", cfg.windowWidth);
	lerI("window_height", cfg.windowHeight);

	lerI("map_tiles", cfg.mapTiles);
	lerF("tile_size", cfg.tileSize);
	lerI("border_tiles", cfg.borderTiles);
	lerU("seed", cfg.seed);

	lerF("speed_min", cfg.speedMin);
	lerF("speed_max", cfg.speedMax);
	lerF("speed_start", cfg.speedStart);
	lerF("accel", cfg.accel);
	lerF("turn_rate", cfg.turnRate);
	lerI("player_health", cfg.playerHealth);

	lerF("bullet_speed", cfg.bulletSpeed);
	lerF("bullet_life", cfg.bulletLife);
	lerF("gun_cooldown", cfg.gunCooldown);
	lerI("bullet_damage", cfg.bulletDamage);

	lerI("bomb_count", cfg.bombCount);
	lerF("bomb_fall_time", cfg.bombFallTime);
	lerF("bomb_radius", cfg.bombRadius);
	lerF("explosion_time", cfg.explosionTime);

	lerI("enemy_count", cfg.enemyCount);
	lerI("enemy_health", cfg.enemyHealth);
	lerF("enemy_fire_cooldown", cfg.enemyFireCooldown);
	lerF("enemy_bullet_speed", cfg.enemyBulletSpeed);
	lerF("enemy_range", cfg.enemyRange);
	lerF("turret_turn_rate", cfg.turretTurnRate);

	lerI("score_per_tank", cfg.scorePerTank);

	lerF("radar_size", cfg.radarSize);
	lerF("radar_margin", cfg.radarMargin);

	lerF("parallax_clouds_low", cfg.parallaxCloudsLow);
	lerF("parallax_clouds_high", cfg.parallaxCloudsHigh);
	lerI("cloud_count", cfg.cloudCount);

	std::cout << "[config] " << kv.size() << " chaves lidas de " << path << std::endl;
	return true;
}
