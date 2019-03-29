set(Nemu_TESTS 
        # EasyAdd 
        # CPUStackPush
        # EasyAnd
        # MemoryPermission
        NESMemoryReadAndWrite
        NESMemory16Read
        FlagRegister
        NROM128ReadWrite
		Sandbox
)

foreach (test ${Nemu_TESTS})
        add_executable(${test} ${test}.cpp)
		target_include_directories(${test} PRIVATE ${CMAKE_SOURCE_DIR}/include)
		set_target_properties(${test} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/Debug/${test}
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/Release/${test}
)
endforeach()

