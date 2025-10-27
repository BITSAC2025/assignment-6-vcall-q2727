add_library(a6lib A6Lib.cpp)

add_executable(vcall VCall.cpp)
target_link_libraries(vcall PRIVATE
        ${SVF_LIB}
        ${LLVM_LIB}
        a6lib
        )
set_target_properties(vcall PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})