set(NEMU_TESTS 
        # EasyAdd 
        # CPUStackPush
        # EasyAnd
        # MemoryPermission
        NESMemoryReadAndWrite
        NESMemory16Read
        FlagRegister
        NROM128ReadWrite
)

foreach (test ${NEMU_TESTS})
        add_executable(${test} ${test}.cpp)
		target_link_libraries(${test} NemuLib)
		set_target_properties(${test} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/Debug/${test}
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/Release/${test}
)
endforeach()

