#include "SoundManager.hpp"

//STD
#include <vector>

namespace enki
{
sf::Sound* SoundManager::playSound(const std::string& path, bool loop)
{
	//remove any old sounds
	std::erase_if(sounds,
		[&](auto& sound) {
			return sound->getStatus() == sf::Sound::Stopped;
		});

	if (buffers.count(path))
	{
		auto s = std::make_unique<sf::Sound>();
		s->setBuffer(*buffers[path].get());
		s->setLoop(loop);
		s->play();
		sounds.emplace_back(std::move(s));
	}

	return sounds.back().get();
}

void SoundManager::registerSound(const std::string& path)
{
	buffers[path] = std::make_unique<sf::SoundBuffer>();
	buffers[path]->loadFromFile(path);
}

sf::SoundBuffer* SoundManager::getSoundBuffer(const std::string& path)
{
	if (buffers.count(path))
	{
		return buffers[path].get();
	}

	return nullptr;
}
}	// namespace enki