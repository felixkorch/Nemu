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
        add_executable("test.${test}" ${test}.cpp)

        target_include_directories("test.${test}" PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include> )

		set_target_properties(${PROJECT_NAME} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/Debug/${PROJECT_NAME}
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/Release/${PROJECT_NAME}
)
endforeach()

