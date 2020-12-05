#pragma once

//SELF
#include "Enki/Window/Window.hpp"

namespace enki
{

class WindowLog final : public Window
{
public:
	WindowLog();
	WindowLog(std::unique_ptr<Window> window);

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
	
	static constexpr HashedID type = hash_constexpr("Log");

	Window& getWindow() const;

private:
	std::unique_ptr<Window> window;
};


}