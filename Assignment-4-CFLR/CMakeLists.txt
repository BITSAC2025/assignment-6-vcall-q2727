add_library(a4lib A4Lib.cpp)

add_executable(cflr CFLR.cpp)
target_link_libraries(cflr PRIVATE
        ${SVF_LIB}
        ${LLVM_LIB}
        a4lib
        )
set_target_properties(cflr PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})