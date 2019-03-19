project(Nemu)
cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 11)

if(!MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()


add_subdirectory(deps/Sgl)

set(sources
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
#NEMU LIB
add_library(NemuLib INTERFACE)
#target_sources(NemuLib INTERFACE ${sources})
target_include_directories(NemuLib INTERFACE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include> )


#NEMU BIN
add_executable(${PROJECT_NAME} src/Nemu.cpp)
target_link_libraries(${PROJECT_NAME} NemuLib Sgl ${SGL_LIBRARIES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_LIST_DIR}/bin/Debug/${PROJECT_NAME}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_LIST_DIR}/bin/Release/${PROJECT_NAME}
)

#TESTS
add_subdirectory(test)