#pragma once

//STD
#include <string>
#include <map>

//LIBS
#include <SFML/Graphics.hpp>

namespace enki
{
	class FontManager
	{
	public:
		void registerFont(const std::string& path);
		sf::Font* getFont(const std::string& path);

	private:
		std::map<std::string, std::unique_ptr<sf::Font>> fonts;
	};
}