#pragma once

//SELF
#include "Enki/Hash.hpp"

namespace enki
{

class Texture
{
public:
	Texture(HashedID type)
		: type(type)
	{
	}

	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;
	Texture(Texture&&) noexcept = delete;
	Texture& operator=(Texture&&) noexcept = delete;
	virtual ~Texture() noexcept = default;

	virtual bool loadFromFile(const std::string& file) = 0;
	[[nodiscard]] virtual unsigned int getHeight() const = 0;
	[[nodiscard]] virtual unsigned int getWidth() const = 0;
	
	[[nodiscard]] HashedID getType() const { return type; }

	template <typename T>
	[[nodiscard]] bool is()
	{
		static_assert(std::is_base_of_v<Texture, T>, "This type is not derived from enki::Texture");
		return T::type == type;
	}

	template <typename T>
	[[nodiscard]] T* as()
	{
		static_assert(std::is_base_of_v<Texture, T>, "This type is not derived from enki::Texture");
		return T::type == type ? static_cast<T*>(this) : nullptr;
	}

	template <typename T>
	[[nodiscard]] const T* as() const
	{
		static_assert(std::is_base_of_v<Texture, T>, "This type is not derived from enki::Texture");
		return T::type == type ? static_cast<const T*>(this) : nullptr;
	}
	

protected:
	HashedID type;
};

}	 // namespace enki
