#include "FontSFML.hpp"

namespace enki
{

FontSFML::FontSFML()
	: Font(type)
{
	
}

bool FontSFML::loadFromFile(const std::string& file)
{
	return font.loadFromFile(file);
}

sf::Font& FontSFML::getRawFont()
{
	return font;
}

const sf::Font& FontSFML::getRawFont() const
{
	return font;
}

}	 // namespace enki
