#pragma once

#include <SFML/Graphics.hpp>
#include <cstdint>
#include <optional>

#include "chip8.hpp"

class Platform
{
public:
	Platform(const char* title, int window_width, int window_height, int texture_width, int texture_height);
	~Platform() = default;

	void update(const uint8_t* buffer);
	bool process_input(Chip8& chip8);

private:
	sf::RenderWindow m_window;
	sf::Texture m_texture;
	sf::Sprite m_sprite;
};
