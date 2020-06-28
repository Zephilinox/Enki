#pragma once

//STD
#include <string>

//LIBS
//todo: Once we make SFML windows optional we need to move this out. Kept for now due to convinience
#include <SFML/Graphics.hpp>

//SELF
#include "Enki/NonCopyable.hpp"
#include "Enki/Hash.hpp"
#include "Enki/Input/Events.hpp"

namespace enki
{
// Window is polymorphic so that the windowing can be changed at run time
// This will probably only happen once, before it is first constructed
// Use Case: Disable windowing for running headless servers with command line argument
// Possible optimization: If we only change what the window type will be once, is the compiler able to devirtualize calls?
//  Try and limit it to only change once, it's unlikely we'd want to change the window after it is made (and the difficulties of doing so when stuff relies on it being around)
class Window : private enki::NonCopyableAndNonMovable
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
		, properties(std::move(properties)){};
	Window(Window&&) noexcept = default;
	Window& operator=(Window&&) noexcept = default;
	virtual ~Window() noexcept = default;

	virtual bool poll(Event& e) = 0;
	[[nodiscard]] virtual bool isOpen() = 0;
	[[nodiscard]] virtual bool isVerticalSyncEnabled() = 0;

	virtual void close() = 0;
	virtual void clear(int r, int g, int b) = 0;
	virtual void display() = 0;

	virtual void setVerticalSyncEnabled(bool enabled) = 0;

	[[nodiscard]] HashedID getType() const { return type; }

	template <typename W>
	bool is()
	{
		static_assert(std::is_base_of_v<Window, W>);
		return W::type == type;
	}

	template <typename W>
	W* as()
	{
		static_assert(std::is_base_of_v<Window, W>);
		return W::type == type ? static_cast<W*>(this) : nullptr;
	}

protected:
	HashedID type = hash("Window");
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
		if (window)
			return false;
		
		window = std::move(w);
		return true;
	}

	[[nodiscard]] Window& get() const noexcept
	{
		// We only want to set the window once due to devirtualization, so we can't use a NullObject pattern
		// We don't want to return nullptr, because then users have to do checks
		// We don't want to use if statements, because then we have to do branching for every get, and keep around a NullObject to return instead of a nullptr
		// We just assert and hope that no user code will actually do this at release
		// We can now return a reference.
		assert(window);
		return *(window.get());
	}

private:
	std::unique_ptr<Window> window;
};

}	// namespace enki
