set(Nemu_TESTS 
    6502CPUFunctional
    NROM128ReadWrite
    StatusRegister
)

foreach (test ${Nemu_TESTS})
    add_executable(test-${test} Test.cpp)
    target_include_directories(test-${test} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_compile_definitions(test-${test} PRIVATE NEMU_TEST_FILE=\"${test}.h\")

    set_target_properties(test-${test} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/Debug/test
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/Release/test
    )    
endforeach()

# Required by 6502CPUFunctional
configure_file(
    ${CMAKE_SOURCE_DIR}/test/asm/6502_functional_test.bin
    # TODO: 
    #   Should work for all kinds of builds and not only debug.
    ${CMAKE_SOURCE_DIR}/bin/Debug/test/resources/6502_functional_test.bin
    COPYONLY
)
target_compile_definitions(
    test-6502CPUFunctional
    PRIVATE NEMU_6502CPUFUNCTIONAL_ASMFILE="${CMAKE_SOURCE_DIR}/bin/Debug/test/resources/6502_functional_test.bin"
)