#include "TextureSFML.hpp"

enki::TextureSFML::TextureSFML()
	: Texture(type)
{
}

bool enki::TextureSFML::loadFromFile(const std::string& file)
{
	return texture.loadFromFile(file);
}

unsigned int enki::TextureSFML::getHeight() const
{
	return texture.getSize().y;
}

unsigned int enki::TextureSFML::getWidth() const
{
	return texture.getSize().x;
}

sf::Texture& enki::TextureSFML::getRawTexture()
{
	return texture;
}

const sf::Texture& enki::TextureSFML::getRawTexture() const
{
	return texture;
}
