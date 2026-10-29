cmake_minimum_required (VERSION 3.28)

# Enable Hot Reload for MSVC compilers if supported.
if(POLICY CMP0141)
	cmake_policy(SET CMP0141 NEW)
	set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(chip8emu LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.1.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

add_library(chip8_core STATIC
    src/chip8.cpp
)
target_include_directories(chip8_core PUBLIC src)
target_compile_features(chip8_core PUBLIC cxx_std_20)

add_executable(chip8emu
    src/main.cpp
    src/platform.cpp
)
target_include_directories(chip8emu PRIVATE src)
target_compile_features(chip8emu PRIVATE cxx_std_20)
target_link_libraries(chip8emu PRIVATE chip8_core SFML::Graphics SFML::Audio)

include(CTest)
if(BUILD_TESTING)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.18.0
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM
    )
    FetchContent_MakeAvailable(googletest)
    add_subdirectory(tests)
endif()
