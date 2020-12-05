#include "TextSFML.hpp"

//SELF
#include "Enki/Graphics/FontSFML.hpp"

namespace enki
{

TextSFML::TextSFML()
	: Text(type)
{
}

void TextSFML::setPosition(float x, float y)
{
	text.setPosition(x, y);
}

Vector2 TextSFML::getPosition() const
{
	const auto pos = text.getPosition();
	return {pos.x, pos.y};
}

void TextSFML::setFillColor(Colour colour)
{
	text.setFillColor({colour.r, colour.g, colour.b, colour.a});
}

void TextSFML::setFont(Font* font)
{
	text.setFont(font->as<FontSFML>()->getRawFont());
}

void TextSFML::setString(const std::string& string)
{
	text.setString(string);
}

void TextSFML::setCharacterSize(std::uint8_t size)
{
	text.setCharacterSize(size);
}

sf::Text& TextSFML::getRawText()
{
	return text;
}

const sf::Text& TextSFML::getRawText() const
{
	return text;
}

}