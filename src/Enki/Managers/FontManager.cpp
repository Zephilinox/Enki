#include "FontManager.hpp"

namespace enki
{
sf::Font* FontManager::registerFont(const std::string& path, HashedID name)
{
	auto f = std::make_unique<sf::Font>();
	f->loadFromFile(path);

	const auto pair_font = fonts.emplace(name, std::move(f));

	if (!pair_font.second)	//hash exists, failed to insert
	{
		const auto font_path_it = font_paths.find(name);
		if (font_path_it == font_paths.end())	//path doesn't exist
		{
			spdlog::error(
				"Failed to add font at path '{}' because the hash ({}) already exists. There is no path associated with this other font, unknown error, investigate.",
				path,
				name);
			return nullptr;
		}
		else
		{
			if (font_path_it->second == path)
			{
				spdlog::warn(
					"Failed to add font at path '{}' because the hash ({}) already exists. This matches the other font's path '{}'. Likely registering the same font twice.",
					path,
					name,
					font_path_it->second);
			}
			else
			{
				spdlog::error(
					"Failed to add font at path '{}' because the hash ({}) already exists. This does not match the other fonts path '{}'. Possible reuse of font name or hash collision has occured.",
					path,
					name,
					font_path_it->second);
			}

			return nullptr;
		}
	}
	else
	{
		for (const auto& existing_path : font_paths)
		{
			if (existing_path.second == path)
			{
				spdlog::warn(
					"The font at path '{}' already exists with hash {}, and is now also referenced by hash {}",
					path,
					existing_path.first,
					name);
			}
		}

		font_paths.emplace(name, path);
		spdlog::info("The font at path '{}' has been registered using the hash {}", path, name);

		return pair_font.first->second.get();	//pair to iterator to unique pointer to raw pointer
	}
}

sf::Font* FontManager::getFont(HashedID name) const
{
	const auto font_it = fonts.find(name);
	if (font_it != fonts.end())
		return font_it->second.get();
	
	spdlog::warn("Failed to get font with hash {}", name);
	return nullptr;
}

std::string FontManager::getFontPath(HashedID name) const
{
	const auto path_it = font_paths.find(name);
	if (path_it != font_paths.end())
		return path_it->second;
	
	spdlog::warn("Failed to get path with hash {}", name);
	return {};
}
}	// namespace enki