# The worm tested as a class, without a library in the way; the seam tested deviceless; the
# library tested as the Grid loads it - through the one exported symbol and the table it
# returns; and, with the panel built, the window tested on Qt Test's own thread.

add_executable(worm_tests worm_tests.cpp)
target_link_libraries(worm_tests PRIVATE worm testing)
add_test(NAME worm_tests COMMAND worm_tests)

add_executable(body_tests body_tests.cpp)
target_link_libraries(body_tests PRIVATE worm testing)
add_test(NAME body_tests COMMAND body_tests)

add_executable(seam_tests seam_tests.cpp)
target_link_libraries(seam_tests PRIVATE worm testing)
add_test(NAME seam_tests COMMAND seam_tests)

add_executable(vtable_tests vtable_tests.cpp)
target_link_libraries(vtable_tests PRIVATE rc_worm program_abi testing)
add_test(NAME vtable_tests COMMAND vtable_tests)

# With the panel built, the library needs the Qt runtime of its kit at load - on the PATH for a
# test (Windows has no rpath), beside the Program for a deployment (the install rules and
# tools/check_deploy.py). The window test is a Qt Test executable with moc of its own.
if(RC_WORM_PANEL)
    if(WIN32)
        set_tests_properties(vtable_tests PROPERTIES
            ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
        )
    endif()

    # A plain executable with moc on, not qt_add_executable: under clang-cl the latter embeds a
    # side-by-side manifest Windows refuses to start ("side-by-side configuration is incorrect"),
    # and nothing it adds beyond that is needed by a test against a shared Qt.
    add_executable(tst_panel tst_panel.cpp)
    set_target_properties(tst_panel PROPERTIES AUTOMOC ON)
    target_link_libraries(tst_panel PRIVATE panel worm Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Test)
    # Plain text to stdout, asked for by name: Qt Test on Windows otherwise writes where a
    # captured console does not look, and a failure in CI would be a silent one.
    add_test(NAME tst_panel COMMAND tst_panel -o -,txt)
    if(WIN32)
        set_tests_properties(tst_panel PROPERTIES
            ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
        )
    endif()
endif()
