#pragma once

//SELF
#include "Enki/Graphics/Sprite.hpp"

namespace enki
{

class SpriteLog final : public Sprite
{
public:
	SpriteLog();
	SpriteLog(std::unique_ptr<Sprite> sprite);

	void setTexture(Texture* texture) final;

	void setPosition(float x, float y) final;
	[[nodiscard]] virtual Vector2 getPosition() const final;

	void setRotation(float rotation) final;
	[[nodiscard]] virtual float getRotation() const final;

	[[nodiscard]] virtual Colour getColour() const final;
	void setColour(Colour colour) final;

	static constexpr HashedID type = hash_constexpr("Log");

private:
	std::unique_ptr<Sprite> sprite;
};

}	 // namespace enki