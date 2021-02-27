#include "TextureNone.hpp"

namespace enki
{

TextureNone::TextureNone()
	: Texture(type)
{
}

bool TextureNone::loadFromFile(const std::string&)
{
	return false;
}

unsigned int TextureNone::getHeight() const
{
	return 0;
}

unsigned int TextureNone::getWidth() const
{
	return 0;
}

}	 // namespace enki
