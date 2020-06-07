#include "Renderer.hpp"

//STD
#include <algorithm>
#include <execution>

namespace enki
{
Renderer::Renderer(sf::RenderWindow* window)
	: window(window)
{
	sprites.reserve(10'000);
}

void Renderer::draw(SpriteOrderInfo&& s)
{
	sprites.emplace_back(s);
}

void Renderer::draw(sf::Drawable* drawable)
{
	window->draw(*drawable);
}

void Renderer::end()
{
	std::sort(std::execution::par_unseq, sprites.begin(), sprites.end(), 
		[](const SpriteOrderInfo& left, const SpriteOrderInfo& right) {
		if (left.layer == right.layer)
			return left.order < right.order;
		else
			return left.layer < right.layer;
	});

	for (const auto& s : sprites)
		window->draw(*s.sprite);

	sprites.clear();
}
}	// namespace enki