#pragma once

//SELF
#include "Enki/Hash.hpp"
#include "Enki/Graphics/Sprite.hpp"
#include "Enki/Graphics/Font.hpp"

namespace enki
{

class Text
{
public:
	Text(HashedID type)
		: type(type)
	{
	}

	Text(const Text&) = delete;
	Text& operator=(const Text&) = delete;
	Text(Text&&) noexcept = delete;
	Text& operator=(Text&&) noexcept = delete;
	virtual ~Text() noexcept = default;

	virtual void setPosition(float x, float y) = 0;
	[[nodiscard]] virtual Vector2 getPosition() const = 0;

	virtual void setFillColor(Colour colour) = 0;
	virtual void setFont(Font* font) = 0;
	virtual void setString(const std::string& string) = 0;
	virtual void setCharacterSize(std::uint8_t size) = 0;
	
	[[nodiscard]] HashedID getType() const { return type; }

	template <typename T>
	[[nodiscard]] bool is()
	{
		static_assert(std::is_base_of_v<Text, T>, "This type is not derived from enki::Text");
		return T::type == type;
	}

	template <typename T>
	[[nodiscard]] T* as()
	{
		static_assert(std::is_base_of_v<Text, T>, "This type is not derived from enki::Text");
		return T::type == type ? static_cast<T*>(this) : nullptr;
	}

	template <typename T>
	[[nodiscard]] const T* as() const
	{
		static_assert(std::is_base_of_v<Text, T>, "This type is not derived from enki::Text");
		return T::type == type ? static_cast<const T*>(this) : nullptr;
	}
	

protected:
	HashedID type;
};

}	 // namespace enki
