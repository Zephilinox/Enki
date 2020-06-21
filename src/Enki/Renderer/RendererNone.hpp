#pragma once

//SELF
#include "Renderer.hpp"

namespace enki
{

class RendererNone : public Renderer
{
public:
	RendererNone(Window* window);

	void draw(const Sprite* sprite) final;
	void draw(const Text* text) final;
	void end() final;

	static constexpr RendererType type = hash_constexpr("None");

private:
};

}	// namespace enki