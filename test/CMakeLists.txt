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

        target_include_directories(${test} PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include> )

		set_target_properties(${test} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/Debug/${test}
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/Release/${test}
)
endforeach()

