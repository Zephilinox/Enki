#pragma once

//LIBS
#include <SFML/Audio.hpp>

//STD
#include <map>
#include <string>
#include <vector>

namespace enki
{
class SoundManager
{
public:
	sf::Sound* playSound(const std::string& path, bool loop = false);
	void registerSound(const std::string& path);
	sf::SoundBuffer* getSoundBuffer(const std::string& path);

private:
	std::vector<std::unique_ptr<sf::Sound>> sounds;
	std::map<std::string, std::unique_ptr<sf::SoundBuffer>> buffers;
};
}	// namespace enki