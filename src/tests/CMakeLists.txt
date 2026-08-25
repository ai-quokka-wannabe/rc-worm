# The worm tested as a class, without a library in the way; and the library tested as the Grid
# loads it - through the one exported symbol and the table it returns.
add_executable(worm_tests worm_tests.cpp)
target_link_libraries(worm_tests PRIVATE worm testing)
add_test(NAME worm_tests COMMAND worm_tests)

add_executable(body_tests body_tests.cpp)
target_link_libraries(body_tests PRIVATE worm testing)
add_test(NAME body_tests COMMAND body_tests)

add_executable(vtable_tests vtable_tests.cpp)
target_link_libraries(vtable_tests PRIVATE rc_worm program_abi testing)
add_test(NAME vtable_tests COMMAND vtable_tests)

# With the panel built, the library needs the Qt runtime of its kit at load - on the PATH for a
# test, beside the Program for a deployment (Etape 4 settles the latter).
if(RC_WORM_PANEL AND WIN32)
    set_tests_properties(vtable_tests PROPERTIES
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
    )
endif()
