project(Glad)

add_library(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/src/glad.c)
target_include_directories(${PROJECT_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
            
)