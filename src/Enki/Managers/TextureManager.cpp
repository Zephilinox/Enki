#include "TextureManager.hpp"

namespace enki
{
void TextureManager::registerTexture(const std::string& path)
{
	textures[path] = std::make_unique<sf::Texture>();
	textures[path]->loadFromFile(path);
}

sf::Texture* TextureManager::getTexture(const std::string& path)
{
	if (textures.count(path))
		return textures[path].get();

	return nullptr;
}

bool TextureManager::textureExists(const std::string& path)
{
	return textures.find(path) != textures.end();
}
}
