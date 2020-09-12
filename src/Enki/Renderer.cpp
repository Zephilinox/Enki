#include "Renderer.hpp"

//STD
#include <algorithm>
#include <execution>

//LIBS
#ifdef TRACY_ENABLE
#	undef min
#	undef max
#include <Tracy.hpp>
#include <TracyC.h>
#endif

namespace enki
{
Renderer::Renderer(sf::RenderWindow* window)
	: window(window)
{
	sprites.reserve(10'000);
}

void Renderer::draw(SpriteOrderInfo&& s)
{
	ZoneScopedN("draw sprite order info")
	sprites.emplace_back(s);
}

void Renderer::draw(sf::Drawable* drawable)
{
	ZoneScopedN("draw drawable")
	window->draw(*drawable);
}

void Renderer::end()
{
	ZoneNamedN(end, "end", true)
	TracyCZoneN(sort, "end sort", true)
	std::sort(std::execution::par_unseq, sprites.begin(), sprites.end(), 
		[](const SpriteOrderInfo& left, const SpriteOrderInfo& right) {
		if (left.layer == right.layer)
			return left.order < right.order;
		else
			return left.layer < right.layer;
	});
	TracyCZoneEnd(sort)

	TracyCZoneN(loop, "end loop", true)
	for (const auto& s : sprites)
		window->draw(*s.sprite);
	TracyCZoneEnd(loop)
	
	sprites.clear();
}
}	// namespace enki