#include "chip8.hpp"
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

	Platform platform("CHIP-8 Emulator", VM_VIDEO_BUFFER_W * video_scale, VM_VIDEO_BUFFER_H * video_scale, VM_VIDEO_BUFFER_W, VM_VIDEO_BUFFER_H);

	Chip8 chip8;
	chip8.LoadROM(rom_filename);

	int video_pitch = sizeof(chip8.video[0]) * VM_VIDEO_BUFFER_W;

	auto last_cycle_time = std::chrono::high_resolution_clock::now();
	bool quit = false;

	while (!quit)
	{
		quit = platform.ProcessInput(chip8.keypad);

		auto current_time = std::chrono::high_resolution_clock::now();
		float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(current_time - last_cycle_time).count();

		if (dt > cycle_delay)
		{
			last_cycle_time = current_time;

			chip8.Cycle();

			platform.Update(chip8.video, video_pitch);
		}
	}

	return 0;
}
