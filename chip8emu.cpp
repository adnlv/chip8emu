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
    Chip8() :
		random_engine(static_cast<std::default_random_engine::result_type>(std::chrono::system_clock::now().time_since_epoch().count()))
	{
		// Initialize PC
		pc = VM_MEMORY_ROM_START_ADDRESS;

		// Load fonts into memory
		for (size_t i = 0; i < VM_FONTSET_CAPACITY; ++i)
		{
			memory[VM_FONTSET_START_ADDRESS + i] = fontset[i];
		}

        // Initialize RNG (std::uniform_int_distribution requires standard integer types)
		random_byte = std::uniform_int_distribution<int>(0, 0xFF);
		
		// Set up function pointer table
		table[0x0] = &Chip8::Table0;
		table[0x1] = &Chip8::OP_1NNN;
		table[0x2] = &Chip8::OP_2NNN;
		table[0x3] = &Chip8::OP_3XNN;
		table[0x4] = &Chip8::OP_4XNN;
		table[0x5] = &Chip8::OP_5XY0;
		table[0x6] = &Chip8::OP_6XNN;
		table[0x7] = &Chip8::OP_7XNN;
		table[0x8] = &Chip8::Table8;
		table[0x9] = &Chip8::OP_9XY0;
		table[0xA] = &Chip8::OP_ANNN;
		table[0xB] = &Chip8::OP_BNNN;
		table[0xC] = &Chip8::OP_CXNN;
		table[0xD] = &Chip8::OP_DXYN;
		table[0xE] = &Chip8::TableE;
		table[0xF] = &Chip8::TableF;

		for (size_t i = 0; i <= 0xE; i++)
		{
			table0[i] = &Chip8::OP_NULL;
			table8[i] = &Chip8::OP_NULL;
			tableE[i] = &Chip8::OP_NULL;
		}

		table0[0x0] = &Chip8::OP_00E0;
		table0[0xE] = &Chip8::OP_00EE;

		table8[0x0] = &Chip8::OP_8XY0;
		table8[0x1] = &Chip8::OP_8XY1;
		table8[0x2] = &Chip8::OP_8XY2;
		table8[0x3] = &Chip8::OP_8XY3;
		table8[0x4] = &Chip8::OP_8XY4;
		table8[0x5] = &Chip8::OP_8XY5;
		table8[0x6] = &Chip8::OP_8XY6;
		table8[0x7] = &Chip8::OP_8XY7;
		table8[0xE] = &Chip8::OP_8XYE;

		tableE[0x1] = &Chip8::OP_EXA1;
		tableE[0xE] = &Chip8::OP_EX9E;

		for (size_t i = 0; i <= 0x65; i++)
		{
			tableF[i] = &Chip8::OP_NULL;
		}

		tableF[0x07] = &Chip8::OP_FX07;
		tableF[0x0A] = &Chip8::OP_FX0A;
		tableF[0x15] = &Chip8::OP_FX15;
		tableF[0x18] = &Chip8::OP_FX18;
		tableF[0x1E] = &Chip8::OP_FX1E;
		tableF[0x29] = &Chip8::OP_FX29;
		tableF[0x33] = &Chip8::OP_FX33;
		tableF[0x55] = &Chip8::OP_FX55;
		tableF[0x65] = &Chip8::OP_FX65;
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

	void Cycle(void)
	{
		// Fetch
		opcode = (memory[pc] << 8) | memory[pc + 1];

		// Increment the PC before we execute anything
		pc += 2;

		// Decode and Execute
		((*this).*(table[(opcode & 0xF000) >> 12]))();

		// Decrement the delay timer if it's been set
		if (delay_timer > 0)
		{
			--delay_timer;
		}

		// Decrement the sound timer if it's been set
		if (sound_timer > 0)
		{
			--sound_timer;
		}
	}

	void OP_0NNN(void) {}

	void OP_00E0(void)
	{
		memset(video, 0, VM_VIDEO_BUFFER_CAPACITY);
	}

	void OP_00EE(void)
	{
		--sp;
		pc = stack[sp];
	}

	void OP_1NNN(void)
	{
		pc = opcode & 0x0FFF;
	}

	void OP_2NNN(void)
	{
		stack[sp] = pc;
		++sp;

		pc = opcode & 0x0FFF;
	}

	void OP_3XNN(void)
	{
		const uint8_t Vx = (opcode & 0x0F00) >> 8;
		const uint8_t byte = opcode & 0x00FF;

		if (registers[Vx] == byte)
		{
			pc += 2;
		}
	}

	void OP_4XNN(void)
	{
		const uint8_t Vx = (opcode & 0x0F00) >> 8;
		const uint8_t byte = opcode & 0x00FF;

		if (registers[Vx] != byte)
		{
			pc += 2;
		}
	}

	void OP_5XY0(void)
	{
		const uint8_t Vx = (opcode & 0x0F00) >> 8;
		const uint8_t Vy = (opcode & 0x00F0) >> 4;

		if (registers[Vx] == registers[Vy])
		{
			pc += 2;
		}
	}

	void OP_6XNN(void)
	{
		const uint8_t Vx = (opcode & 0x0F00) >> 8;
		const uint8_t byte = opcode & 0x00FF;

		registers[Vx] = byte;
	}

	void OP_7XNN(void)
	{
		const uint8_t Vx = (opcode & 0x0F00) >> 8;
		const uint8_t byte = opcode & 0x00FF;

		registers[Vx] += byte;
	}

	void OP_8XY0(void)
	{
		const uint8_t Vx = (opcode & 0x0F00) >> 8;
		const uint8_t Vy = (opcode & 0x00F0) >> 4;

		registers[Vx] = registers[Vy];
	}

	void OP_8XY1(void)
	{
		const uint8_t Vx = (opcode & 0x0F00) >> 8;
		const uint8_t Vy = (opcode & 0x00F0) >> 4;

		registers[Vx] |= registers[Vy];
	}

	void OP_8XY2(void)
	{
		const uint8_t Vx = (opcode & 0x0F00) >> 8;
		const uint8_t Vy = (opcode & 0x00F0) >> 4;

		registers[Vx] &= registers[Vy];
	}

	void OP_8XY3(void)
	{
		const uint8_t Vx = (opcode & 0x0F00) >> 8;
		const uint8_t Vy = (opcode & 0x00F0) >> 4;

		registers[Vx] ^= registers[Vy];
	}

	void OP_8XY4(void)
	{
		const uint8_t Vx = (opcode & 0x0F00) >> 8;
		const uint8_t Vy = (opcode & 0x00F0) >> 4;

		const uint16_t sum = registers[Vx] + registers[Vy];

		if (sum > 255)
		{
			registers[0x0F] = 1;
		}
		else
		{
			registers[0x0F] = 0;
		}

		registers[Vx] = sum & 0xFF;
	}

	void OP_8XY5(void)
	{
		const uint8_t Vx = (opcode & 0x0F00) >> 8;
		const uint8_t Vy = (opcode & 0x00F0) >> 4;

		if (registers[Vx] > registers[Vy])
		{
			registers[0x0F] = 1;
		}
		else
		{
			registers[0x0F] = 0;
		}

		registers[Vx] -= registers[Vy];
	}

	void OP_8XY6(void)
	{
		const uint8_t Vx = (opcode & 0x0F00) >> 8;

		// Save LSB in VF
		registers[0xF] = (registers[Vx] & 0x1);

		registers[Vx] >>= 1;
	}

	void OP_8XY7(void)
	{
		const uint8_t Vx = (opcode & 0x0F00) >> 8;
		const uint8_t Vy = (opcode & 0x00F0) >> 4;

		if (registers[Vy] > registers[Vx])
		{
			registers[0x0F] = 1;
		}
		else
		{
			registers[0x0F] = 0;
		}

		registers[Vx] = registers[Vy] - registers[Vx];
	}

	void OP_8XYE(void)
	{
		const uint8_t Vx = (opcode & 0x0F00) >> 8;

		// Save MSB in VF
		registers[0x0F] = (registers[Vx] & 0x80) >> 7;

		registers[Vx] <<= 1;
	}

	void OP_9XY0(void)
	{
		const uint8_t Vx = (opcode & 0x0F00) >> 8;
		const uint8_t Vy = (opcode & 0x00F0) >> 4;

		if (registers[Vx] != registers[Vy])
		{
			pc += 2;
		}
	}

	void OP_ANNN(void)
	{
		index = opcode & 0x0FFF;
	}

	void OP_BNNN(void)
	{
		pc = registers[0] + opcode & 0x0FFF;
	}

    void OP_CXNN(void)
	{
		const uint8_t Vx = (opcode & 0x0F00) >> 8;
		const uint8_t byte = opcode & 0x00FF;

		// uniform_int_distribution is instantiated with int; cast result to uint8_t
		registers[Vx] = static_cast<uint8_t>(random_byte(random_engine) & byte);
	}

	void OP_DXYN(void)
	{
		uint8_t Vx = (opcode & 0x0F00) >> 8;
		uint8_t Vy = (opcode & 0x00F0) >> 4;
		uint8_t height = opcode & 0x000F;

		// Wrap if going beyond screen boundaries
		uint8_t x_pos = registers[Vx] % VM_VIDEO_BUFFER_W;
		uint8_t y_pos = registers[Vy] % VM_VIDEO_BUFFER_H;

		registers[0x0F] = 0;

		for (size_t row = 0; row < height; ++row)
		{
			const uint8_t spriteByte = memory[index + row];

			for (size_t col = 0; col < 8; ++col)
			{
				const uint8_t spritePixel = spriteByte & (0x80u >> col);
				uint32_t* screenPixel = (uint32_t*)&video[(y_pos + row) * VM_VIDEO_BUFFER_W + (x_pos + col)];

				// Sprite pixel is on
				if (spritePixel != 0)
				{
					// Screen pixel also on - collision
					if (*screenPixel == 0xFFFFFFFF)
					{
						registers[0x0F] = 1;
					}

					// Effectively XOR with the sprite pixel
					*screenPixel ^= 0xFFFFFFFF;
				}
			}
		}
	}

	void OP_EX9E(void)
	{
		const uint8_t Vx = (opcode & 0x0F00) >> 8;
		const uint8_t key = registers[Vx];

		if (keypad[key])
		{
			pc += 2;
		}
	}

	void OP_EXA1(void)
	{
		const uint8_t Vx = (opcode & 0x0F00) >> 8;
		const uint8_t key = registers[Vx];

		if (keypad[key] == 0)
		{
			pc += 2;
		}
	}

	void OP_FX07(void) 
	{
		const uint8_t Vx = (opcode & 0x0F00) >> 8;

		registers[Vx] = delay_timer;
	}

	void OP_FX0A(void) 
	{
		const uint8_t Vx = (opcode & 0x0F00) >> 8;

		if (keypad[0])
		{
			registers[Vx] = 0;
		}
		else if (keypad[1])
		{
			registers[Vx] = 1;
		}
		else if (keypad[2])
		{
			registers[Vx] = 2;
		}
		else if (keypad[3])
		{
			registers[Vx] = 3;
		}
		else if (keypad[4])
		{
			registers[Vx] = 4;
		}
		else if (keypad[5])
		{
			registers[Vx] = 5;
		}
		else if (keypad[6])
		{
			registers[Vx] = 6;
		}
		else if (keypad[7])
		{
			registers[Vx] = 7;
		}
		else if (keypad[8])
		{
			registers[Vx] = 8;
		}
		else if (keypad[9])
		{
			registers[Vx] = 9;
		}
		else if (keypad[10])
		{
			registers[Vx] = 10;
		}
		else if (keypad[11])
		{
			registers[Vx] = 11;
		}
		else if (keypad[12])
		{
			registers[Vx] = 12;
		}
		else if (keypad[13])
		{
			registers[Vx] = 13;
		}
		else if (keypad[14])
		{
			registers[Vx] = 14;
		}
		else if (keypad[15])
		{
			registers[Vx] = 15;
		}
		else
		{
			pc -= 2;
		}
	}

	void OP_FX15(void) 
	{
		const uint8_t Vx = (opcode & 0x0F00) >> 8;

		delay_timer = registers[Vx];
	}
	void OP_FX18(void) 
	{
		const uint8_t Vx = (opcode & 0x0F00) >> 8;

		sound_timer = registers[Vx];
	}

	void OP_FX1E(void) 
	{
		const uint8_t Vx = (opcode & 0x0F00) >> 8;

		index += registers[Vx];
	}

	void OP_FX29(void) 
	{
		const uint8_t Vx = (opcode & 0x0F00) >> 8;
		const uint8_t digit = registers[Vx];

		index = VM_FONTSET_START_ADDRESS + (5 * digit);
	}

	void OP_FX33(void) 
	{
		uint8_t Vx = (opcode & 0x0F00) >> 8;
		uint8_t value = registers[Vx];

		// Ones-place
		memory[index + 2] = value % 10;
		value /= 10;

		// Tens-place
		memory[index + 1] = value % 10;
		value /= 10;

		// Hundreds-place
		memory[index] = value % 10;
	}

	void OP_FX55(void) 
	{
		const uint8_t Vx = (opcode & 0x0F00) >> 8;

		for (uint8_t i = 0; i <= Vx; ++i)
		{
			memory[index + i] = registers[i];
		}
	}

	void OP_FX65(void) 
	{
		const uint8_t Vx = (opcode & 0x0F00) >> 8;

		for (uint8_t i = 0; i <= Vx; ++i)
		{
			registers[i] = memory[index + i];
		}
	}

	void Table0()
	{
		((*this).*(table0[opcode & 0x000F]))();
	}

	void Table8()
	{
		((*this).*(table8[opcode & 0x000F]))();
	}

	void TableE()
	{
		((*this).*(tableE[opcode & 0x000F]))();
	}

	void TableF()
	{
		((*this).*(tableF[opcode & 0x00FF]))();
	}

	void OP_NULL()
	{
	}

	typedef void (Chip8::*Chip8Func)();
	Chip8Func table[0x0F + 1];
	Chip8Func table0[0x0E + 1];
	Chip8Func table8[0x0E + 1];
	Chip8Func tableE[0x0E + 1];
	Chip8Func tableF[0x65 + 1];

private:
	std::default_random_engine random_engine;
	std::uniform_int_distribution<int> random_byte;

	uint8_t memory[VM_MEMORY_CAPACITY]{};
	uint8_t registers[VM_REGISTERS_CAPACITY]{};
	uint16_t index{}; // Index Register
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
