#pragma once

//LIBS
#include <SFML/Graphics/Text.hpp>

//SELF
#include "Enki/Graphics/Text.hpp"

namespace enki
{

class TextSFML final : public Text
{
public:
	TextSFML();
	
	void setPosition(float x, float y) final;
	[[nodiscard]] Vector2 getPosition() const final;
	
	void setFillColor(Colour colour) final;
	void setFont(Font* font) final;
	void setString(const std::string& string) final;
	void setCharacterSize(std::uint8_t size) final;
	
	static constexpr HashedID type = hash_constexpr("SFML");

	sf::Text& getRawText();
	const sf::Text& getRawText() const;
	
private:
	sf::Text text;
};

}