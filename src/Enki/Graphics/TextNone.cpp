#include "TextNone.hpp"

namespace enki
{

void TextNone::setPosition(float x, float y)
{
	position = {x, y};
}

Vector2 TextNone::getPosition() const
{
	return position;
}

void TextNone::setFillColor(Colour colour)
{
}

void TextNone::setFont(Font* font)
{
}

void TextNone::setString(const std::string& string)
{
}

void TextNone::setCharacterSize(std::uint8_t size)
{
}

}
