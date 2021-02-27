#pragma once

//SELF
#include "Enki/Graphics/Texture.hpp"

//LIBS
#include <SFML/Graphics/Texture.hpp>

namespace enki
{

class TextureSFML final : public Texture
{
public:
	TextureSFML();
	
	bool loadFromFile(const std::string& file) final;

	[[nodiscard]] unsigned int getHeight() const final;
	[[nodiscard]] unsigned int getWidth() const final;

	static constexpr HashedID type = hash_constexpr("SFML");
	
	[[nodiscard]] sf::Texture& getRawTexture();
	[[nodiscard]] const sf::Texture& getRawTexture() const;

private:
	sf::Texture texture;
};

}	 // namespace enki
