#pragma once

#include <cstdint>
#include <optional>
#include <random>
#include <span>
#include <string_view>

class Chip8
{
public:
	static constexpr int STACK_CAP = 16;
	static constexpr int NUM_REGISTERS = 16;
	static constexpr int ADDR_MEM_CAP = 4096;
	static constexpr int FRAME_BUF_CAP = 2048;
	static constexpr int FRAME_BUF_W = 64;
	static constexpr int FRAME_BUF_H = 32;
	static constexpr int KEYPAD_CAP = 16;
	static constexpr int IN_START_ADDR = 0x200;
	static constexpr int MAX_ROM_SIZE = ADDR_MEM_CAP - IN_START_ADDR;

	explicit Chip8(std::optional<unsigned int> rng_seed = std::nullopt);
	~Chip8() = default;

	void set_rng_seed(unsigned int seed);

	bool load_rom(std::span<const uint8_t> data);
	bool load_rom(std::string_view filename);
	void cycle();
	void tick_timers();

	[[nodiscard]] const uint8_t* get_frame_buffer() const;
	[[nodiscard]] bool is_sound_playing() const;
	[[nodiscard]] bool draw_flag() const;
	void reset_draw_flag();

	void set_key(uint8_t key, bool pressed);
	[[nodiscard]] bool is_key_pressed(uint8_t key) const;

	[[nodiscard]] uint8_t get_register(uint8_t index) const;
	[[nodiscard]] uint16_t get_i() const;
	[[nodiscard]] uint16_t get_pc() const;
	[[nodiscard]] uint8_t get_sp() const;
	[[nodiscard]] uint16_t get_stack(uint8_t index) const;
	[[nodiscard]] uint8_t get_delay_timer() const;
	[[nodiscard]] uint8_t get_sound_timer() const;
	[[nodiscard]] uint8_t get_memory(uint16_t address) const;
	[[nodiscard]] std::span<const uint8_t> get_memory_span() const;

	void set_register(uint8_t index, uint8_t value);
	void set_i(uint16_t value);
	void set_pc(uint16_t value);
	void set_sp(uint8_t value);
	void set_stack(uint8_t index, uint16_t value);
	void set_delay_timer(uint8_t value);
	void set_sound_timer(uint8_t value);
	void write_memory(uint16_t address, uint8_t value);
	void write_memory(uint16_t address, std::span<const uint8_t> bytes);

private:
	std::independent_bits_engine<std::mt19937, 8, unsigned int> m_be;

	uint8_t m_V[NUM_REGISTERS]{};
	uint16_t m_Vi{};
	uint16_t m_sb[STACK_CAP]{};
	uint8_t m_sp{};
	uint8_t m_am[ADDR_MEM_CAP]{};
	uint8_t m_fb[FRAME_BUF_CAP]{};
	uint8_t m_kp[KEYPAD_CAP]{};
	uint8_t m_dt{};
	uint8_t m_st{};
	uint16_t m_pc{};
	uint16_t m_opcode{};
	bool m_draw_flag{false};
	std::optional<uint8_t> m_waiting_key{};

	using Chip8OpFunc = void (Chip8::*)();
	Chip8OpFunc m_table[0x0F + 1]{};
	Chip8OpFunc m_table_0[0xFF + 1]{};
	Chip8OpFunc m_table_8[0x0F + 1]{};
	Chip8OpFunc m_table_E[0xFF + 1]{};
	Chip8OpFunc m_table_F[0xFF + 1]{};

	uint16_t opcode_nnn() const;
	uint8_t opcode_n() const;
	uint8_t opcode_x() const;
	uint8_t opcode_y() const;
	uint8_t opcode_kk() const;

	void table_0();
	void table_8();
	void table_E();
	void table_F();

	void op_null();
	void op_00E0();
	void op_00EE();
	void op_1nnn();
	void op_2nnn();
	void op_3xkk();
	void op_4xkk();
	void op_5xy0();
	void op_6xkk();
	void op_7xkk();
	void op_8xy0();
	void op_8xy1();
	void op_8xy2();
	void op_8xy3();
	void op_8xy4();
	void op_8xy5();
	void op_8xy6();
	void op_8xy7();
	void op_8xyE();
	void op_9xy0();
	void op_Annn();
	void op_Bnnn();
	void op_Cxkk();
	void op_Dxyn();
	void op_Ex9E();
	void op_ExA1();
	void op_Fx07();
	void op_Fx0A();
	void op_Fx15();
	void op_Fx18();
	void op_Fx1E();
	void op_Fx29();
	void op_Fx33();
	void op_Fx55();
	void op_Fx65();
};
