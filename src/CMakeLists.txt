project(Nemu)

set(sources
    Nemu.cpp
)

find_package(Threads REQUIRED)

add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES} Sgl Threads::Threads)