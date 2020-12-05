#pragma once

//SELF
#include "Enki/Graphics/Font.hpp"

namespace enki
{

class FontLog final : public Font
{
public:
	FontLog();
	FontLog(std::unique_ptr<Font> font);

	bool loadFromFile(const std::string& file) final;
	
	static constexpr HashedID type = hash_constexpr("Log");

private:
	std::unique_ptr<Font> font;
};

}