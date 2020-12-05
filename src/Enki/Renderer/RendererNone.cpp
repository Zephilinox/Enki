#include "RendererNone.hpp"

//SELF
#include "Enki/Graphics/SpriteNone.hpp"
#include "Enki/Graphics/TextNone.hpp"
#include "Enki/Graphics/TextureNone.hpp"
#include "Enki/Graphics/FontNone.hpp"

namespace enki
{

RendererNone::RendererNone(Window* window)
	: Renderer(type, window)
{
}

std::unique_ptr<Sprite> RendererNone::createSprite() const
{
	return std::make_unique<SpriteNone>();
}

std::unique_ptr<Text> RendererNone::createText() const
{
	return std::make_unique<TextNone>();
}

std::unique_ptr<Texture> RendererNone::createTexture() const
{
	return std::make_unique<TextureNone>();
}

std::unique_ptr<Font> RendererNone::createFont() const
{
	return std::make_unique<FontNone>();
}

void RendererNone::queue(const Sprite* sprite)
{
}

void RendererNone::queue(const Text* text)
{
}

void RendererNone::render()
{
}

}