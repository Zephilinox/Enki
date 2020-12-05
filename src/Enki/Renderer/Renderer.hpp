#pragma once

//SELF
#include "Enki/Hash.hpp"
#include "Enki/Graphics/Font.hpp"
#include "Enki/Graphics/Sprite.hpp"
#include "Enki/Graphics/Text.hpp"
#include "Enki/Graphics/Texture.hpp"
#include "Enki/Window/Window.hpp"

namespace enki
{

struct SpriteOrderInfo
{
	Sprite* sprite = nullptr;
	std::uint8_t layer = 0;
	float order = 0;
};

class Renderer
{
public:
	Renderer(HashedID type, Window* window)
		: type(type)
		, window(window)
	{
		
	}

	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;
	Renderer(Renderer&&) noexcept = delete;
	Renderer& operator=(Renderer&&) noexcept = delete;
	virtual ~Renderer() noexcept = default;
	
	[[nodiscard]] virtual std::unique_ptr<Sprite> createSprite() const = 0;
	[[nodiscard]] virtual std::unique_ptr<Text> createText() const = 0;
	[[nodiscard]] virtual std::unique_ptr<Texture> createTexture() const = 0;
	[[nodiscard]] virtual std::unique_ptr<Font> createFont() const = 0;
	
	virtual void queue(const Sprite* sprite) = 0;
	virtual void queue(const Text* text) = 0;
	virtual void render() = 0;

	[[nodiscard]] HashedID getType() const { return type; }
	[[nodiscard]] Window* getWindow() const { return window; }

	template <typename T>
	[[nodiscard]] bool is()
	{
		static_assert(std::is_base_of_v<Renderer, T>, "This type is not derived from enki::Renderer");
		return T::type == type;
	}

	template <typename T>
	[[nodiscard]] T* as()
	{
		static_assert(std::is_base_of_v<Renderer, T>, "This type is not derived from enki::Renderer");
		return T::type == type ? static_cast<T*>(this) : nullptr;
	}

	template <typename T>
	[[nodiscard]] const T* as() const
	{
		static_assert(std::is_base_of_v<Renderer, T>, "This type is not derived from enki::Renderer");
		return T::type == type ? static_cast<const T*>(this) : nullptr;
	}

protected:
	HashedID type;
	Window* window;
};

}	// namespace enki