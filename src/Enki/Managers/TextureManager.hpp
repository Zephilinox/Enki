#pragma once

//SELF
#include "Enki/Graphics/Texture.hpp"
#include "Enki/Renderer/Renderer.hpp"

//STD
#include <map>
#include <string>

namespace enki
{

class TextureManager
{
public:
	void registerTexture(Renderer* renderer, const std::string& path);
	Texture* getTexture(const std::string& path);
	bool textureExists(const std::string& path);

private:
	std::map<std::string, std::unique_ptr<Texture>> textures;
};

}	// namespace enki