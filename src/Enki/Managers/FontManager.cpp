#include "FontManager.hpp"

namespace enki
{
	void FontManager::registerFont(const std::string& path)
	{
		fonts[path] = std::make_unique<sf::Font>();
		fonts[path]->loadFromFile(path);
	}

	sf::Font* FontManager::getFont(const std::string& path)
	{
		if (fonts.count(path))
		{
			return fonts[path].get();
		}

		return nullptr;
	}
}