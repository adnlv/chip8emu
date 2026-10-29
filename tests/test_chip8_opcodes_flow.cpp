#include <gtest/gtest.h>

#include "chip8.hpp"

namespace
{
	void execute_instruction(Chip8 &chip8, uint16_t opcode)
	{
		const uint16_t pc = chip8.get_pc();
		chip8.write_memory(pc, static_cast<uint8_t>(opcode >> 8));
		chip8.write_memory(static_cast<uint16_t>(pc + 1), static_cast<uint8_t>(opcode & 0xFF));
		chip8.cycle();
	}
}

TEST(Chip8FlowTest, Opcode1nnnJumpsToSpecifiedAddress)
{
	Chip8 chip8;
	execute_instruction(chip8, 0x1500); // JP 0x500

	EXPECT_EQ(chip8.get_pc(), 0x500);
}

TEST(Chip8FlowTest, Opcode2nnnAnd00EECallsSubroutineAndReturns)
{
	Chip8 chip8;
	// Call subroutine at 0x350
	execute_instruction(chip8, 0x2350);

	EXPECT_EQ(chip8.get_pc(), 0x350);
	EXPECT_EQ(chip8.get_sp(), 1);
	EXPECT_EQ(chip8.get_stack(0), Chip8::IN_START_ADDR + 2); // Return address

	// Return from subroutine
	execute_instruction(chip8, 0x00EE);

	EXPECT_EQ(chip8.get_pc(), Chip8::IN_START_ADDR + 2);
	EXPECT_EQ(chip8.get_sp(), 0);
}

TEST(Chip8FlowTest, Opcode3xkkSkipsNextInstructionWhenVxEqualsImmediate)
{
	Chip8 chip8;
	chip8.set_register(1, 0x55);

	execute_instruction(chip8, 0x3155); // SE V1, 0x55

	EXPECT_EQ(chip8.get_pc(), Chip8::IN_START_ADDR + 4); // Skipped
}

TEST(Chip8FlowTest, Opcode3xkkDoesNotSkipWhenVxDoesNotEqualImmediate)
{
	Chip8 chip8;
	chip8.set_register(1, 0x55);

	execute_instruction(chip8, 0x3156); // SE V1, 0x56

	EXPECT_EQ(chip8.get_pc(), Chip8::IN_START_ADDR + 2); // Normal step
}

TEST(Chip8FlowTest, Opcode4xkkSkipsNextInstructionWhenVxNotEqualsImmediate)
{
	Chip8 chip8;
	chip8.set_register(1, 0x55);

	execute_instruction(chip8, 0x4156); // SNE V1, 0x56

	EXPECT_EQ(chip8.get_pc(), Chip8::IN_START_ADDR + 4); // Skipped
}

TEST(Chip8FlowTest, Opcode4xkkDoesNotSkipWhenVxEqualsImmediate)
{
	Chip8 chip8;
	chip8.set_register(1, 0x55);

	execute_instruction(chip8, 0x4155); // SNE V1, 0x55

	EXPECT_EQ(chip8.get_pc(), Chip8::IN_START_ADDR + 2); // Normal step
}

TEST(Chip8FlowTest, Opcode5xy0SkipsNextInstructionWhenVxEqualsVy)
{
	Chip8 chip8;
	chip8.set_register(1, 0x42);
	chip8.set_register(2, 0x42);

	execute_instruction(chip8, 0x5120); // SE V1, V2

	EXPECT_EQ(chip8.get_pc(), Chip8::IN_START_ADDR + 4);
}

TEST(Chip8FlowTest, Opcode5xy0DoesNotSkipWhenVxNotEqualsVy)
{
	Chip8 chip8;
	chip8.set_register(1, 0x42);
	chip8.set_register(2, 0x43);

	execute_instruction(chip8, 0x5120); // SE V1, V2

	EXPECT_EQ(chip8.get_pc(), Chip8::IN_START_ADDR + 2);
}

TEST(Chip8FlowTest, Opcode9xy0SkipsNextInstructionWhenVxNotEqualsVy)
{
	Chip8 chip8;
	chip8.set_register(1, 0x42);
	chip8.set_register(2, 0x43);

	execute_instruction(chip8, 0x9120); // SNE V1, V2

	EXPECT_EQ(chip8.get_pc(), Chip8::IN_START_ADDR + 4);
}

TEST(Chip8FlowTest, Opcode9xy0DoesNotSkipWhenVxEqualsVy)
{
	Chip8 chip8;
	chip8.set_register(1, 0x42);
	chip8.set_register(2, 0x42);

	execute_instruction(chip8, 0x9120); // SNE V1, V2

	EXPECT_EQ(chip8.get_pc(), Chip8::IN_START_ADDR + 2);
}

TEST(Chip8FlowTest, OpcodeBnnnJumpsToAddressPlusV0)
{
	Chip8 chip8;
	chip8.set_register(0, 0x12);

	execute_instruction(chip8, 0xB300); // JP V0, 0x300 -> 0x300 + 0x12 = 0x312

	EXPECT_EQ(chip8.get_pc(), 0x312);
}
