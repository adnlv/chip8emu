#include <gtest/gtest.h>

#include "chip8.hpp"

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

TEST(Chip8AluTest, Opcode6xkkLoadsImmediateIntoRegister)
{
	Chip8 chip8;
	execute_instruction(chip8, 0x6142); // LD V1, 0x42

	EXPECT_EQ(chip8.get_register(1), 0x42);
	EXPECT_EQ(chip8.get_pc(), Chip8::IN_START_ADDR + 2);
}

TEST(Chip8AluTest, Opcode7xkkAddsImmediateWithoutAlteringCarryFlag)
{
	Chip8 chip8;
	chip8.set_register(2, 250);
	chip8.set_register(0xF, 0);

	execute_instruction(chip8, 0x7210); // ADD V2, 16 -> 266 % 256 = 10

	EXPECT_EQ(chip8.get_register(2), 10);
	EXPECT_EQ(chip8.get_register(0xF), 0); // Carry flag must NOT be modified
}

TEST(Chip8AluTest, Opcode8xy0CopiesVyIntoVx)
{
	Chip8 chip8;
	chip8.set_register(3, 0xAB);

	execute_instruction(chip8, 0x8230); // LD V2, V3

	EXPECT_EQ(chip8.get_register(2), 0xAB);
	EXPECT_EQ(chip8.get_register(3), 0xAB);
}

TEST(Chip8AluTest, Opcode8xy1PerformsBitwiseOr)
{
	Chip8 chip8;
	chip8.set_register(1, 0x0F);
	chip8.set_register(2, 0xF0);

	execute_instruction(chip8, 0x8121); // OR V1, V2

	EXPECT_EQ(chip8.get_register(1), 0xFF);
}

TEST(Chip8AluTest, Opcode8xy2PerformsBitwiseAnd)
{
	Chip8 chip8;
	chip8.set_register(1, 0x3C);
	chip8.set_register(2, 0x0F);

	execute_instruction(chip8, 0x8122); // AND V1, V2

	EXPECT_EQ(chip8.get_register(1), 0x0C);
}

TEST(Chip8AluTest, Opcode8xy3PerformsBitwiseXor)
{
	Chip8 chip8;
	chip8.set_register(1, 0xFF);
	chip8.set_register(2, 0x0F);

	execute_instruction(chip8, 0x8123); // XOR V1, V2

	EXPECT_EQ(chip8.get_register(1), 0xF0);
}

TEST(Chip8AluTest, Opcode8xy4AddsWithCarryWhenNoOverflow)
{
	Chip8 chip8;
	chip8.set_register(1, 50);
	chip8.set_register(2, 100);

	execute_instruction(chip8, 0x8124); // ADD V1, V2

	EXPECT_EQ(chip8.get_register(1), 150);
	EXPECT_EQ(chip8.get_register(0xF), 0);
}

TEST(Chip8AluTest, Opcode8xy4AddsWithCarryOnOverflow)
{
	Chip8 chip8;
	chip8.set_register(1, 200);
	chip8.set_register(2, 100);

	execute_instruction(chip8, 0x8124); // ADD V1, V2 -> 300 % 256 = 44, carry = 1

	EXPECT_EQ(chip8.get_register(1), 44);
	EXPECT_EQ(chip8.get_register(0xF), 1);
}

TEST(Chip8AluTest, Opcode8xy5SubtractsNoBorrowWhenVxGreaterOrEqualVy)
{
	Chip8 chip8;
	chip8.set_register(1, 100);
	chip8.set_register(2, 40);

	execute_instruction(chip8, 0x8125); // SUB V1, V2 -> 60, not_borrow = 1

	EXPECT_EQ(chip8.get_register(1), 60);
	EXPECT_EQ(chip8.get_register(0xF), 1);
}

TEST(Chip8AluTest, Opcode8xy5SubtractsBorrowWhenVxLessThanVy)
{
	Chip8 chip8;
	chip8.set_register(1, 40);
	chip8.set_register(2, 100);

	execute_instruction(chip8, 0x8125); // SUB V1, V2 -> 196, not_borrow = 0

	EXPECT_EQ(chip8.get_register(1), 196);
	EXPECT_EQ(chip8.get_register(0xF), 0);
}

TEST(Chip8AluTest, Opcode8xy6ShiftsRightSavingLsbInVf)
{
	Chip8 chip8;
	chip8.set_register(1, 0x05); // binary 00000101 -> lsb = 1, result = 2

	execute_instruction(chip8, 0x8106); // SHR V1

	EXPECT_EQ(chip8.get_register(1), 0x02);
	EXPECT_EQ(chip8.get_register(0xF), 1);

	chip8.set_pc(Chip8::IN_START_ADDR);
	chip8.set_register(1, 0x06); // binary 00000110 -> lsb = 0, result = 3

	execute_instruction(chip8, 0x8106); // SHR V1

	EXPECT_EQ(chip8.get_register(1), 0x03);
	EXPECT_EQ(chip8.get_register(0xF), 0);
}

TEST(Chip8AluTest, Opcode8xy7SubnSubtractsNoBorrowWhenVyGreaterOrEqualVx)
{
	Chip8 chip8;
	chip8.set_register(1, 40);
	chip8.set_register(2, 100);

	execute_instruction(chip8, 0x8127); // SUBN V1, V2 -> V2 - V1 = 60, not_borrow = 1

	EXPECT_EQ(chip8.get_register(1), 60);
	EXPECT_EQ(chip8.get_register(0xF), 1);
}

TEST(Chip8AluTest, Opcode8xy7SubnSubtractsBorrowWhenVyLessThanVx)
{
	Chip8 chip8;
	chip8.set_register(1, 100);
	chip8.set_register(2, 40);

	execute_instruction(chip8, 0x8127); // SUBN V1, V2 -> V2 - V1 = 196, not_borrow = 0

	EXPECT_EQ(chip8.get_register(1), 196);
	EXPECT_EQ(chip8.get_register(0xF), 0);
}

TEST(Chip8AluTest, Opcode8xyEShiftsLeftSavingMsbInVf)
{
	Chip8 chip8;
	chip8.set_register(1, 0x85); // msb = 1, (0x85 << 1) & 0xFF = 0x0A

	execute_instruction(chip8, 0x810E); // SHL V1

	EXPECT_EQ(chip8.get_register(1), 0x0A);
	EXPECT_EQ(chip8.get_register(0xF), 1);

	chip8.set_pc(Chip8::IN_START_ADDR);
	chip8.set_register(1, 0x45); // msb = 0, (0x45 << 1) & 0xFF = 0x8A

	execute_instruction(chip8, 0x810E); // SHL V1

	EXPECT_EQ(chip8.get_register(1), 0x8A);
	EXPECT_EQ(chip8.get_register(0xF), 0);
}

TEST(Chip8AluTest, OpcodeAnnnLoadsAddressIntoIndexRegister)
{
	Chip8 chip8;
	execute_instruction(chip8, 0xA456); // LD I, 0x456

	EXPECT_EQ(chip8.get_i(), 0x456);
}

TEST(Chip8AluTest, OpcodeCxkkGeneratesMaskedRandomByteWithSeededEngine)
{
	// With fixed seed, execution is deterministic
	Chip8 chip8_a(12345);
	Chip8 chip8_b(12345);

	execute_instruction(chip8_a, 0xC10F); // RND V1, 0x0F
	execute_instruction(chip8_b, 0xC10F); // RND V1, 0x0F

	EXPECT_EQ(chip8_a.get_register(1), chip8_b.get_register(1));
	EXPECT_LE(chip8_a.get_register(1), 0x0F);
}
