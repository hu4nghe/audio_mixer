file(GLOB input_module_base_src
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(input_module_base STATIC ${input_module_base_src})

set_target_properties(input_module_base PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(input_module_base
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(input_module_base
    PUBLIC
        audio_queue
)