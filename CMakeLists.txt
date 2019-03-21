project(Nemu)
cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 11)

if(!MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()


add_subdirectory(deps/Sgl)

set(Nemu_SOURCE
	src/Nemu.cpp
    include/Nemu/CPU.h
    include/Nemu/PPU.h
    include/Nemu/MemoryPermission.h
    include/Nemu/NESInput.h
    include/Nemu/NESMemory.h
    include/Nemu/Stack.h
    include/Nemu/StatusRegister.h
    include/Nemu/System.h
    include/Nemu/Mapper/InternalNESMapper.h
    include/Nemu/Mapper/Mapper.h
    include/Nemu/Mapper/NROM128Mapper.h
    include/Nemu/Mapper/NROM256Mapper.h
)

add_executable(${PROJECT_NAME} ${Nemu_SOURCE})
target_link_libraries(${PROJECT_NAME} Sgl ${Sgl_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_LIST_DIR}/bin/Debug/${PROJECT_NAME}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_LIST_DIR}/bin/Release/${PROJECT_NAME}
)

add_subdirectory(test)