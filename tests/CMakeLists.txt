project(async_timer_test VERSION 0.0.0.1 LANGUAGES CXX)

link_libraries(
    GTest::GTest
    GTest::Main
    Threads::Threads
)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests)
add_executable(${PROJECT_NAME} 
    AsyncTimerTest.cpp
)

target_include_directories(${PROJECT_NAME}
PRIVATE
    ${CMAKE_SOURCE_DIR}/src/
)

target_link_libraries(${PROJECT_NAME}
PRIVATE
    async_timer
)

add_test(
    NAME ${PROJECT_NAME}
    COMMAND ${PROJECT_NAME} "--gtest_output=xml:reports/TestReport_${PROJECT_NAME}.xml" "--gtest_filter=*.*"
    WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")