#include "chip8.hpp"

#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>

#include "fontset.hpp"

Chip8::Chip8()
	: m_be(static_cast<std::mt19937::result_type>(
	      std::chrono::system_clock::now().time_since_epoch().count()))
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

	for (size_t i = 0; i <= 0x0F; ++i)
	{
		m_table_8[i] = &Chip8::op_null;
	}

	for (size_t i = 0; i <= 0xFF; ++i)
	{
		m_table_0[i] = &Chip8::op_null;
		m_table_E[i] = &Chip8::op_null;
		m_table_F[i] = &Chip8::op_null;
	}

	m_table_0[0xE0] = &Chip8::op_00E0;
	m_table_0[0xEE] = &Chip8::op_00EE;

	m_table_8[0x0] = &Chip8::op_8xy0;
	m_table_8[0x1] = &Chip8::op_8xy1;
	m_table_8[0x2] = &Chip8::op_8xy2;
	m_table_8[0x3] = &Chip8::op_8xy3;
	m_table_8[0x4] = &Chip8::op_8xy4;
	m_table_8[0x5] = &Chip8::op_8xy5;
	m_table_8[0x6] = &Chip8::op_8xy6;
	m_table_8[0x7] = &Chip8::op_8xy7;
	m_table_8[0xE] = &Chip8::op_8xyE;

	m_table_E[0x9E] = &Chip8::op_Ex9E;
	m_table_E[0xA1] = &Chip8::op_ExA1;

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

bool Chip8::load_rom(std::string_view filename)
{
	std::ifstream file(std::string(filename), std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		std::cerr << "Failed to open ROM file: " << filename << '\n';
		return false;
	}

	const std::streampos size = file.tellg();
	if (size <= 0)
	{
		std::cerr << "ROM file is empty: " << filename << '\n';
		return false;
	}

	if (size > MAX_ROM_SIZE)
	{
		std::cerr << "ROM file size (" << size << " bytes) exceeds maximum available memory ("
		          << MAX_ROM_SIZE << " bytes): " << filename << '\n';
		return false;
	}

	file.seekg(0, std::ios::beg);
	file.read(reinterpret_cast<char*>(&m_am[IN_START_ADDR]), size);
	return true;
}

void Chip8::cycle()
{
	// Fetch
	m_pc &= (ADDR_MEM_CAP - 1);
	m_opcode = (static_cast<uint16_t>(m_am[m_pc]) << 8) | m_am[(m_pc + 1) & (ADDR_MEM_CAP - 1)];

	// Increment the PC before execution
	m_pc = (m_pc + 2) & (ADDR_MEM_CAP - 1);

	// Decode and Execute
	(this->*m_table[(m_opcode & 0xF000) >> 12])();
}

void Chip8::tick_timers()
{
	if (m_dt > 0)
	{
		--m_dt;
	}

	if (m_st > 0)
	{
		--m_st;
	}
}

const uint8_t* Chip8::get_frame_buffer() const
{
	return m_fb;
}

bool Chip8::is_sound_playing() const
{
	return m_st > 0;
}

bool Chip8::draw_flag() const
{
	return m_draw_flag;
}

void Chip8::reset_draw_flag()
{
	m_draw_flag = false;
}

void Chip8::set_key(uint8_t key, bool pressed)
{
	if (key < KEYPAD_CAP)
	{
		m_kp[key] = pressed ? 1 : 0;
	}
}

bool Chip8::is_key_pressed(uint8_t key) const
{
	if (key < KEYPAD_CAP)
	{
		return m_kp[key] != 0;
	}
	return false;
}

uint16_t Chip8::opcode_nnn() const
{
	return m_opcode & 0x0FFF;
}

uint8_t Chip8::opcode_n() const
{
	return m_opcode & 0x000F;
}

uint8_t Chip8::opcode_x() const
{
	return static_cast<uint8_t>((m_opcode & 0x0F00) >> 8);
}

uint8_t Chip8::opcode_y() const
{
	return static_cast<uint8_t>((m_opcode & 0x00F0) >> 4);
}

uint8_t Chip8::opcode_kk() const
{
	return static_cast<uint8_t>(m_opcode & 0x00FF);
}

void Chip8::table_0()
{
	if ((m_opcode & 0x0F00) == 0)
	{
		(this->*m_table_0[m_opcode & 0x00FF])();
	}
}

void Chip8::table_8()
{
	(this->*m_table_8[m_opcode & 0x000F])();
}

void Chip8::table_E()
{
	(this->*m_table_E[m_opcode & 0x00FF])();
}

void Chip8::table_F()
{
	(this->*m_table_F[m_opcode & 0x00FF])();
}

void Chip8::op_null()
{
}

void Chip8::op_00E0()
{
	std::memset(m_fb, 0, FRAME_BUF_CAP);
	m_draw_flag = true;
}

void Chip8::op_00EE()
{
	if (m_sp == 0)
	{
		return;
	}

	--m_sp;
	m_pc = m_sb[m_sp];
}

void Chip8::op_1nnn()
{
	m_pc = opcode_nnn();
}

void Chip8::op_2nnn()
{
	if (m_sp >= STACK_CAP)
	{
		return;
	}

	m_sb[m_sp] = m_pc;
	++m_sp;
	m_pc = opcode_nnn();
}

void Chip8::op_3xkk()
{
	const uint8_t x = opcode_x();
	const uint8_t kk = opcode_kk();

	if (m_V[x] == kk)
	{
		m_pc += 2;
	}
}

void Chip8::op_4xkk()
{
	const uint8_t x = opcode_x();
	const uint8_t kk = opcode_kk();

	if (m_V[x] != kk)
	{
		m_pc += 2;
	}
}

void Chip8::op_5xy0()
{
	if (opcode_n() != 0)
	{
		return;
	}

	const uint8_t x = opcode_x();
	const uint8_t y = opcode_y();

	if (m_V[x] == m_V[y])
	{
		m_pc += 2;
	}
}

void Chip8::op_6xkk()
{
	const uint8_t x = opcode_x();
	const uint8_t kk = opcode_kk();

	m_V[x] = kk;
}

void Chip8::op_7xkk()
{
	const uint8_t x = opcode_x();
	const uint8_t kk = opcode_kk();

	m_V[x] += kk;
}

void Chip8::op_8xy0()
{
	const uint8_t x = opcode_x();
	const uint8_t y = opcode_y();

	m_V[x] = m_V[y];
}

void Chip8::op_8xy1()
{
	const uint8_t x = opcode_x();
	const uint8_t y = opcode_y();

	m_V[x] |= m_V[y];
}

void Chip8::op_8xy2()
{
	const uint8_t x = opcode_x();
	const uint8_t y = opcode_y();

	m_V[x] &= m_V[y];
}

void Chip8::op_8xy3()
{
	const uint8_t x = opcode_x();
	const uint8_t y = opcode_y();

	m_V[x] ^= m_V[y];
}

void Chip8::op_8xy4()
{
	const uint8_t x = opcode_x();
	const uint8_t y = opcode_y();
	const uint16_t result = static_cast<uint16_t>(m_V[x]) + static_cast<uint16_t>(m_V[y]);
	const uint8_t carry = result > UINT8_MAX ? 1 : 0;

	m_V[NUM_REGISTERS - 1] = carry;
	m_V[x] = static_cast<uint8_t>(result & 0x00FF);
}

void Chip8::op_8xy5()
{
	const uint8_t x = opcode_x();
	const uint8_t y = opcode_y();

	m_V[NUM_REGISTERS - 1] = m_V[x] > m_V[y] ? 1 : 0;
	m_V[x] -= m_V[y];
}

void Chip8::op_8xy6()
{
	const uint8_t x = opcode_x();

	m_V[NUM_REGISTERS - 1] = m_V[x] & 1;
	m_V[x] >>= 1;
}

void Chip8::op_8xy7()
{
	const uint8_t x = opcode_x();
	const uint8_t y = opcode_y();

	m_V[NUM_REGISTERS - 1] = m_V[y] > m_V[x] ? 1 : 0;
	m_V[x] = m_V[y] - m_V[x];
}

void Chip8::op_8xyE()
{
	const uint8_t x = opcode_x();

	m_V[NUM_REGISTERS - 1] = (m_V[x] & 0x80) >> 7;
	m_V[x] <<= 1;
}

void Chip8::op_9xy0()
{
	if (opcode_n() != 0)
	{
		return;
	}

	const uint8_t x = opcode_x();
	const uint8_t y = opcode_y();

	if (m_V[x] != m_V[y])
	{
		m_pc += 2;
	}
}

void Chip8::op_Annn()
{
	m_Vi = opcode_nnn();
}

void Chip8::op_Bnnn()
{
	const uint16_t nnn = opcode_nnn();
	m_pc = (nnn + m_V[0]) & (ADDR_MEM_CAP - 1);
}

void Chip8::op_Cxkk()
{
	const uint8_t r = static_cast<uint8_t>(m_be());
	const uint8_t x = opcode_x();
	const uint8_t kk = opcode_kk();

	m_V[x] = r & kk;
}

void Chip8::op_Dxyn()
{
	const uint8_t n = opcode_n();
	const uint8_t x = opcode_x();
	const uint8_t y = opcode_y();

	const uint8_t x_pos = m_V[x] % FRAME_BUF_W;
	const uint8_t y_pos = m_V[y] % FRAME_BUF_H;

	m_V[NUM_REGISTERS - 1] = 0;

	for (size_t row = 0; row < n; ++row)
	{
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

	m_draw_flag = true;
}

void Chip8::op_Ex9E()
{
	const uint8_t x = opcode_x();

	if (is_key_pressed(m_V[x] & 0x0F))
	{
		m_pc = (m_pc + 2) & (ADDR_MEM_CAP - 1);
	}
}

void Chip8::op_ExA1()
{
	const uint8_t x = opcode_x();

	if (!is_key_pressed(m_V[x] & 0x0F))
	{
		m_pc = (m_pc + 2) & (ADDR_MEM_CAP - 1);
	}
}

void Chip8::op_Fx07()
{
	const uint8_t x = opcode_x();
	m_V[x] = m_dt;
}

void Chip8::op_Fx0A()
{
	const uint8_t x = opcode_x();

	for (size_t i = 0; i < KEYPAD_CAP; ++i)
	{
		if (m_kp[i])
		{
			m_V[x] = static_cast<uint8_t>(i);
			return;
		}
	}

	m_pc -= 2;
}

void Chip8::op_Fx15()
{
	const uint8_t x = opcode_x();
	m_dt = m_V[x];
}

void Chip8::op_Fx18()
{
	const uint8_t x = opcode_x();
	m_st = m_V[x];
}

void Chip8::op_Fx1E()
{
	const uint8_t x = opcode_x();
	m_Vi = (m_Vi + m_V[x]) & (ADDR_MEM_CAP - 1);
}

void Chip8::op_Fx29()
{
	const uint8_t x = opcode_x();
	constexpr uint8_t byte_height = 5;

	m_Vi = FONTSET_START_ADDR + byte_height * (m_V[x] & 0x0F);
}

void Chip8::op_Fx33()
{
	const uint8_t x = opcode_x();
	uint8_t Vx = m_V[x];

	m_am[m_Vi & (ADDR_MEM_CAP - 1)] = static_cast<uint8_t>(Vx / 100);
	Vx %= 100;

	m_am[(m_Vi + 1) & (ADDR_MEM_CAP - 1)] = static_cast<uint8_t>(Vx / 10);
	Vx %= 10;

	m_am[(m_Vi + 2) & (ADDR_MEM_CAP - 1)] = Vx;
}

void Chip8::op_Fx55()
{
	const uint8_t x = opcode_x();

	for (size_t i = 0; i <= x; ++i)
	{
		m_am[(m_Vi + i) & (ADDR_MEM_CAP - 1)] = m_V[i];
	}
}

void Chip8::op_Fx65()
{
	const uint8_t x = opcode_x();

	for (size_t i = 0; i <= x; ++i)
	{
		m_V[i] = m_am[(m_Vi + i) & (ADDR_MEM_CAP - 1)];
	}
}
