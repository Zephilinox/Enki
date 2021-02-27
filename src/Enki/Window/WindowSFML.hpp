#pragma once

//SELF
#include "Window.hpp"

//LIBS
#include <SFML/Graphics/RenderWindow.hpp>

//STD

//todo: I want this to be optionally available only when SFML is available. So it needs to be header-only I think.

namespace enki
{
class WindowSFML final : public Window
{
public:
	WindowSFML(Properties properties);

	bool poll(Event& e) final;
	[[nodiscard]] bool isOpen() const final;
	[[nodiscard]] bool isVerticalSyncEnabled() const final;
	
	[[nodiscard]] unsigned int getWidth() const final;
	[[nodiscard]] unsigned int getHeight() const final;

	void close() final;
	void clear(int r, int g, int b) final;
	void display() final;

	void setVerticalSyncEnabled(bool enabled) final;
	void setWidth(unsigned int width) final;
	void setHeight(unsigned int height) final;

	sf::RenderWindow* getRawWindow();

	static constexpr HashedID type = hash_constexpr("SFML");

private:
	sf::RenderWindow window;
};
}