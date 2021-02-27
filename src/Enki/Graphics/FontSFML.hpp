#pragma once

//SELF
#include "Enki/Graphics/Font.hpp"

//LIBS
#include <SFML/Graphics/Font.hpp>

namespace enki
{

class FontSFML final : public Font
{
public:
	FontSFML();

	bool loadFromFile(const std::string& file) final;
	
	static constexpr HashedID type = hash_constexpr("SFML");

	[[nodiscard]] const sf::Font& getRawFont() const;
	[[nodiscard]] sf::Font& getRawFont();

private:
	sf::Font font;
};

}	 // namespace enki