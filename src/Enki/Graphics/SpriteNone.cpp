#include "SpriteNone.hpp"

namespace enki
{

SpriteNone::SpriteNone()
	: Sprite(type)
{
}

void SpriteNone::setTexture(Texture* texture)
{
	m_texture = texture;
}

void SpriteNone::setPosition(float x, float y)
{
	m_position = {x, y};
}

Vector2 SpriteNone::getPosition() const
{
	return m_position;
}

void SpriteNone::setRotation(float rotation)
{
	m_rotation = rotation;
}

float SpriteNone::getRotation() const
{
	return m_rotation;
}

Colour SpriteNone::getColour() const
{
	return m_colour;
}

void SpriteNone::setColour(Colour colour)
{
	m_colour = colour;
}

}	 // namespace enki