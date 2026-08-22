# The Program: one shared library the Grid loads by name - `rc_worm.dll`, `librc_worm.so` - with
# one exported symbol. The worm itself is a static library of vanilla C++20 beneath it, so the
# tests reach the class without a DLL in the way and the panel (when built) is one more thing the
# shared library links, not something the worm knows about.

add_library(worm STATIC
    worm.cpp
)

target_include_directories(worm PUBLIC .)
target_link_libraries(worm PUBLIC program_abi)
target_compile_features(worm PUBLIC cxx_std_20)
set_target_properties(worm PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(rc_worm SHARED
    abi.cpp
)

target_link_libraries(rc_worm PRIVATE worm)
if(RC_WORM_PANEL)
    target_link_libraries(rc_worm PRIVATE panel)
    target_compile_definitions(rc_worm PRIVATE RC_WORM_HAS_PANEL=1)
endif()
target_compile_features(rc_worm PRIVATE cxx_std_20)

# Only tglGetProgramVTable leaves the library: on Windows by the header's dllexport, on Linux by
# hiding everything else - a Program with a second exported symbol is a Program with a side door.
set_target_properties(rc_worm PROPERTIES
    PREFIX "${RC_WORM_LIBRARY_PREFIX}"
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)
