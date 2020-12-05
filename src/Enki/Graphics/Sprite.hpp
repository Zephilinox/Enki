#pragma once

//SELF
#include "Enki/Hash.hpp"
#include "Enki/Graphics/Texture.hpp"
#include "Enki/Graphics/Colour.hpp"
#include "Enki/Graphics/Vector2.hpp"

namespace enki
{

class Sprite
{
public:
	Sprite(HashedID type)
		: type(type)
	{
		
	}

	Sprite(const Sprite&) = delete;
	Sprite& operator=(const Sprite&) = delete;
	Sprite(Sprite&&) noexcept = delete;
	Sprite& operator=(Sprite&&) noexcept = delete;
	virtual ~Sprite() noexcept = default;

	virtual void setTexture(Texture* texture) = 0;
	
	virtual void setPosition(float x, float y) = 0;
	[[nodiscard]] virtual Vector2 getPosition() const = 0;
	
	virtual void setRotation(float rotation) = 0;
	[[nodiscard]] virtual float getRotation() const = 0;

	[[nodiscard]] virtual Colour getColour() const = 0;
	virtual void setColour(Colour colour) = 0;

	[[nodiscard]] HashedID getType() const { return type; }
	
	template <typename T>
	[[nodiscard]] bool is()
	{
		static_assert(std::is_base_of_v<Sprite, T>, "This type is not derived from enki::Sprite");
		return T::type == type;
	}

	template <typename T>
	[[nodiscard]] T* as()
	{
		static_assert(std::is_base_of_v<Sprite, T>, "This type is not derived from enki::Sprite");
		return T::type == type ? static_cast<T*>(this) : nullptr;
	}

	template <typename T>
	[[nodiscard]] const T* as() const
	{
		static_assert(std::is_base_of_v<Sprite, T>, "This type is not derived from enki::Sprite");
		return T::type == type ? static_cast<const T*>(this) : nullptr;
	}

protected:
	HashedID type;
};

}
