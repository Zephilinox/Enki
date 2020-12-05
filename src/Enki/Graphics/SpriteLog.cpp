#include "SpriteLog.hpp"

//SELF
#include "SpriteNone.hpp"

namespace enki
{

SpriteLog::SpriteLog()
	: SpriteLog(std::make_unique<SpriteNone>())
{
}

SpriteLog::SpriteLog(std::unique_ptr<Sprite> sprite)
	: Sprite(type)
	, sprite(std::move(sprite))
{
}

void SpriteLog::setTexture(Texture* texture)
{
	sprite->setTexture(texture);
	spdlog::info("setTexture = {}", texture->getType());
}

void SpriteLog::setPosition(float x, float y)
{
	sprite->setPosition(x, y);
	spdlog::info("setPosition = {{{}, {}}}", x, y);
}

Vector2 SpriteLog::getPosition() const
{
	const auto pos = sprite->getPosition();
	spdlog::info("getPosition = {{{}, {}}}", pos.x, pos.y);
	return pos;
}

void SpriteLog::setRotation(float rotation)
{
	sprite->setRotation(rotation);
	spdlog::info("setRotation = {}", rotation);
}

float SpriteLog::getRotation() const
{
	const auto rot = sprite->getRotation();
	spdlog::info("getRotation = {}", rot);
	return rot;
}

Colour SpriteLog::getColour() const
{
	const auto colour = sprite->getColour();
	spdlog::info("getColour = {{{}, {}, {}, {}}}", colour.r, colour.g, colour.b, colour.a);
	return colour;
}

void SpriteLog::setColour(Colour colour)
{
	sprite->setColour(colour);
	spdlog::info("setColour = {{}, {}, {}, {}}}", colour.r, colour.g, colour.b, colour.a);
}

}	 // namespace enki