#pragma once

//STD
#include <map>
#include <string>

//LIBS
#include <SFML/Graphics.hpp>

namespace enki
{
class TextureManager
{
public:
	void registerTexture(const std::string& path);
	sf::Texture* getTexture(const std::string& path);
	bool textureExists(const std::string& path);

private:
	std::map<std::string, std::unique_ptr<sf::Texture>> textures;
};
}	// namespace enki