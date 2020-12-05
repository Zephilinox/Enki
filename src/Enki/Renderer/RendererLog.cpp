#include "RendererLog.hpp"

namespace enki
{
RendererLog::RendererLog(Window* window)
	: Renderer(type, window)
{
	if (!window)
		throw;
}

RendererLog::RendererLog(std::unique_ptr<Renderer> renderer)
	: Renderer(type, renderer->getWindow())
	, renderer(std::move(renderer))
{
	if (!window)
		throw;
}

std::unique_ptr<Sprite> RendererLog::createSprite() const
{
	auto sprite = renderer->createSprite();
	spdlog::info("createSprite");
	return sprite;
}

std::unique_ptr<Text> RendererLog::createText() const
{
	auto text = renderer->createText();
	spdlog::info("createText");
	return text;
}

std::unique_ptr<Texture> RendererLog::createTexture() const
{
	auto texture = renderer->createTexture();
	spdlog::info("createTexture");
	return texture;
}

std::unique_ptr<Font> RendererLog::createFont() const
{
	auto font = renderer->createFont();
	spdlog::info("createFont");
	return font;
}

void RendererLog::queue(const Sprite* sprite)
{
	renderer->queue(sprite);
	spdlog::info("queue sprite");
}

void RendererLog::queue(const Text* text)
{
	renderer->queue(text);
	spdlog::info("queue text");
}

void RendererLog::render()
{
	renderer->render();
	spdlog::info("render");
}

Renderer& RendererLog::getRenderer() const
{
	spdlog::info("getRenderer = {}", renderer->getType());
	return *renderer;
}

}	 // namespace enki