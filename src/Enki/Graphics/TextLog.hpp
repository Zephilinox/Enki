#pragma once

//SELF
#include "Enki/Graphics/Text.hpp"

namespace enki
{
class TextLog final : public Text
{
public:
	TextLog();
	TextLog(std::unique_ptr<Text> text);

	void setPosition(float x, float y) final;
	[[nodiscard]] Vector2 getPosition() const final;

	void setFillColor(Colour colour) final;
	void setFont(Font* font) final;
	void setString(const std::string& string) final;
	void setCharacterSize(std::uint8_t size) final;

	static constexpr HashedID type = hash_constexpr("Log");

private:
	std::unique_ptr<Text> text;
};

}	 // namespace enki