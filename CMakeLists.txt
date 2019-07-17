project(Nemu)
cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 17)

if(!MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()

add_subdirectory("ext/GLFW")
add_subdirectory("ext/Glad")

set(Nemu_SOURCE
    src/Nemu/Main.cpp
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
    src/Nemu/Core/Utilities.h
	src/Nemu/Graphics/Event.h
	src/Nemu/Graphics/Input.h
	src/Nemu/Graphics/OpenGL.h
	src/Nemu/Graphics/Shader.h
	src/Nemu/Graphics/Texture.h
	src/Nemu/Graphics/Window.h
)

add_executable(${PROJECT_NAME} ${Nemu_SOURCE})
target_include_directories(${PROJECT_NAME}
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/ext/glm>
	${GLFW_INCLUDE_DIRS}
)
target_link_libraries(${PROJECT_NAME} PUBLIC glfw ${GLFW_LIBRARIES} Glad)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_LIST_DIR}/bin/Debug/${PROJECT_NAME}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_LIST_DIR}/bin/Release/${PROJECT_NAME}
)

if (NEMU_ENABLE_TESTS)
    add_subdirectory(test)
endif()
