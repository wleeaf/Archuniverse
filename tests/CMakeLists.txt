# doctest's implementation compiled exactly once, shared by all test binaries.
add_library(doctest_main STATIC doctest_main.cpp)
target_include_directories(doctest_main PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/doctest)

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp)

add_executable(archuniverse_tests ${TEST_SOURCES})
target_link_libraries(archuniverse_tests PRIVATE archuniverse doctest_main)
target_include_directories(archuniverse_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/third_party/doctest)

add_test(NAME unit COMMAND archuniverse_tests)
