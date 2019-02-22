set(NEMU_TESTS 
        EasyAdd 
        MaskIteratorRead 
        MaskIteratorWrite
        MemoryPermission
        NESMemoryReadAndWrite
)

foreach (test ${NEMU_TESTS})
        add_executable("test.${test}" ${test}.cpp)
        target_include_directories("test.${test}"
                PUBLIC "${CMAKE_SOURCE_DIR}/Nemu/include"
        )
        # add_library(${PROJECT_NAME} ${sources})
endforeach()

