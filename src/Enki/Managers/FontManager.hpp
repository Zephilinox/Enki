#pragma once

//STD
#include <string>
#include <unordered_map>

//LIBS
#include <spdlog/spdlog.h>
#include <SFML/Graphics.hpp>

//SELF
#include "Enki/Hash.hpp"

namespace enki
{
class FontManager
{
public:
	sf::Font* registerFont(const std::string& path, HashedID name);

	[[nodiscard]] sf::Font* getFont(HashedID name) const;

	[[nodiscard]] std::string getFontPath(HashedID name) const;

	template <size_t N>
	sf::Font* registerFont(const std::string& path, const char (&name)[N]);

	template <size_t N>
	[[nodiscard]] sf::Font* getFont(const char (&name)[N]) const;

	template <size_t N>
	[[nodiscard]] std::string getFontPath(const char (&name)[N]) const;

private:
	std::unordered_map<HashedID, std::unique_ptr<sf::Font>> fonts;
	std::unordered_map<HashedID, std::string> font_paths;
	std::unordered_map<HashedID, std::string> font_names;
};

template <size_t N>
sf::Font* FontManager::registerFont(const std::string& path, const char (&name)[N])
{
	auto font = registerFont(std::forward<const std::string&>(path), hash(name));

	if (font)
	{
		font_names[hash(name)] = name;
	}
	else
	{
		auto font_name_it = font_names.find(hash(name));

		if (font_name_it != font_names.end())	//hash matches font that already exists
		{
			if (font_name_it->second != name)	//name passed through doesn't match existing name, so it must be a collision)
			{
				spdlog::critical(
					"Font name {} has not been used before, collision has occured.",
					font_name_it->second);
			}
			else
			{
				spdlog::error(
					"Font name {} has been used before, not a collision.",
					font_name_it->second);
			}
		}
		else	//the font name hasn't been added before due to mixing this call vs the other
		{
			spdlog::error(
				"Could not provide more information regarding font failure, please call this version of the function. {}, {}, {}",
				font_name_it->second,
				path,
				hash(name));
		}
	}

	return font;
}

template <size_t N>
sf::Font* FontManager::getFont(const char (&name)[N]) const
{
	auto font = getFont(hash(name));

	if (!font)
	{
		auto font_name_it = font_names.find(hash(name));
		if (font_name_it == font_names.end())
		{
			spdlog::warn(
				"Font named {} with hash {} could not be found, either because this template function was not used to add it, or because it hasn't been registered at all.",
				name,
				hash(name));
		}
		else
		{
			spdlog::error("Unknown error occured in getting the font, investigate. {}, {}", name, hash(name));
		}
	}

	return font;
}

template <size_t N>
std::string FontManager::getFontPath(const char (&name)[N]) const
{
	auto path = getFontPath(hash(name));

	if (path.empty())
	{
		auto font_name_it = font_names.find(hash(name));
		if (font_name_it == font_names.end())
		{
			spdlog::warn(
				"Font path from the font named {} with hash {} could not be found, either because this template function was not used to add it, or because it hasn't been registered at all.",
				name,
				hash(name));
		}
		else
		{
			spdlog::error("Unknown error occured in getting the font path, investigate. {}, {}", name, hash(name));
		}
	}

	return path;
}
}	// namespace enki