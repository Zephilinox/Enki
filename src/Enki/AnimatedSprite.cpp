#include "AnimatedSprite.hpp"

//LIBS
#include <spdlog/spdlog.h>

namespace enki
{
AnimatedSprite::AnimatedSprite(TextureManager* texture_manager, bool loop)
	: texture_manager(texture_manager)
	, loop(loop)
{
	timer.on_trigger.connect([&]() {
		nextFrame();
	});
}

void AnimatedSprite::addFrame(FrameData data)
{
	if (!texture_manager->textureExists(data.texture_path))
	{
		spdlog::error("AnimatedSprite couldn't add a frame because desired texture has not been registered, path is '{}'", data.texture_path);
		return;
	}

	frames.emplace_back(std::move(data));

	//currently -1, make it 0 and update sprite's texture and stuff with the frame data
	if (frames.size() == 1)
	{
		nextFrame();
	}
}

void AnimatedSprite::update(float dt)
{
	//the position will be updated regularly, so we need to update the sprite to match
	sprite.setPosition(position + frames[current_frame].relative_pos);

	if (!paused && !frames.empty())
	{
		timer.update(dt);
	}
}

void AnimatedSprite::draw(Renderer* renderer)
{
	renderer->draw({&sprite, layer, order});
}

void AnimatedSprite::play()
{
	paused = false;
}

void AnimatedSprite::pause()
{
	paused = true;
}

void AnimatedSprite::restart()
{
	paused = !loop;
}

bool AnimatedSprite::isOver() const
{
	if (loop || frames.empty())
	{
		return false;
	}

	return current_frame == frames.size() - 1 && timer.getTimeInSeconds() >= frames.back().frame_duration_seconds;
}

bool AnimatedSprite::isPaused() const
{
	return paused;
}

Frame AnimatedSprite::getCurrentFrame() const
{
	if (!frames.empty())
	{
		return {current_frame, frames[current_frame]};
	}
	else
	{
		return {};
	}
}

sf::Sprite* AnimatedSprite::getSprite()
{
	return &sprite;
}

void AnimatedSprite::nextFrame()
{
	if (current_frame < frames.size() - 1)
	{
		current_frame++;
	}
	else if (loop)	//on last frame, reset to start
	{
		current_frame = 0;
	}
	else	//stay on last frame
	{
		return;
	}

	sf::Texture* texture = texture_manager->getTexture(frames[current_frame].texture_path);
	sprite.setTexture(*texture, true);

	if (frames[current_frame].subtexture_rect != sf::IntRect{})
	{
		sprite.setTextureRect(frames[current_frame].subtexture_rect);
	}

	sprite.setPosition(position + frames[current_frame].relative_pos);
	timer.trigger_time_seconds = frames[current_frame].frame_duration_seconds;
}
}	// namespace enki