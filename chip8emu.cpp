#include <string>

#include "platform.hpp"

int main(int argc, char** argv)
{
	if (argc != 4)
	{
		std::cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>\n";
		std::exit(EXIT_FAILURE);
	}

	int video_scale = std::stoi(argv[1]);
	int cycle_delay = std::stoi(argv[2]);
	char const* rom_filename = argv[3];

	Platform platform("CHIP-8 Emulator", Chip8::FRAME_BUF_W * video_scale, Chip8::FRAME_BUF_H * video_scale,
	                  Chip8::FRAME_BUF_W, Chip8::FRAME_BUF_H);

	Chip8 chip8;
	chip8.load_rom(rom_filename);

	int video_pitch = sizeof(chip8.m_fb[0]) * Chip8::FRAME_BUF_W;

	auto last_cycle_time = std::chrono::high_resolution_clock::now();
	bool quit = false;

	while (!quit)
	{
		quit = platform.process_input(chip8.m_kp);

		auto current_time = std::chrono::high_resolution_clock::now();
		float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(current_time - last_cycle_time).
			count();

		if (dt > cycle_delay)
		{
			last_cycle_time = current_time;

			chip8.cycle();

			platform.update(chip8.m_fb, video_pitch);
		}
	}

	return 0;
}
