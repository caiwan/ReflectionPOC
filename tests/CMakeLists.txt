include(CTest)
enable_testing()

find_package(GTest CONFIG)

file(GLOB_RECURSE SOURCE_FILES *.cpp)
file(GLOB_RECURSE HEADER_FILES *.h)

add_executable(UnitTests EXCLUDE_FROM_ALL ${SOURCE_FILES} ${HEADER_FILES})

add_test(NAME UnitTests
  COMMAND $<TARGET_FILE:UnitTests>
  WORKING_DIRECTORY ${TEST_DEPLOY_DIR}
)

target_link_libraries(UnitTests PUBLIC
    Grafkit::reflection
	Grafkit::serialization
    GTest::GTest
)
