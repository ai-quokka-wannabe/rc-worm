# The Program: one shared library the Grid loads by name - `rc_worm.dll`, `librc_worm.so` - with
# one exported symbol. The worm itself is a static library of vanilla C++20 beneath it, so the
# tests reach the class without a DLL in the way and the panel (when built) is one more thing the
# shared library links, not something the worm knows about.

add_library(worm STATIC
    body.cpp
    gait.cpp
    seam.cpp
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
# On Linux the Qt runtime is looked for beside the library first ($ORIGIN), then the system's.
set_target_properties(rc_worm PROPERTIES
    PREFIX "${RC_WORM_LIBRARY_PREFIX}"
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    INSTALL_RPATH "$ORIGIN"
)

# Deployment: the Program lands in `programs/`, which is where the Grid looks, and on Windows the
# Qt runtime it needs lands beside it - windeployqt, told the DLL, with the compiler runtime's
# redistributable and without the translations, the software OpenGL and the plugin families
# (image formats, icon engines, network, TLS, touch) a panel of labels and sliders never loads.
# What it produced is judged by tools/check_deploy.py: every import beside the worm or the
# system's, the platform plugin and the style included.
install(TARGETS rc_worm
    RUNTIME DESTINATION programs
    LIBRARY DESTINATION programs
)
if(RC_WORM_PANEL AND WIN32)
    get_target_property(RC_WORM_QMAKE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(RC_WORM_QT_BIN "${RC_WORM_QMAKE}" DIRECTORY)
    find_program(RC_WORM_WINDEPLOYQT windeployqt HINTS "${RC_WORM_QT_BIN}" REQUIRED)
    install(CODE "
        message(STATUS \"windeployqt: \${CMAKE_INSTALL_PREFIX}/programs/$<TARGET_FILE_NAME:rc_worm>\")
        execute_process(
            COMMAND \"${RC_WORM_WINDEPLOYQT}\" --compiler-runtime --no-translations --no-opengl-sw --no-system-d3d-compiler
                    --skip-plugin-types generic,networkinformation,tls,imageformats,iconengines
                    --dir \"\${CMAKE_INSTALL_PREFIX}/programs\" \"\${CMAKE_INSTALL_PREFIX}/programs/$<TARGET_FILE_NAME:rc_worm>\"
            RESULT_VARIABLE deploy_result
        )
        if(NOT deploy_result EQUAL 0)
            message(FATAL_ERROR \"windeployqt failed with \${deploy_result}\")
        endif()
    ")
endif()
