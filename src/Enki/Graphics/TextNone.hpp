#pragma once

//SELF
#include "Enki/Graphics/Text.hpp"

namespace enki
{
class TextNone final : public Text
{
public:
	TextNone();

	void setPosition(float x, float y) final;
	[[nodiscard]] Vector2 getPosition() const final;

	void setFillColor(Colour colour) final;
	void setFont(Font* font) final;
	void setString(const std::string& string) final;
	void setCharacterSize(std::uint8_t size) final;

	static constexpr HashedID type = hash_constexpr("None");

private:
	Vector2 position;
};

}	 // namespace enki