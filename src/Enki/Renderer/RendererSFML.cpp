#include "RendererSFML.hpp"

//SELF
#include "Enki/Window/WindowSFML.hpp"
#include "Enki/Window/WindowLog.hpp"
#include "Enki/Graphics/SpriteSFML.hpp"
#include "Enki/Graphics/TextSFML.hpp"
#include "Enki/Graphics/TextureSFML.hpp"
#include "Enki/Graphics/FontSFML.hpp"

namespace enki
{

RendererSFML::RendererSFML(Window* window)
	: Renderer(type, window)
{
	WindowSFML* window_sfml;
	
	auto* log = window->as<WindowLog>();
	if (log)
		window_sfml = log->getWindow().as<WindowSFML>();
	else
		window_sfml = window->as<WindowSFML>();

	if (!window_sfml)
		throw;

	sfml = window_sfml->getRawWindow();
}

std::unique_ptr<Sprite> RendererSFML::createSprite() const
{
	return std::make_unique<SpriteSFML>();
}

std::unique_ptr<Text> RendererSFML::createText() const
{
	return std::make_unique<TextSFML>();
}

std::unique_ptr<Texture> RendererSFML::createTexture() const
{
	return std::make_unique<TextureSFML>();
}

std::unique_ptr<Font> RendererSFML::createFont() const
{
	return std::make_unique<FontSFML>();
}

void RendererSFML::queue(const Sprite* sprite)
{
	const auto* sf_sprite = sprite->as<SpriteSFML>();
	sfml->draw(sf_sprite->getRawSprite());
}

void RendererSFML::queue(const Text*)
{
	
}

void RendererSFML::queue(Line line)
{
	static sf::Vertex sf_line[2];
	
	sf_line[0].position = {line.points[0].x, line.points[0].y};
	sf_line[0].color = {line.colour.r, line.colour.g, line.colour.b, line.colour.a};
	sf_line[1].position = {line.points[1].x, line.points[1].y};
	sf_line[1].color = {line.colour.r, line.colour.g, line.colour.b, line.colour.a};
	sfml->draw(sf_line, 3, sf::Lines);
}

void RendererSFML::render()
{
}

void RendererSFML::queue(sf::Drawable* drawable)
{
	sfml->draw(*drawable);
}

}	 // namespace enki