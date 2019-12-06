project(Nemu)
cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 17)

if(!MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${SDL2_TTF_INCLUDE_DIRS})

set(Nemu_SOURCE
    src/Nemu/Main.cpp
    src/Nemu/Utilities.h
    src/Nemu/GUI.h
    src/Nemu/Input.h
    src/Nemu/Mapper/CPUMapper.h
    src/Nemu/Mapper/PPUMapper.h
    src/Nemu/Mapper/NROM128Mapper.h
    src/Nemu/Mapper/NROM256Mapper.h
    src/Nemu/Mapper/UxROMMapper.h
    src/Nemu/Mapper/MMC1Mapper.h
    src/Nemu/Mapper/MMC3Mapper.h
    src/Nemu/Core/CPU.h
    src/Nemu/Core/Joypad.h
    src/Nemu/Core/NESInput.h
    src/Nemu/Core/NESInstance.h
    src/Nemu/Core/PPU.h
    src/Nemu/Core/ROMLayout.h
    src/Nemu/Core/StatusRegister.h
)

add_executable(${PROJECT_NAME} ${Nemu_SOURCE})

target_compile_features(Nemu PUBLIC cxx_std_17)

target_include_directories(${PROJECT_NAME}
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src>
)
target_link_libraries(${PROJECT_NAME} PUBLIC ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_LIST_DIR}/bin/Debug/${PROJECT_NAME}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_LIST_DIR}/bin/Release/${PROJECT_NAME}
)

if (NEMU_ENABLE_TESTS)
    add_subdirectory(test)
endif()
