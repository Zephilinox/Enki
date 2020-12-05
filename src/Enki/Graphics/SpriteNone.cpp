#include "SpriteNone.hpp"

namespace enki
{

SpriteNone::SpriteNone()
	: Sprite(type)
{
}

void SpriteNone::setTexture(Texture* texture)
{
	this->texture = texture;
}

void SpriteNone::setPosition(float x, float y)
{
	position = {x, y};
}

Vector2 SpriteNone::getPosition() const
{
	return position;
}

void SpriteNone::setRotation(float rotation)
{
	this->rotation = rotation;
}

float SpriteNone::getRotation() const
{
	return rotation;
}

Colour SpriteNone::getColour() const
{
	return colour;
}

void SpriteNone::setColour(Colour colour)
{
	this->colour = colour;
}

}	 // namespace enki