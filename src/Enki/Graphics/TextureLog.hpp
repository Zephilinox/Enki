#pragma once

//SELF
#include "Enki/Graphics/Texture.hpp"

namespace enki
{

class TextureLog final : public Texture
{
public:
	TextureLog();
	TextureLog(std::unique_ptr<Texture> texture);

	bool loadFromFile(const std::string& file) final;

	[[nodiscard]] unsigned int getHeight() const final;
	[[nodiscard]] unsigned int getWidth() const final;

	static constexpr HashedID type = hash_constexpr("None");

private:
	std::unique_ptr<Texture> texture;
};

}	 // namespace enki