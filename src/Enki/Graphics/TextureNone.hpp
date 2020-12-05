#pragma once

//SELF
#include "Enki/Graphics/Texture.hpp"

namespace enki
{

class TextureNone final : public Texture
{
public:
	TextureNone();
	
	bool loadFromFile(const std::string& file) final;

	[[nodiscard]] unsigned int getHeight() const final;
	[[nodiscard]] unsigned int getWidth() const final;

	static constexpr HashedID type = hash_constexpr("None");

private:
	//todo: will need to load from file to get accurate dimensions
};

}