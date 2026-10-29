#include <gtest/gtest.h>
#include <vector>

#include "chip8.hpp"
#include "fontset.hpp"

TEST(Chip8InitTest, DefaultRegistersAndPointersAreZeroedOrExpected)
{
	const Chip8 chip8;

	EXPECT_EQ(chip8.get_pc(), Chip8::IN_START_ADDR);
	EXPECT_EQ(chip8.get_sp(), 0);
	EXPECT_EQ(chip8.get_i(), 0);
	EXPECT_EQ(chip8.get_delay_timer(), 0);
	EXPECT_EQ(chip8.get_sound_timer(), 0);
	EXPECT_FALSE(chip8.is_sound_playing());
	EXPECT_FALSE(chip8.draw_flag());

	for (uint8_t i = 0; i < Chip8::NUM_REGISTERS; ++i)
	{
		EXPECT_EQ(chip8.get_register(i), 0);
	}

	for (uint8_t i = 0; i < Chip8::STACK_CAP; ++i)
	{
		EXPECT_EQ(chip8.get_stack(i), 0);
	}

	for (uint8_t i = 0; i < Chip8::KEYPAD_CAP; ++i)
	{
		EXPECT_FALSE(chip8.is_key_pressed(i));
	}
}

TEST(Chip8InitTest, FontsetIsCorrectlyLoadedInMemory)
{
	const Chip8 chip8;

	for (size_t i = 0; i < FONTSET_CAP; ++i)
	{
		EXPECT_EQ(chip8.get_memory(static_cast<uint16_t>(FONTSET_START_ADDR + i)), FONTSET[i])
		    << "Fontset byte mismatch at index " << i;
	}
}

TEST(Chip8InitTest, LoadRomFromSpanSuccess)
{
	Chip8 chip8;
	const std::vector<uint8_t> test_rom = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC};

	EXPECT_TRUE(chip8.load_rom(test_rom));

	for (size_t i = 0; i < test_rom.size(); ++i)
	{
		EXPECT_EQ(chip8.get_memory(static_cast<uint16_t>(Chip8::IN_START_ADDR + i)), test_rom[i]);
	}
}

TEST(Chip8InitTest, LoadRomRejectsEmptySpan)
{
	Chip8 chip8;
	EXPECT_FALSE(chip8.load_rom(std::span<const uint8_t>{}));
}

TEST(Chip8InitTest, LoadRomRejectsOversizedSpan)
{
	Chip8 chip8;
	const std::vector<uint8_t> oversized_rom(Chip8::MAX_ROM_SIZE + 1, 0xFF);
	EXPECT_FALSE(chip8.load_rom(oversized_rom));
}

