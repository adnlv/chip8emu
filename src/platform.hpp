#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <array>
#include <cstdint>
#include <optional>

#include "chip8.hpp"

class Platform
{
public:
	Platform(const char* title, int window_width, int window_height, int texture_width, int texture_height);
	~Platform() = default;

	void update(const uint8_t* buffer);
	void render();
	void update_audio(bool is_playing);
	bool process_input(Chip8& chip8);

private:
	void init_audio();

	sf::RenderWindow m_window;
	sf::Texture m_texture;
	sf::Sprite m_sprite;
	std::array<uint8_t, Chip8::FRAME_BUF_W * Chip8::FRAME_BUF_H * 4> m_rgba_pixels{};

	sf::SoundBuffer m_beep_buffer;
	std::optional<sf::Sound> m_beep_sound;
};
