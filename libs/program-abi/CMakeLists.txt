# The Program ABI, vendored verbatim from the flagship (tron-grid-lite/libs/program-abi): the header
# and its fingerprint. The flagship moves it; this repository re-vendors and never edits. A header
# and nothing else, so an INTERFACE target with no compile features - the file compiles as C or C++
# under any standard, and a target that pinned one would quietly stop being true of the other.

add_library(program_abi INTERFACE)

target_include_directories(program_abi
    INTERFACE include/
)

add_subdirectory(tests)
