if(RC_WORM_PANEL)
    # Found here rather than in the panel's own directory so the tests, a sibling, can name the
    # Qt runtime the library needs at load.
    find_package(Qt6 REQUIRED COMPONENTS Core)
    add_subdirectory(panel)
endif()
add_subdirectory(worm)
add_subdirectory(tests)
