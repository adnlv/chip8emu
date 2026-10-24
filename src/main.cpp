#include <chrono>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>

#include "chip8.hpp"
#include "platform.hpp"

namespace
{
constexpr int MAX_VIDEO_SCALE = 100;
constexpr int MAX_CLOCK_HZ = 100000;

void print_usage(const char* program_name)
{
	std::cout << "CHIP-8 Emulator\n"
	          << "Usage: " << program_name << " <ROM> [options]\n"
	          << "       " << program_name << " <ROM> [Scale] [ClockHz]\n\n"
	          << "Arguments:\n"
	          << "  <ROM>                 Path to the CHIP-8 ROM file (.ch8)\n\n"
	          << "Options:\n"
	          << "  -s, --scale <factor>  Window scale factor (integer, default: 15)\n"
	          << "  -c, --clock <Hz>      CPU clock frequency in Hz (integer, default: 700)\n"
	          << "  -h, --help            Display this help message and exit\n\n"
	          << "Examples:\n"
	          << "  " << program_name << " share/flightrunner.ch8\n"
	          << "  " << program_name << " share/octojam1title.ch8 --scale 20 --clock 600\n"
	          << "  " << program_name << " share/petdog.ch8 -s 20\n";
}

bool parse_positive_int(std::string_view str, const char* name, int max_val, int& out_val)
{
	try
	{
		size_t pos = 0;
		const std::string s(str);
		const int val = std::stoi(s, &pos);
		if (pos != s.size() || val <= 0 || val > max_val)
		{
			std::cerr << "Error: " << name << " must be an integer between 1 and " << max_val
			          << ", got '" << str << "'.\n";
			return false;
		}
		out_val = val;
		return true;
	}
	catch (const std::exception&)
	{
		std::cerr << "Error: " << name << " must be an integer between 1 and " << max_val
		          << ", got '" << str << "'.\n";
		return false;
	}
}
} // namespace

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		print_usage(argv[0]);
		return EXIT_FAILURE;
	}

	std::string_view rom_filename;
	int video_scale = 15;
	int clock_hz = 700;
	int positional_index = 0;

	for (int i = 1; i < argc; ++i)
	{
		const std::string_view arg = argv[i];

		if (arg == "-h" || arg == "--help")
		{
			print_usage(argv[0]);
			return EXIT_SUCCESS;
		}
		if (arg == "-s" || arg == "--scale")
		{
			if (i + 1 >= argc)
			{
				std::cerr << "Error: Missing value for " << arg << " option.\n";
				return EXIT_FAILURE;
			}
			++i;
			if (!parse_positive_int(argv[i], "Video scale", MAX_VIDEO_SCALE, video_scale))
			{
				return EXIT_FAILURE;
			}
		}
		else if (arg.starts_with("--scale=") || arg.starts_with("-s="))
		{
			const size_t eq_pos = arg.find('=');
			if (!parse_positive_int(arg.substr(eq_pos + 1), "Video scale", MAX_VIDEO_SCALE, video_scale))
			{
				return EXIT_FAILURE;
			}
		}
		else if (arg == "-c" || arg == "--clock")
		{
			if (i + 1 >= argc)
			{
				std::cerr << "Error: Missing value for " << arg << " option.\n";
				return EXIT_FAILURE;
			}
			++i;
			if (!parse_positive_int(argv[i], "Clock frequency", MAX_CLOCK_HZ, clock_hz))
			{
				return EXIT_FAILURE;
			}
		}
		else if (arg.starts_with("--clock=") || arg.starts_with("-c="))
		{
			const size_t eq_pos = arg.find('=');
			if (!parse_positive_int(arg.substr(eq_pos + 1), "Clock frequency", MAX_CLOCK_HZ, clock_hz))
			{
				return EXIT_FAILURE;
			}
		}
		else if (arg.starts_with("-"))
		{
			std::cerr << "Error: Unknown option '" << arg << "'.\n\n";
			print_usage(argv[0]);
			return EXIT_FAILURE;
		}
		else
		{
			if (positional_index == 0)
			{
				rom_filename = arg;
			}
			else if (positional_index == 1)
			{
				if (!parse_positive_int(arg, "Video scale", MAX_VIDEO_SCALE, video_scale))
				{
					return EXIT_FAILURE;
				}
			}
			else if (positional_index == 2)
			{
				if (!parse_positive_int(arg, "Clock frequency", MAX_CLOCK_HZ, clock_hz))
				{
					return EXIT_FAILURE;
				}
			}
			else
			{
				std::cerr << "Error: Unexpected positional argument '" << arg << "'.\n";
				return EXIT_FAILURE;
			}
			++positional_index;
		}
	}

	if (rom_filename.empty())
	{
		std::cerr << "Error: No ROM file specified.\n\n";
		print_usage(argv[0]);
		return EXIT_FAILURE;
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

		bool timer_ticked = false;
		while (timer_accumulator >= timer_tick_duration)
		{
			chip8.tick_timers();
			timer_accumulator -= timer_tick_duration;
			timer_ticked = true;
		}

		platform.update_audio(chip8.is_sound_playing());

		if (chip8.draw_flag())
		{
			platform.update(chip8.get_frame_buffer());
			chip8.reset_draw_flag();
		}
		else if (timer_ticked)
		{
			platform.render();
		}

		std::this_thread::sleep_for(std::chrono::microseconds(500));
	}

	return EXIT_SUCCESS;
}
