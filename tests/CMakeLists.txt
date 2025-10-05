find_package(Catch2 3 QUIET)

if(NOT Catch2_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.10.0
    )
    FetchContent_MakeAvailable(Catch2)
endif()

include(Catch)

add_executable(test_basic test_basic.cpp)
target_link_libraries(test_basic
    PRIVATE
        audio_queue
        Catch2::Catch2WithMain
)

set_target_properties(test_basic PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

if(TARGET Catch2::Catch2WithMain)
    target_include_directories(test_basic
        PRIVATE
            $<TARGET_PROPERTY:Catch2::Catch2WithMain,INTERFACE_INCLUDE_DIRECTORIES>
    )
endif()

catch_discover_tests(test_basic)
