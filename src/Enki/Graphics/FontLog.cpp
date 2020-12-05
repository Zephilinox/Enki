#include "FontLog.hpp"

//SELF
#include "Enki/Graphics/FontNone.hpp"

namespace enki
{

FontLog::FontLog()
	: FontLog(std::make_unique<FontNone>())
{
	
}

FontLog::FontLog(std::unique_ptr<Font> font)
	: Font(type)
	, font(std::move(font))
{
}

bool FontLog::loadFromFile(const std::string& file)
{
	const auto success = font->loadFromFile(file);
	spdlog::info("loadFromFile = {}, {}", file, success);
	return success;
}

}