#pragma once

//SELF
#include "Enki/Renderer/Renderer.hpp"

namespace enki
{
class RendererNone final : public Renderer
{
public:
	RendererNone(Window* window);

	[[nodiscard]] std::unique_ptr<Sprite> createSprite() const final;
	[[nodiscard]] std::unique_ptr<Text> createText() const final;
	[[nodiscard]] std::unique_ptr<Texture> createTexture() const final;
	[[nodiscard]] std::unique_ptr<Font> createFont() const final;

	void queue(const Sprite* sprite) final;
	void queue(const Text* text) final;
	void queue(Line line);
	void render() final;

	static constexpr HashedID type = hash_constexpr("None");

private:
};

}	 // namespace enki