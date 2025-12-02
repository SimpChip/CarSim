file(GLOB LOGICTEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(logictests ${LOGICTEST_SOURCES})

target_link_libraries(logictests
        PRIVATE
        Catch2::Catch2WithMain
        threepp::threepp
        logic collision entities controllers
)

include(Catch)
catch_discover_tests(logictests)

