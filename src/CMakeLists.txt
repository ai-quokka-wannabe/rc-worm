if(RC_WORM_PANEL)
    # Found here rather than in the panel's own directory so the tests, a sibling, can name the
    # Qt runtime the library needs at load and link Qt Test for the panel's own tests.
    find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Test)
endif()

# The worm first: the panel reads its seam, the worm knows nothing of the panel.
add_subdirectory(worm)
if(RC_WORM_PANEL)
    add_subdirectory(panel)
endif()
add_subdirectory(tests)
