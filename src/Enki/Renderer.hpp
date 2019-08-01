#pragma once

//LIBS
#include <SFML/Graphics.hpp>

namespace enki
{
	struct SpriteOrderInfo
	{
		sf::Sprite* sprite = nullptr;
		std::uint8_t layer = 0;
		float order = 0;
	};

	class Renderer
	{
	public:
		Renderer(sf::RenderWindow* window);

		void draw(SpriteOrderInfo);
		void draw(sf::Drawable* drawable);
		void end();

	private:
		std::vector<SpriteOrderInfo> sprites;
		sf::RenderWindow* window = nullptr;
	};
}