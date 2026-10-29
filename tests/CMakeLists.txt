add_executable(chip8_tests
    test_chip8_init.cpp
    test_chip8_opcodes_alu.cpp
    test_chip8_opcodes_flow.cpp
    test_chip8_peripherals.cpp
)
target_link_libraries(chip8_tests PRIVATE chip8_core GTest::gtest_main)
target_compile_features(chip8_tests PRIVATE cxx_std_20)

include(GoogleTest)
gtest_discover_tests(chip8_tests)
