#include "Score.hpp"

Score::Score(enki::EntityInfo info, enki::GameData* game_data)
	: Entity(info, game_data)
{
	if (!font.loadFromFile("resources/arial.ttf"))
	{
		spdlog::get("console")->error("Failed to load font");
	}
}

void Score::onSpawn([[maybe_unused]]enki::Packet& p)
{
	score1.setFont(font);
	score1.setCharacterSize(30);
	score1.setFillColor(sf::Color(0, 150, 255));
	score1.setPosition(320 - 60, 180);
	score1.setString("0");

	score2.setFont(font);
	score2.setCharacterSize(30);
	score2.setFillColor(sf::Color(220, 25, 25));
	score2.setPosition(320 + 60, 180);
	score2.setString("0");
}

void Score::draw(enki::Renderer* renderer)
{
	renderer->draw(&score1);
	renderer->draw(&score2);
}

void Score::serializeOnConnection(enki::Packet& p)
{
	p << score1_points << score2_points;
}

void Score::deserializeOnConnection(enki::Packet& p)
{
	p >> score1_points >> score2_points;
	score1.setString(std::to_string(score1_points));
	score2.setString(std::to_string(score2_points));
}

void Score::increaseScore1()
{
	score1_points++;
	score1.setString(std::to_string(score1_points));
}

void Score::increaseScore2()
{
	score2_points++;
	score2.setString(std::to_string(score2_points));
}
