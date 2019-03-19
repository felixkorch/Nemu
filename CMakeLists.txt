project(Nemu)
cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 11)

if(!MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()


add_subdirectory(deps/Sgl)

set(NemuLib_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Nemu/CPU.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Nemu/PPU.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Nemu/MemoryPermission.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Nemu/NESInput.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Nemu/NESMemory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Nemu/Stack.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Nemu/StatusRegister.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Nemu/System.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Nemu/Mapper/InternalNESMapper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Nemu/Mapper/Mapper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Nemu/Mapper/NROM128Mapper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Nemu/Mapper/NROM256Mapper.h
)
#NEMU LIB
add_library(NemuLib INTERFACE)
target_sources(NemuLib INTERFACE ${NemuLib_SOURCE})
target_include_directories(NemuLib INTERFACE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include> )


#NEMU BIN
add_executable(${PROJECT_NAME} src/Nemu.cpp)
target_link_libraries(${PROJECT_NAME} NemuLib Sgl ${SGL_LIBRARIES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_LIST_DIR}/bin/Debug/${PROJECT_NAME}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_LIST_DIR}/bin/Release/${PROJECT_NAME}
)

add_subdirectory(test)