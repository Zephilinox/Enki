#include "SpriteSFML.hpp"

//SELF
#include "Enki/Graphics/TextureSFML.hpp"

namespace enki
{

SpriteSFML::SpriteSFML()
	: Sprite(type)
{
}

void SpriteSFML::setTexture(Texture* texture)
{
	auto* t = texture->as<TextureSFML>();
	if (t)
		sprite.setTexture(t->getRawTexture());
	
	sprite.setOrigin(
		t->getRawTexture().getSize().x / 2.0f,
		t->getRawTexture().getSize().y / 2.0f);
}

void SpriteSFML::setPosition(float x, float y)
{
	sprite.setPosition(x, y);
}

Vector2 SpriteSFML::getPosition() const
{
	const auto pos = sprite.getPosition();
	return Vector2{pos.x, pos.y};
}

void SpriteSFML::setRotation(float rotation)
{
	sprite.setRotation(rotation);
}

float SpriteSFML::getRotation() const
{
	return sprite.getRotation();
}

Colour SpriteSFML::getColour() const
{
	const auto c = sprite.getColor();
	return Colour{c.r, c.g, c.b, c.a};
}

void SpriteSFML::setColour(Colour colour)
{
	sprite.setColor({
		static_cast<sf::Uint8>(colour.r),
			static_cast<sf::Uint8>(colour.g),
			static_cast<sf::Uint8>(colour.b),
			static_cast<sf::Uint8>(colour.a),
	});
}

sf::Sprite& SpriteSFML::getRawSprite()
{
	return sprite;
}

const sf::Sprite& SpriteSFML::getRawSprite() const
{
	return sprite;
}

}