#pragma once

//STD

//LIBS
#include <SFML/Graphics/RenderWindow.hpp>

//SELF
#include "Window.hpp"

//todo: I want this to be optionally available only when SFML is available. So it needs to be header-only I think.

namespace enki
{
class WindowSFML final : public Window
{
public:
	WindowSFML(Properties properties);
	WindowSFML(WindowSFML&&) noexcept = default;
	WindowSFML& operator=(WindowSFML&&) noexcept = default;
	virtual ~WindowSFML() noexcept = default;

	bool poll(Event& e) final;
	[[nodiscard]] bool isOpen() final;
	[[nodiscard]] bool isVerticalSyncEnabled() final;

	void close() final;
	void clear(int r, int g, int b) final;
	void display() final;

	void setVerticalSyncEnabled(bool enabled) final;

	sf::RenderWindow* getRawWindow();

	static constexpr HashedID type = hash_constexpr("SFML");

private:
	sf::RenderWindow window;
	bool vsync = false;

};
}