#pragma once

//LIBS
#include <SFML/Graphics.hpp>
#include <Enki/Entity.hpp>

class Score : public enki::Entity
{
public:
	Score(enki::EntityInfo info, enki::GameData* game_data);

	void onSpawn(enki::Packet p) final;
	void draw(enki::Renderer* renderer) final;

	void serializeOnConnection(enki::Packet& p) final;
	void deserializeOnConnection(enki::Packet& p) final;

	void increaseScore1();
	void increaseScore2();

private:
	sf::Font font;
	sf::Text score1;
	sf::Text score2;

	int score1_points = 0;
	int score2_points = 0;
};