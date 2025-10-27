add_executable(svfir SVFIR.cpp)
target_link_libraries(svfir PRIVATE
        ${SVF_LIB}
        ${LLVM_LIB}
        )
set_target_properties(svfir PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})