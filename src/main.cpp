#include <chrono>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>

#include "chip8.hpp"
#include "platform.hpp"

namespace
{
void print_usage(const char* program_name)
{
	std::cout << "CHIP-8 Emulator\n"
	          << "Usage: " << program_name << " <ROM> [Scale] [ClockHz]\n\n"
	          << "Arguments:\n"
	          << "  <ROM>     Path to the CHIP-8 ROM file (.ch8)\n"
	          << "  [Scale]   Window scale factor (integer, default: 15)\n"
	          << "  [ClockHz] CPU clock frequency in Hz (integer, default: 700)\n\n"
	          << "Options:\n"
	          << "  -h, --help  Display this help message and exit\n\n"
	          << "Example:\n"
	          << "  " << program_name << " share/flightrunner.ch8\n"
	          << "  " << program_name << " share/octojam1title.ch8 20 600\n";
}
} // namespace

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		print_usage(argv[0]);
		return EXIT_FAILURE;
	}

	const std::string_view first_arg = argv[1];
	if (first_arg == "-h" || first_arg == "--help")
	{
		print_usage(argv[0]);
		return EXIT_SUCCESS;
	}

	const std::string_view rom_filename = argv[1];
	int video_scale = 15;
	int clock_hz = 700;

	if (argc >= 3)
	{
		try
		{
			video_scale = std::stoi(argv[2]);
			if (video_scale <= 0)
			{
				std::cerr << "Error: Video scale must be positive.\n";
				return EXIT_FAILURE;
			}
		}
		catch (const std::exception&)
		{
			std::cerr << "Error: Invalid video scale '" << argv[2] << "'. Must be an integer.\n";
			return EXIT_FAILURE;
		}
	}

	if (argc >= 4)
	{
		try
		{
			clock_hz = std::stoi(argv[3]);
			if (clock_hz <= 0)
			{
				std::cerr << "Error: Clock frequency must be positive.\n";
				return EXIT_FAILURE;
			}
		}
		catch (const std::exception&)
		{
			std::cerr << "Error: Invalid clock frequency '" << argv[3] << "'. Must be an integer.\n";
			return EXIT_FAILURE;
		}
	}

	Chip8 chip8;
	if (!chip8.load_rom(rom_filename))
	{
		return EXIT_FAILURE;
	}

	Platform platform("CHIP-8 Emulator",
	                  Chip8::FRAME_BUF_W * video_scale,
	                  Chip8::FRAME_BUF_H * video_scale,
	                  Chip8::FRAME_BUF_W,
	                  Chip8::FRAME_BUF_H);

	constexpr double timer_tick_duration = 1.0 / 60.0;
	const double cpu_cycle_duration = 1.0 / static_cast<double>(clock_hz);

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

	return EXIT_SUCCESS;
}
