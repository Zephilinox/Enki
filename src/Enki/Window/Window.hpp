#pragma once

//STD
#include <string>

//LIBS
//todo: Once we make SFML windows optional we need to move this out. Kept for now due to convinience
#include <SFML/Graphics.hpp>

//SELF
#include "Enki/Hash.hpp"
#include "Enki/Input/Events.hpp"

namespace enki
{

class Window
{
public:
	struct Properties
	{
		unsigned int width{};
		unsigned int height{};
		std::string title{};
		bool vsync{false};
	};

	Window(HashedID type, Properties properties)
		: type(type)
		, properties(std::move(properties))
	{
		
	}
	
	Window(const Window&) noexcept = delete;
	Window(Window&&) noexcept = delete;
	Window& operator=(Window&&) noexcept = delete;
	Window& operator=(const Window&) noexcept = delete;
	virtual ~Window() noexcept = default;

	virtual bool poll(Event& e) = 0;
	[[nodiscard]] virtual bool isOpen() const = 0;
	[[nodiscard]] virtual bool isVerticalSyncEnabled() const = 0;
	
	[[nodiscard]] virtual unsigned int getWidth() const = 0;
	[[nodiscard]] virtual unsigned int getHeight() const = 0;

	virtual void close() = 0;
	virtual void clear(int r, int g, int b) = 0;
	virtual void display() = 0;

	virtual void setVerticalSyncEnabled(bool enabled) = 0;
	virtual void setWidth(unsigned int width) = 0;
	virtual void setHeight(unsigned int height) = 0;

	[[nodiscard]] HashedID getType() const { return type; }

	template <typename T>
	[[nodiscard]] bool is()
	{
		static_assert(std::is_base_of_v<Window, T>, "This type is not derived from enki::Window");
		return T::type == type;
	}

	template <typename T>
	[[nodiscard]] T* as()
	{
		static_assert(std::is_base_of_v<Window, T>, "This type is not derived from enki::Window");
		return T::type == type ? static_cast<T*>(this) : nullptr;
	}
	
	template <typename T>
	[[nodiscard]] const T* as() const
	{
		static_assert(std::is_base_of_v<Window, T>, "This type is not derived from enki::Window");
		return T::type == type ? static_cast<const T*>(this) : nullptr;
	}

protected:
	HashedID type;
	Properties properties;
};

// Create a static instance globally for a typical ServiceLocator (e.g. window_locator.get())
// Create an instance as a member for more limited uses (e.g. engine.window.get())
// Create a static instance as a member (e.g. Engine::window.get()
// Alternatively: modify the class to be all static (e.g. WindowLocator::get).
//  I don't usually like it since it constrains how many service locators you can have for a class
//  The upside is the syntax looks a little nicer maybe?
//todo: I could easily make some templated locator thing with CRTP, but for now let's keep it simple
class WindowLocator
{
public:
	bool provide(std::unique_ptr<Window>&& w) noexcept
	{
		assert(w);
		
		if (window)
			return false;
		
		window = std::move(w);
		return true;
	}

	[[nodiscard]] Window& get() const noexcept
	{
		assert(window);
		return *(window.get());
	}

private:
	std::unique_ptr<Window> window;
};

}	// namespace enki
