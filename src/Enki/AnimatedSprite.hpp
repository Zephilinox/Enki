#pragma once

//LIBS
#include <SFML/Graphics.hpp>

//SELF
#include "Enki/Hash.hpp"
#include "Enki/Managers/TextureManager.hpp"
#include "Enki/Renderer.hpp"
#include "Enki/TimerTrigger.hpp"

namespace enki
{
	struct FrameData
	{
		std::string texture_path;
		float frame_duration_seconds = 0;
		sf::IntRect subtexture_rect;
		sf::Vector2f relative_pos;
	};

	struct Frame
	{
		int frame_number = -1;
		FrameData frame_data;
	};

	class AnimatedSprite
	{
	public:
		AnimatedSprite(TextureManager* texture_manager, bool loop = true);

		void addFrame(FrameData);
		void update(float dt);
		void draw(Renderer* renderer);

		void play();
		void pause();
		void restart();

		[[nodiscard]]
		bool isOver() const;

		[[nodiscard]]
		bool isPaused() const;

		[[nodiscard]]
		Frame getCurrentFrame() const;

		[[nodiscard]]
		sf::Sprite* getSprite();

		sf::Vector2f position;
		std::uint8_t layer = 0;
		float order = 0;

	private:
		void nextFrame();

		TextureManager* texture_manager;
		bool paused = false;
		bool loop;

		std::vector<FrameData> frames;
		sf::Sprite sprite;

		int current_frame = -1;
		TimerTrigger timer;
	};
}