#pragma once

//SELF
#include <Enki/Hash.hpp>
#include <Enki/Window/Window.hpp>

namespace enki
{

//todo: move this stuff to proper locations with actual implementations
//todo: create a texture manager that can produce texture IDs
//todo: reference counted wrapper for ID's that isn't shared_ptr (probably), with a weak_ptr equivalent
using TextureID = HashedID;

//todo: tie in to font manager
using FontID = HashedID;

using RendererType = HashedID;

//todo: make appropriate class (and one with bytes instead of floats? enums for type of colour space? optional alpha? etc)
struct Colour
{
	//0 to 1
	float r = 0;
	float g = 0;
	float b = 0;
	float a = 0;
};

/*
 * Sprites are rendered back to front based on layer and ordering. todo: different sorting/unsorted methods
 * A sprite on layer will ALWAYS be placed behind a sprite on a higher layer
 * A sprite will be placed below a sprite on the same layer that has a higher ordering
 * E.g. SpriteA = layer 0, order 1.0f. SpriteB = layer 1, order 0.0f
 *     SpriteA is placed behind SpriteB due to being on a lower layer, despite having a higher ordering
 * E.g SpriteA = layer 0, order 1.0f. SpriteB = layer 0, order 0.5f
 *     Sprite A is placed in front of SpriteB due to being on the same layer with a higher order
 */
struct OrderInfo
{
	//can be thought of as the Z axis despite there not being one, with a lower order being drawn in the background (in to the screen)
	float order = 0;
	//allows you to group sprites up in to different layers, which take in to account ordering separately
	std::uint8_t layer = 0;
};

struct TransformDecomposed
{
	//todo: renders can (and will) treat the origin differently, needs some kind of enum so the user can say what they want it to be (?)
	float pos_x = 0;
	float pos_y = 0;

	//in degrees, not radians.
	//todo: renderers can (and will) treat the angle differently, needs some kind of enum so the user can say what they want it to be(?)
	float angle = 0;
	
	float scale_x = 1;
	float scale_y = 1;
};

struct Sprite
{
	TransformDecomposed transform;
	OrderInfo order_info;
	Colour colour;
	TextureID texture = 0;
};

struct Text
{
	std::string text;
	FontID font_id;
	TransformDecomposed transform;
	OrderInfo order_info;
	Colour colour;
	Colour colour_outline;
};

class Renderer
{
public:
	Renderer(const Renderer&) = delete;
	Renderer(Renderer&&) = delete;
	Renderer& operator=(const Renderer&) = delete;
	Renderer& operator=(Renderer&&) = delete;
	virtual ~Renderer() = default;
	
	virtual void draw(const Sprite* sprite) = 0;
	virtual void draw(const Text* text) = 0;
	virtual void end() = 0;

	[[nodiscard]] RendererType getType() const { return type; };

protected:
	Renderer(RendererType type, Window* window)
		: type(type)
		, window(window)
	{
	}
	
	RendererType type;
	Window* window;
};
}	// namespace enki