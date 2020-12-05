#pragma once

//LIBS
#include <SFML/Graphics.hpp>

//SELF
#include "Enki/Renderer/Renderer.hpp"

namespace enki
{

class WindowSFML;

class RendererSFML final : public Renderer
{
public:
	RendererSFML(Window* window);
	
	[[nodiscard]] std::unique_ptr<Sprite> createSprite() const final;
	[[nodiscard]] std::unique_ptr<Text> createText() const final;
	[[nodiscard]] std::unique_ptr<Texture> createTexture() const final;
	[[nodiscard]] std::unique_ptr<Font> createFont() const final;
	
	void queue(const Sprite* sprite) final;
	void queue(const Text* text) final;
	void render() final;

	static constexpr HashedID type = hash_constexpr("SFML");

	void queue(sf::Drawable* drawable);

private:
	sf::RenderWindow* sfml;
};

}