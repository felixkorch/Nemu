set(NEMU_TESTS
    6502CPUFunctional
    NROM128ReadWrite
    StatusRegister
)

set(NEMU_TEST_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/test)

foreach (test ${NEMU_TESTS})
    set(NEMU_TEST_TARGET Test${test})

    add_executable(${NEMU_TEST_TARGET} Test.cpp)
    target_include_directories(${NEMU_TEST_TARGET} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_compile_definitions(${NEMU_TEST_TARGET} PRIVATE NEMU_TEST_FILE="${test}.h")

    set_target_properties(${NEMU_TEST_TARGET} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${NEMU_TEST_OUTPUT_DIR}
    )    
endforeach()

# Required by 6502CPUFunctional
configure_file(
    ${CMAKE_SOURCE_DIR}/test/asm/6502_functional_test.bin
    ${NEMU_TEST_OUTPUT_DIR}/resources/6502_functional_test.bin
    COPYONLY
)
target_compile_definitions(
    Test6502CPUFunctional
    PRIVATE NEMU_6502CPUFUNCTIONAL_ASMFILE="${NEMU_TEST_OUTPUT_DIR}/resources/6502_functional_test.bin"
)