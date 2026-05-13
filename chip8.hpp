#pragma once

#include <iostream>
#include <fstream>
#include <random>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <climits>

#include "fontset.hpp"

class Chip8
{
public:
	static constexpr int STACK_CAP = 16; // stack capacity
	static constexpr int NUM_REGISTERS = 16; // number of registers
	static constexpr int ADDR_MEM_CAP = 4096; // addressable memory capacity
	static constexpr int FRAME_BUF_CAP = 2048; // overall frame buffer capacity
	static constexpr int FRAME_BUF_W = 64; // frame buffer width
	static constexpr int FRAME_BUF_H = 32; // frame buffer height
	static constexpr int KEYPAD_CAP = 16; // keypad capacity
	static constexpr int IN_START_ADDR = 0x200; // all the supported programs will start at this memory location

	std::default_random_engine m_re; // random engine
	std::independent_bits_engine<std::mt19937, CHAR_BIT, uint8_t> m_be; // byte engine

	uint8_t m_V[NUM_REGISTERS]{}; // registers
	uint16_t m_Vi{}; // index register
	uint16_t m_sb[STACK_CAP]{}; // stack
	uint8_t m_sp{}; // stack pointer, used to point to the topmost level of the stack
	uint8_t m_am[ADDR_MEM_CAP]{}; // addressable memory
	uint8_t m_fb[FRAME_BUF_CAP]{}; // frame buffer
	uint8_t m_kp[KEYPAD_CAP]{}; // keypad
	uint8_t m_dt{}; // delay timer
	uint8_t m_st{}; // sound timer
	uint16_t m_pc{}; // program counter
	uint16_t m_opcode{}; // opcode

	typedef void (Chip8::*Chip8OpFunc)();
	Chip8OpFunc m_table[0x0F + 1]{};
	Chip8OpFunc m_table_0[0x0E + 1]{};
	Chip8OpFunc m_table_8[0x0E + 1]{};
	Chip8OpFunc m_table_E[0x0E + 1]{};
	Chip8OpFunc m_table_F[0x65 + 1]{};

	/// @brief Get the lowest 12 bits of the instruction.
	uint16_t opcode_nnn()
	{
		return m_opcode & 0x0FFF; // 0x0FFF == 0000 1111 1111 1111
	}

	/// @brief Get the lowest 4 bits of the instruction.
	uint8_t opcode_n()
	{
		return m_opcode & 0x000F; // 0x000F == 0000 0000 0000 1111
	}

	/// @brief Get the lower 4 bits of the high byte of the instruction.
	uint8_t opcode_x()
	{
		return (m_opcode & 0x0F00) >> 8; // 0x0F00 == 0000 1111 0000 0000
	}

	/// @brief Get the upper 4 bits of the low byte of the instruction.
	uint8_t opcode_y()
	{
		return (m_opcode & 0x00F0) >> 4; // 0x00F0 == 0000 0000 1111 0000
	}

	/// @brief Get the lowest 8 bits of the instruction.
	uint8_t opcode_kk()
	{
		return m_opcode & 0x00FF; // 0x00FF == 0000 0000 1111 1111
	}

	Chip8()
		: m_re(std::chrono::system_clock::now().time_since_epoch().count()),
		  m_be(m_re())
	{
		m_pc = IN_START_ADDR;

		// Load fontset into memory
		for (size_t i = 0; i < FONTSET_CAP; ++i)
		{
			m_am[FONTSET_START_ADDR + i] = FONTSET[i];
		}

		m_table[0x0] = &Chip8::table_0;
		m_table[0x1] = &Chip8::op_1nnn;
		m_table[0x2] = &Chip8::op_2nnn;
		m_table[0x3] = &Chip8::op_3xkk;
		m_table[0x4] = &Chip8::op_4xkk;
		m_table[0x5] = &Chip8::op_5xy0;
		m_table[0x6] = &Chip8::op_6xkk;
		m_table[0x7] = &Chip8::op_7xkk;
		m_table[0x8] = &Chip8::table_8;
		m_table[0x9] = &Chip8::op_9xy0;
		m_table[0xA] = &Chip8::op_Annn;
		m_table[0xB] = &Chip8::op_Bnnn;
		m_table[0xC] = &Chip8::op_Cxkk;
		m_table[0xD] = &Chip8::op_Dxyn;
		m_table[0xE] = &Chip8::table_E;
		m_table[0xF] = &Chip8::table_F;

		for (size_t i = 0; i <= 0xE; i++)
		{
			m_table_0[i] = &Chip8::op_null;
			m_table_8[i] = &Chip8::op_null;
			m_table_E[i] = &Chip8::op_null;
		}

		m_table_0[0x0] = &Chip8::op_00E0;
		m_table_0[0xE] = &Chip8::op_00EE;

		m_table_8[0x0] = &Chip8::op_8xy0;
		m_table_8[0x1] = &Chip8::op_8xy1;
		m_table_8[0x2] = &Chip8::op_8xy2;
		m_table_8[0x3] = &Chip8::op_8xy3;
		m_table_8[0x4] = &Chip8::op_8xy4;
		m_table_8[0x5] = &Chip8::op_8xy5;
		m_table_8[0x6] = &Chip8::op_8xy6;
		m_table_8[0x7] = &Chip8::op_8xy7;
		m_table_8[0xE] = &Chip8::op_8xyE;

		m_table_E[0x1] = &Chip8::op_ExA1;
		m_table_E[0xE] = &Chip8::op_Ex9E;

		for (size_t i = 0; i <= 0x65; i++)
		{
			m_table_F[i] = &Chip8::op_null;
		}

		m_table_F[0x07] = &Chip8::op_Fx07;
		m_table_F[0x0A] = &Chip8::op_Fx0A;
		m_table_F[0x15] = &Chip8::op_Fx15;
		m_table_F[0x18] = &Chip8::op_Fx18;
		m_table_F[0x1E] = &Chip8::op_Fx1E;
		m_table_F[0x29] = &Chip8::op_Fx29;
		m_table_F[0x33] = &Chip8::op_Fx33;
		m_table_F[0x55] = &Chip8::op_Fx55;
		m_table_F[0x65] = &Chip8::op_Fx65;
	}

	~Chip8() = default;

	void load_rom(const std::string& filename)
	{
		if (std::ifstream file(filename, std::ios::binary | std::ios::ate); file.is_open())
		{
			const std::streampos size = file.tellg();
			const auto buffer = new char[size];

			file.seekg(0, std::ios::beg);
			file.read(buffer, size);
			file.close();

			for (size_t i = 0; i < static_cast<size_t>(size); ++i)
			{
				m_am[IN_START_ADDR + i] = buffer[i];
			}

			delete[] buffer;
		}
	}

	void cycle()
	{
		// Fetch
		m_opcode = (m_am[m_pc] << 8) | m_am[m_pc + 1];

		// Increment the PC before we execute anything
		m_pc += 2;

		// Decode and Execute
		(this->*m_table[(m_opcode & 0xF000) >> 12])();

		// Decrement the delay timer if it's been set
		if (m_dt > 0)
		{
			--m_dt;
		}

		// Decrement the sound timer if it's been set
		if (m_st > 0)
		{
			--m_st;
		}
	}

	void table_0()
	{
		(this->*m_table_0[m_opcode & 0x000F])();
	}

	void table_8()
	{
		(this->*m_table_8[m_opcode & 0x000F])();
	}

	void table_E()
	{
		(this->*m_table_E[m_opcode & 0x000F])();
	}

	void table_F()
	{
		(this->*m_table_F[m_opcode & 0x00FF])();
	}

	void op_null()
	{
	}

	/// @brief Jump to a machine code routine at nnn.
	/// This instruction is only used on the old computers on which Chip-8 was
	/// originally implemented. It is ignored by modern interpreters.
	void op_0nnn()
	{
	}

	/// @brief Clear the display.
	void op_00E0()
	{
		memset(m_fb, 0, Chip8::FRAME_BUF_CAP);
	}

	/// @brief Return from a subroutine.
	/// The interpreter sets the program counter to the address at the top of
	/// the stack, then subtracts 1 from the stack pointer.
	void op_00EE()
	{
		if (m_sp == 0)
		{
			return;
		}

		--m_sp;
		m_pc = m_sb[m_sp];
	}

	/// @brief Jump to location nnn.
	/// The interpreter sets the program counter to nnn.
	void op_1nnn()
	{
		m_pc = opcode_nnn();
	}

	/// @brief Call subroutine at nnn.
	/// The interpreter increments the stack pointer, then puts the current PC
	/// on the top of the stack. The PC is then set to nnn.
	void op_2nnn()
	{
		if (m_sp >= STACK_CAP)
		{
			return;
		}

		m_sb[m_sp] = m_pc;
		++m_sp;
		m_pc = opcode_nnn();
	}

	/// @brief Skip next instruction if Vx = kk.
	/// The interpreter compares register Vx to kk, and if they are equal,
	/// increments the program counter by 2.
	void op_3xkk()
	{
		const uint8_t x = opcode_x();
		const uint8_t kk = opcode_kk();

		if (m_V[x] == kk)
		{
			m_pc += 2;
		}
	}

	/// @brief Skip next instruction if Vx != kk.
	/// The interpreter compares register Vx to kk, and if they are not equal,
	/// increments the program counter by 2.
	void op_4xkk()
	{
		const uint8_t x = opcode_x();
		const uint8_t kk = opcode_kk();

		if (m_V[x] != kk)
		{
			m_pc += 2;
		}
	}

	/// @brief Skip next instruction if Vx = Vy.
	/// The interpreter compares register Vx to register Vy, and if they are
	/// equal, increments the program counter by 2.
	void op_5xy0()
	{
		const uint8_t x = opcode_x();
		const uint8_t y = opcode_y();

		if (m_V[x] == m_V[y])
		{
			m_pc += 2;
		}
	}

	/// @brief Set Vx = kk.
	/// The interpreter puts the value kk into register Vx.
	void op_6xkk()
	{
		const uint8_t x = opcode_x();
		const uint8_t kk = opcode_kk();

		m_V[x] = kk;
	}

	/// @brief Set Vx = Vx + kk.
	/// Adds the value kk to the value of register Vx, then stores the result
	/// in Vx.
	void op_7xkk()
	{
		const uint8_t x = opcode_x();
		const uint8_t kk = opcode_kk();

		m_V[x] += kk;
	}

	/// @brief Set Vx = Vy.
	/// Stores the value of register Vy in register Vx.
	void op_8xy0()
	{
		const uint8_t x = opcode_x();
		const uint8_t y = opcode_y();

		m_V[x] = m_V[y];
	}

	/// @brief Set Vx = Vx OR Vy.
	/// Performs a bitwise OR on the values of Vx and Vy, then stores the result
	/// in Vx. A bitwise OR compares the corresponding bits from two values, and
	/// if either bit is 1, then the same bit in the result is also 1.
	/// Otherwise, it is 0.
	void op_8xy1()
	{
		const uint8_t x = opcode_x();
		const uint8_t y = opcode_y();

		m_V[x] |= m_V[y];
	}

	/// @brief Set Vx = Vx AND Vy.
	/// Performs a bitwise AND on the values of Vx and Vy, then stores the
	/// result in Vx. A bitwise AND compares the corresponding bits from two
	/// values, and if both bits are 1, then the same bit in the result is
	/// also 1. Otherwise, it is 0.
	void op_8xy2()
	{
		const uint8_t x = opcode_x();
		const uint8_t y = opcode_y();

		m_V[x] &= m_V[y];
	}

	/// @brief Set Vx = Vx XOR Vy.
	/// Performs a bitwise exclusive OR on the values of Vx and Vy, then stores
	/// the result in Vx. An exclusive OR compares the corresponding bits from
	/// two values, and if the bits are not both the same, then the
	/// corresponding bit in the result is set to 1. Otherwise, it is 0.
	void op_8xy3()
	{
		const uint8_t x = opcode_x();
		const uint8_t y = opcode_y();

		m_V[x] ^= m_V[y];
	}

	/// @brief Set Vx = Vx + Vy, set VF = carry.
	/// The values of Vx and Vy are added together.
	/// If the result is greater than 8 bits (i.e., > 255), VF is set to 1,
	/// otherwise 0. Only the lowest 8 bits of the result are kept, and stored
	/// in Vx.
	void op_8xy4()
	{
		const uint8_t x = opcode_x();
		const uint8_t y = opcode_y();
		const uint16_t result = m_V[x] + m_V[y];
		const uint8_t carry = result > UINT8_MAX;

		m_V[NUM_REGISTERS - 1] = carry;
		m_V[x] = static_cast<uint8_t>(result & 0x00FF);
	}

	/// @brief Set Vx = Vx - Vy, set VF = NOT borrow.
	/// If Vx > Vy, then VF is set to 1, otherwise 0.
	/// Then Vy is subtracted from Vx, and the results stored in Vx.
	void op_8xy5()
	{
		const uint8_t x = opcode_x();
		const uint8_t y = opcode_y();

		m_V[NUM_REGISTERS - 1] = m_V[x] > m_V[y];
		m_V[x] -= m_V[y];
	}

	/// @brief Set Vx = Vx SHR 1.
	/// If the least-significant bit of Vx is 1, then VF is set to 1,
	/// otherwise 0. Then Vx is divided by 2.
	void op_8xy6()
	{
		const uint8_t x = opcode_x();

		m_V[NUM_REGISTERS - 1] = m_V[x] & 1;
		m_V[x] >>= 1; // same as m_V[x] /= 2
	}

	/// @brief Set Vx = Vy - Vx, set VF = NOT borrow.
	/// If Vy > Vx, then VF is set to 1, otherwise 0.
	/// Then Vx is subtracted from Vy, and the results stored in Vx.
	void op_8xy7()
	{
		const uint8_t x = opcode_x();
		const uint8_t y = opcode_y();

		m_V[NUM_REGISTERS - 1] = m_V[y] > m_V[x];
		m_V[x] = m_V[y] - m_V[x];
	}

	/// @brief Set Vx = Vx SHL 1.
	/// If the most-significant bit of Vx is 1, then VF is set to 1, otherwise
	/// to 0. Then Vx is multiplied by 2.
	void op_8xyE()
	{
		const uint8_t x = opcode_x();

		m_V[NUM_REGISTERS - 1] = (m_V[x] & 0x80) >> 7;
		m_V[x] <<= 1; // same as m_V[x] *= 2
	}

	/// @brief Skip next instruction if Vx != Vy.
	/// The values of Vx and Vy are compared, and if they are not equal,
	/// the program counter is increased by 2.
	void op_9xy0()
	{
		const uint8_t x = opcode_x();
		const uint8_t y = opcode_y();

		if (m_V[x] != m_V[y])
		{
			m_pc += 2;
		}
	}

	/// @brief Set I = nnn.
	/// The value of register I is set to nnn.
	void op_Annn()
	{
		m_Vi = opcode_nnn();
	}

	/// @brief Jump to location nnn + V0.
	/// The program counter is set to nnn plus the value of V0.
	void op_Bnnn()
	{
		const uint16_t nnn = opcode_nnn();

		m_pc = nnn + m_V[0];
	}

	/// @brief Set Vx = random byte AND kk.
	/// The interpreter generates a random number from 0 to 255, which is then
	/// ANDed with the value kk. The results are stored in Vx. See instruction
	/// 8xy2 for more information on AND.
	void op_Cxkk()
	{
		const uint8_t r = m_be();
		const uint8_t x = opcode_x();
		const uint8_t kk = opcode_kk();

		m_V[x] = r & kk;
	}

	/// @brief Display n-byte sprite starting at memory location I at (Vx, Vy),
	/// set VF = collision.
	///
	/// The interpreter reads n bytes from memory, starting at the address
	/// stored in I. These bytes are then displayed as sprites on screen at
	/// coordinates (Vx, Vy). Sprites are XORed onto the existing screen. If
	/// this causes any pixels to be erased, VF is set to 1, otherwise it is
	/// set to 0. If the sprite is positioned so part of it is outside the
	/// coordinates of the display, it wraps around to the opposite side of the
	/// screen. See instruction 8xy3 for more information on XOR.
	void op_Dxyn()
	{
		// n in this case is the pixel height of the sprite
		const uint8_t n = opcode_n();

		const uint8_t x = opcode_x();
		const uint8_t y = opcode_y();

		const uint8_t x_pos = m_V[x] % FRAME_BUF_W;
		const uint8_t y_pos = m_V[y] % FRAME_BUF_H;

		m_V[NUM_REGISTERS - 1] = 0;

		for (size_t row = 0; row < n; ++row)
		{
			// num_cols is the pixel width of the sprite
			constexpr size_t num_cols = 8;
			const uint8_t sprite_byte = m_am[m_Vi + row];

			for (size_t col = 0; col < num_cols; ++col)
			{
				const uint8_t sprite_pixel = sprite_byte & (0x80 >> col);

				if (sprite_pixel == 0)
				{
					continue;
				}

				const size_t screen_x = (x_pos + col) % FRAME_BUF_W;
				const size_t screen_y = (y_pos + row) % FRAME_BUF_H;
				const size_t pixel_idx = screen_y * FRAME_BUF_W + screen_x;

				if (m_fb[pixel_idx] == 1)
				{
					m_V[NUM_REGISTERS - 1] = 1;
				}

				m_fb[pixel_idx] ^= 1;
			}
		}
	}

	/// @brief Skip the next instruction if a key with the value of Vx is pressed.
	/// Checks the keyboard, and if the key corresponding to the value of Vx is
	/// currently in the down position, PC is increased by 2.
	void op_Ex9E()
	{
		const uint8_t x = opcode_x();

		if (m_kp[m_V[x]])
		{
			m_pc += 2;
		}
	}

	/// @brief Skip the next instruction if the key with the value of Vx is not pressed.
	/// Checks the keyboard, and if the key corresponding to the value of Vx is
	/// currently in the up position, PC is increased by 2.
	void op_ExA1()
	{
		const uint8_t x = opcode_x();

		if (!m_kp[m_V[x]])
		{
			m_pc += 2;
		}
	}

	/// @brief Set Vx = delay timer value.
	/// The value of DT is placed into Vx.
	void op_Fx07()
	{
		const uint8_t x = opcode_x();

		m_V[x] = m_dt;
	}

	/// @brief Wait for a key press, store the value of the key in Vx.
	/// All execution stops until a key is pressed, then the value of that key
	/// is stored in Vx.
	void op_Fx0A()
	{
		const uint8_t x = opcode_x();

		for (size_t i = 0; i < KEYPAD_CAP; ++i)
		{
			if (m_kp[i])
			{
				m_V[x] = i;
				return;
			}
		}

		// Loop back to the Fx0A instruction
		m_pc -= 2;
	}

	/// @brief Set delay timer = Vx.
	/// DT is set equal to the value of Vx.
	void op_Fx15()
	{
		const uint8_t x = opcode_x();

		m_dt = m_V[x];
	}

	/// @brief Set sound timer = Vx.
	/// ST is set equal to the value of Vx.
	void op_Fx18()
	{
		const uint8_t x = opcode_x();

		m_st = m_V[x];
	}

	/// @brief Set I = I + Vx.
	/// The values of I and Vx are added, and the results are stored in I.
	void op_Fx1E()
	{
		const uint8_t x = opcode_x();

		m_Vi += m_V[x];
	}

	/// @brief Set I = location of sprite for digit Vx.
	/// The value of I is set to the location for the hexadecimal sprite
	/// corresponding to the value of Vx.
	void op_Fx29()
	{
		const uint8_t x = opcode_x();
		constexpr uint8_t byte_height{5};

		m_Vi = FONTSET_START_ADDR + byte_height * m_V[x];
	}

	/// @brief Store BCD representation of Vx in memory locations I, I+1, and I+2.
	/// The interpreter takes the decimal value of Vx, and places the hundreds
	/// digit in memory at location in I, the tens digit at location I+1, and
	/// the ones digit at location I+2.
	void op_Fx33()
	{
		const uint8_t x = opcode_x();
		uint8_t Vx = m_V[x];

		m_am[m_Vi] = static_cast<uint8_t>(Vx / 100);
		Vx %= 100;

		m_am[m_Vi + 1] = static_cast<uint8_t>(Vx / 10);
		Vx %= 10;

		m_am[m_Vi + 2] = Vx;
	}

	/// @brief Store registers V0 through Vx in memory starting at location I.
	/// The interpreter copies the values of registers V0 through Vx into
	/// memory, starting at the address in I.
	void op_Fx55()
	{
		const uint8_t x = opcode_x();

		for (size_t i = 0; i <= x; ++i)
		{
			m_am[m_Vi + i] = m_V[i];
		}
	}

	/// @brief Read registers V0 through Vx from memory starting at location I.
	/// The interpreter reads values from memory starting at location I into
	/// registers V0 through Vx.
	void op_Fx65()
	{
		const uint8_t x = opcode_x();

		for (size_t i = 0; i <= x; ++i)
		{
			m_V[i] = m_am[m_Vi + i];
		}
	}
};
