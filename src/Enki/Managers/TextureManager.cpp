#include "TextureManager.hpp"

namespace enki
{
void TextureManager::registerTexture(Renderer* renderer, const std::string& path)
{
	if (textures.count(path))
		return;
	
	textures[path] = renderer->createTexture();
	textures[path]->loadFromFile(path);
}

Texture* TextureManager::getTexture(const std::string& path)
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
