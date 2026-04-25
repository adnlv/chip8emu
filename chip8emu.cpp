#include <iostream>
#include <fstream>
#include <cstdint>
#include <chrono>
#include <random>

#define VM_MEMORY_CAPACITY 0x1000
#define VM_REGISTERS_CAPACITY 0x10
#define VM_STACK_CAPACITY 0x10
#define VM_KEYPAD_CAPACITY 0x10
#define VM_VIDEO_BUFFER_W 0x40
#define VM_VIDEO_BUFFER_H 0x20
#define VM_VIDEO_BUFFER_CAPACITY 0x0800
#define VM_MEMORY_ROM_START_ADDRESS 0x200
#define VM_FONTSET_CAPACITY 0x50
#define VM_FONTSET_START_ADDRESS 0x50

uint8_t fontset[VM_FONTSET_CAPACITY] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

class Chip8
{
public:
	Chip8():
		pc{ VM_MEMORY_ROM_START_ADDRESS },
		random_engine(std::chrono::system_clock::now().time_since_epoch().count())
	{
		// Load fonts into memory
		for (size_t i = 0; i < VM_FONTSET_CAPACITY; ++i)
		{
			memory[VM_FONTSET_START_ADDRESS + i] = fontset[i];
		}

		// Initialize RNG
		random_byte = std::uniform_int_distribution<uint8_t>(0, 0xFF);
	}
	
	~Chip8();

	void LoadROM(std::string filename)
	{
		std::ifstream file(filename, std::ios::binary | std::ios::ate);

		if (file.is_open())
		{
			const std::streampos size = file.tellg();
			char* buffer = new char[size];

			file.seekg(0, std::ios::beg);
			file.read(buffer, size);
			file.close();

			for (size_t i = 0; i < size; ++i)
			{
				memory[VM_MEMORY_ROM_START_ADDRESS + i] = buffer[i];
			}

			delete[] buffer;
		}
	}

private:
	std::default_random_engine random_engine;
	std::uniform_int_distribution<uint8_t> random_byte;

	uint8_t memory[VM_MEMORY_CAPACITY]{};
	uint8_t registers[VM_REGISTERS_CAPACITY]{};
	uint16_t i{}; // Index Register
	uint16_t pc{}; // Program Counter
	uint16_t stack[VM_STACK_CAPACITY]{};
	uint8_t sp{}; // Stack Pointer
	uint8_t delay_timer{};
	uint8_t sound_timer{};
	uint8_t keypad[VM_KEYPAD_CAPACITY]{};
	uint8_t video[VM_VIDEO_BUFFER_CAPACITY]{};
	uint16_t opcode{};
};

int main()
{
	std::cout << "Hello CMake." << std::endl;
	return 0;
}
