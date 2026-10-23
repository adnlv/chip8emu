#include <chrono>
#include <iostream>
#include <string>
#include <thread>

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

	Platform platform("CHIP-8 Emulator", Chip8::FRAME_BUF_W * video_scale, Chip8::FRAME_BUF_H * video_scale,
	                  Chip8::FRAME_BUF_W, Chip8::FRAME_BUF_H);

	Chip8 chip8;
	if (!chip8.load_rom(rom_filename))
	{
		return EXIT_FAILURE;
	}

	constexpr double timer_tick_duration = 1.0 / 60.0;
	const double cpu_cycle_duration = cycle_delay > 0 ? (cycle_delay / 1000.0) : (1.0 / 700.0);

	auto last_time = std::chrono::steady_clock::now();
	double cpu_accumulator = 0.0;
	double timer_accumulator = 0.0;
	bool quit = false;

	while (!quit)
	{
		auto current_time = std::chrono::steady_clock::now();
		double frame_time = std::chrono::duration<double>(current_time - last_time).count();
		last_time = current_time;

		if (frame_time > 0.1)
		{
			frame_time = 0.1;
		}

		cpu_accumulator += frame_time;
		timer_accumulator += frame_time;

		quit = platform.process_input(chip8);

		while (cpu_accumulator >= cpu_cycle_duration)
		{
			chip8.cycle();
			cpu_accumulator -= cpu_cycle_duration;
		}

		while (timer_accumulator >= timer_tick_duration)
		{
			chip8.tick_timers();
			timer_accumulator -= timer_tick_duration;
		}

		platform.update_audio(chip8.is_sound_playing());

		if (chip8.draw_flag())
		{
			platform.update(chip8.get_frame_buffer());
			chip8.reset_draw_flag();
		}

		std::this_thread::sleep_for(std::chrono::microseconds(500));
	}

	return 0;
}
