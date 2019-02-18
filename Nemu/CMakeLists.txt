project(Nemu)

set(sources
    Source/pch.cpp
    Source/Core/CPU.cpp
    Source/Core/Nemu.cpp
)

add_executable(${PROJECT_NAME} ${sources})
target_include_directories(${PROJECT_NAME} PRIVATE Source)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/Debug/${PROJECT_NAME}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/Release/${PROJECT_NAME}
)