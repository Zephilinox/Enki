#include "TextureLog.hpp"

//SELF
#include "TextureNone.hpp"

enki::TextureLog::TextureLog()
	: TextureLog(std::make_unique<TextureNone>())
{
}

enki::TextureLog::TextureLog(std::unique_ptr<Texture> texture)
	: Texture(type)
	, texture(std::move(texture))
{
}

bool enki::TextureLog::loadFromFile(const std::string& file)
{
	const auto success = texture->loadFromFile(file);
	spdlog::info("loadFromFile = {}, {}", file, success);
	return success;
}

unsigned int enki::TextureLog::getHeight() const
{
	const auto height = texture->getHeight();
	spdlog::info("getHeight = {}", height);
	return height;
}

unsigned int enki::TextureLog::getWidth() const
{
	const auto width = texture->getWidth();
	spdlog::info("getWidth = {}", width);
	return width;
}
