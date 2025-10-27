add_library(cfga_lib cfga_lib.cpp)

add_executable(cfga CFGA.cpp)
target_link_libraries(cfga PRIVATE
        ${SVF_LIB}
        ${LLVM_LIB}
        cfga_lib
        )
set_target_properties(cfga PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})