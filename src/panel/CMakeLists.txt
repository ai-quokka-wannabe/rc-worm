# The panel: the one place Qt lives. Etape 2 builds it against Qt6::Core on every kit so the kits
# are proven; the window itself is Etape 4.

add_library(panel STATIC
    panel.cpp
)

target_include_directories(panel PUBLIC include)
target_link_libraries(panel PRIVATE Qt6::Core)
target_compile_features(panel PUBLIC cxx_std_20)
set_target_properties(panel PROPERTIES POSITION_INDEPENDENT_CODE ON)

message(STATUS "Panel: Qt ${Qt6_VERSION} from ${Qt6_DIR}")
