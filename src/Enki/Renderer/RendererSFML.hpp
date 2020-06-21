#pragma once

//LIBS
#include <SFML/Graphics.hpp>

//SELF
#include "Renderer.hpp"

namespace enki
{
class RendererSFML : public Renderer
{
public:
	class Helper
	{
	public:
		const OrderInfo* order_info;
		std::unique_ptr<sf::Drawable> drawable;
	};
	
	RendererSFML(Window* window);

	void draw(const Sprite* sprite) final;
	void draw(const Text* text) final;
	void end() final;

	static constexpr RendererType type = hash_constexpr("SFML");

private:
	//todo: very slow of course
	std::vector<Helper> helpers;
};

}	 // namespace enki