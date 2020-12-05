#include "TextLog.hpp"

//SELF
#include "Enki/Graphics/TextNone.hpp"

namespace enki
{

TextLog::TextLog()
	: TextLog(std::make_unique<TextNone>())
{
}

TextLog::TextLog(std::unique_ptr<Text> text)
	: Text(type)
	, text(std::move(text))
{
}

void TextLog::setPosition(float x, float y)
{
	text->setPosition(x, y);
	spdlog::info("setPosition = {{{}, {}}}", x, y);
}

Vector2 TextLog::getPosition() const
{
	const auto pos = text->getPosition();
	spdlog::info("getPosition = {{{}, {}}}", pos.x, pos.y);
	return pos;
}

void TextLog::setFillColor(Colour colour)
{
	text->setFillColor(colour);
	spdlog::info("setColour = {{}, {}, {}, {}}}", colour.r, colour.g, colour.b, colour.a);
}

void TextLog::setFont(Font* font)
{
	text->setFont(font);
	spdlog::info("setFont = {}", font->getType());
}

void TextLog::setString(const std::string& string)
{
	text->setString(string);
	spdlog::info("setString = {}", string);
}

void TextLog::setCharacterSize(std::uint8_t size)
{
	text->setCharacterSize(size);
	spdlog::info("setCharacterSize = {}", size);
}

}	 // namespace enki
