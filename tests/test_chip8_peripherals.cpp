#include <gtest/gtest.h>

#include "chip8.hpp"
#include "fontset.hpp"

namespace
{
void execute_instruction(Chip8& chip8, uint16_t opcode)
{
	const uint16_t pc = chip8.get_pc();
	chip8.write_memory(pc, static_cast<uint8_t>(opcode >> 8));
	chip8.write_memory(static_cast<uint16_t>(pc + 1), static_cast<uint8_t>(opcode & 0xFF));
	chip8.cycle();
}
} // namespace

TEST(Chip8PeripheralTest, DelayAndSoundTimersTickDown)
{
	Chip8 chip8;
	chip8.set_delay_timer(2);
	chip8.set_sound_timer(1);

	EXPECT_TRUE(chip8.is_sound_playing());
	EXPECT_EQ(chip8.get_delay_timer(), 2);
	EXPECT_EQ(chip8.get_sound_timer(), 1);

	chip8.tick_timers();
	EXPECT_EQ(chip8.get_delay_timer(), 1);
	EXPECT_EQ(chip8.get_sound_timer(), 0);
	EXPECT_FALSE(chip8.is_sound_playing());

	chip8.tick_timers();
	EXPECT_EQ(chip8.get_delay_timer(), 0);
	EXPECT_EQ(chip8.get_sound_timer(), 0);

	// Ensure timers don't underflow
	chip8.tick_timers();
	EXPECT_EQ(chip8.get_delay_timer(), 0);
	EXPECT_EQ(chip8.get_sound_timer(), 0);
}

TEST(Chip8PeripheralTest, OpcodeFx15AndFx07SetAndReadDelayTimer)
{
	Chip8 chip8;
	chip8.set_register(1, 45);

	execute_instruction(chip8, 0xF115); // LD DT, V1
	EXPECT_EQ(chip8.get_delay_timer(), 45);

	execute_instruction(chip8, 0xF207); // LD V2, DT
	EXPECT_EQ(chip8.get_register(2), 45);
}

TEST(Chip8PeripheralTest, OpcodeFx18SetsSoundTimer)
{
	Chip8 chip8;
	chip8.set_register(1, 30);

	execute_instruction(chip8, 0xF118); // LD ST, V1
	EXPECT_EQ(chip8.get_sound_timer(), 30);
	EXPECT_TRUE(chip8.is_sound_playing());
}

TEST(Chip8PeripheralTest, KeypadPressQueryAndUpdates)
{
	Chip8 chip8;
	EXPECT_FALSE(chip8.is_key_pressed(0xA));

	chip8.set_key(0xA, true);
	EXPECT_TRUE(chip8.is_key_pressed(0xA));

	chip8.set_key(0xA, false);
	EXPECT_FALSE(chip8.is_key_pressed(0xA));
}

TEST(Chip8PeripheralTest, OpcodeEx9ESkipsIfKeyPressed)
{
	Chip8 chip8;
	chip8.set_register(1, 0x5);
	chip8.set_key(0x5, true);

	execute_instruction(chip8, 0xE19E); // SKP V1
	EXPECT_EQ(chip8.get_pc(), Chip8::IN_START_ADDR + 4);

	chip8.set_pc(Chip8::IN_START_ADDR);
	chip8.set_key(0x5, false);

	execute_instruction(chip8, 0xE19E); // SKP V1
	EXPECT_EQ(chip8.get_pc(), Chip8::IN_START_ADDR + 2);
}

TEST(Chip8PeripheralTest, OpcodeExA1SkipsIfKeyNotPressed)
{
	Chip8 chip8;
	chip8.set_register(1, 0x5);
	chip8.set_key(0x5, false);

	execute_instruction(chip8, 0xE1A1); // SKNP V1
	EXPECT_EQ(chip8.get_pc(), Chip8::IN_START_ADDR + 4);

	chip8.set_pc(Chip8::IN_START_ADDR);
	chip8.set_key(0x5, true);

	execute_instruction(chip8, 0xE1A1); // SKNP V1
	EXPECT_EQ(chip8.get_pc(), Chip8::IN_START_ADDR + 2);
}

TEST(Chip8PeripheralTest, OpcodeFx0AWaitForKeyPressAndRelease)
{
	Chip8 chip8;
	chip8.write_memory(Chip8::IN_START_ADDR, 0xF1);
	chip8.write_memory(Chip8::IN_START_ADDR + 1, 0x0A); // LD V1, K

	// 1. No key is pressed: PC stays at 0x200
	chip8.cycle();
	EXPECT_EQ(chip8.get_pc(), Chip8::IN_START_ADDR);

	// 2. Key 4 is pressed down: still waiting for release, PC stays at 0x200
	chip8.set_key(4, true);
	chip8.cycle();
	EXPECT_EQ(chip8.get_pc(), Chip8::IN_START_ADDR);

	// 3. Key 4 is released: key accepted into V1 and PC advances to 0x202
	chip8.set_key(4, false);
	chip8.cycle();
	EXPECT_EQ(chip8.get_register(1), 4);
	EXPECT_EQ(chip8.get_pc(), Chip8::IN_START_ADDR + 2);
}

TEST(Chip8PeripheralTest, OpcodeFx1EAddsVxToIndexRegister)
{
	Chip8 chip8;
	chip8.set_i(0x100);
	chip8.set_register(1, 0x25);

	execute_instruction(chip8, 0xF11E); // ADD I, V1
	EXPECT_EQ(chip8.get_i(), 0x125);
}

TEST(Chip8PeripheralTest, OpcodeFx29SetsIndexToFontCharacterSprite)
{
	Chip8 chip8;
	chip8.set_register(1, 0x0A); // Character 'A'

	execute_instruction(chip8, 0xF129); // LD F, V1
	// Fontset start is 0x50, each character is 5 bytes: 0x50 + 5 * 10 = 0x82
	EXPECT_EQ(chip8.get_i(), FONTSET_START_ADDR + 5 * 10);
}

TEST(Chip8PeripheralTest, OpcodeFx33ConvertsByteToBinaryCodedDecimal)
{
	Chip8 chip8;
	chip8.set_i(0x300);
	chip8.set_register(1, 254);

	execute_instruction(chip8, 0xF133); // LD B, V1

	EXPECT_EQ(chip8.get_memory(0x300), 2);
	EXPECT_EQ(chip8.get_memory(0x301), 5);
	EXPECT_EQ(chip8.get_memory(0x302), 4);
}

TEST(Chip8PeripheralTest, OpcodeFx55AndFx65StoresAndLoadsRegistersInMemory)
{
	Chip8 chip8;
	chip8.set_i(0x400);
	chip8.set_register(0, 11);
	chip8.set_register(1, 22);
	chip8.set_register(2, 33);
	chip8.set_register(3, 44);

	// Store V0..V3 to memory at I
	execute_instruction(chip8, 0xF355);

	EXPECT_EQ(chip8.get_memory(0x400), 11);
	EXPECT_EQ(chip8.get_memory(0x401), 22);
	EXPECT_EQ(chip8.get_memory(0x402), 33);
	EXPECT_EQ(chip8.get_memory(0x403), 44);

	// Reset registers
	chip8.set_register(0, 0);
	chip8.set_register(1, 0);
	chip8.set_register(2, 0);
	chip8.set_register(3, 0);

	// Load V0..V3 from memory at I
	execute_instruction(chip8, 0xF365);

	EXPECT_EQ(chip8.get_register(0), 11);
	EXPECT_EQ(chip8.get_register(1), 22);
	EXPECT_EQ(chip8.get_register(2), 33);
	EXPECT_EQ(chip8.get_register(3), 44);
}

TEST(Chip8PeripheralTest, Opcode00E0ClearsDisplayBuffer)
{
	Chip8 chip8;
	// Draw a pixel manually to framebuffer or draw sprite
	chip8.set_i(0x250);
	chip8.write_memory(0x250, 0xFF);
	execute_instruction(chip8, 0xD001); // DRW V0, V0, 1

	EXPECT_TRUE(chip8.draw_flag());
	chip8.reset_draw_flag();
	EXPECT_FALSE(chip8.draw_flag());

	execute_instruction(chip8, 0x00E0); // CLS

	EXPECT_TRUE(chip8.draw_flag());
	const uint8_t* fb = chip8.get_frame_buffer();
	for (size_t i = 0; i < Chip8::FRAME_BUF_CAP; ++i)
	{
		ASSERT_EQ(fb[i], 0);
	}
}

TEST(Chip8PeripheralTest, OpcodeDxynDrawsSpriteAndDetectsCollision)
{
	Chip8 chip8;
	// Store 1-byte sprite: 11110000 at 0x300
	chip8.set_i(0x300);
	chip8.write_memory(0x300, 0xF0);

	chip8.set_register(0, 0); // X = 0
	chip8.set_register(1, 0); // Y = 0

	// 1. Draw sprite the first time (no collision expected, VF = 0)
	execute_instruction(chip8, 0xD011); // DRW V0, V1, 1

	EXPECT_EQ(chip8.get_register(0xF), 0);
	EXPECT_TRUE(chip8.draw_flag());

	const uint8_t* fb = chip8.get_frame_buffer();
	for (size_t col = 0; col < 4; ++col)
	{
		EXPECT_EQ(fb[col], 1);
	}
	for (size_t col = 4; col < 8; ++col)
	{
		EXPECT_EQ(fb[col], 0);
	}

	// 2. Draw same sprite at same location -> pixels XOR back to 0, collision detected (VF = 1)
	execute_instruction(chip8, 0xD011); // DRW V0, V1, 1

	EXPECT_EQ(chip8.get_register(0xF), 1);
	for (size_t col = 0; col < 8; ++col)
	{
		EXPECT_EQ(fb[col], 0);
	}
}

