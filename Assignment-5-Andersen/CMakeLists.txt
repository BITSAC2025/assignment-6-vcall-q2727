add_library(a5lib A5Lib.cpp)

add_executable(andersen Andersen.cpp)
target_link_libraries(andersen PRIVATE
        ${SVF_LIB}
        ${LLVM_LIB}
        a5lib
        )
set_target_properties(andersen PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})