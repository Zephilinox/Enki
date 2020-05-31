#pragma once

//SELF
#include "Window.hpp"

namespace enki
{
class WindowNone final : public Window
{
public:
	WindowNone(Properties properties);
	WindowNone(WindowNone&&) noexcept = default;
	WindowNone& operator=(WindowNone&&) noexcept = default;
	virtual ~WindowNone() noexcept = default;

	bool poll(Event& e) final;
	[[nodiscard]] bool isOpen() final;
	[[nodiscard]] bool isVerticalSyncEnabled() final;

	void close() final;
	void clear(int r, int g, int b) final;
	void display() final;

	void setVerticalSyncEnabled(bool enabled) final;

	static constexpr HashedID type = hash_constexpr("None");

private:
	bool vsync = false;
	bool open = true;
};
}	 // namespace enki