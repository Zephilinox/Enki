#include "RendererNone.hpp"

namespace enki
{

RendererNone::RendererNone(Window* window)
	: Renderer(type, window)
{
}

void RendererNone::draw(const Sprite* s)
{
}

void RendererNone::draw(const Text* t)
{
}

void RendererNone::end()
{
}

}	 // namespace enki