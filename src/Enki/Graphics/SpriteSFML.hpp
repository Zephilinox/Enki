#pragma once

//LIBS
#include <SFML/Graphics/Sprite.hpp>

//SELF
#include "Enki/Graphics/Sprite.hpp"

namespace enki
{
class SpriteSFML final : public Sprite
{
public:
	SpriteSFML();

	void setTexture(Texture* texture) final;

	void setPosition(float x, float y) final;
	[[nodiscard]] Vector2 getPosition() const final;

	void setRotation(float rotation) final;
	[[nodiscard]] float getRotation() const final;

	[[nodiscard]] Colour getColour() const final;
	void setColour(Colour colour) final;
	
	[[nodiscard]] sf::Sprite& getRawSprite();
	[[nodiscard]] const sf::Sprite& getRawSprite() const;

	static constexpr HashedID type = hash_constexpr("SFML");

private:
	sf::Sprite sprite;
};

}	 // namespace enki
