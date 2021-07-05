include(AssignSourceGroup)

project(
	serialization
	VERSION 0.0.1
	LANGUAGES C CXX
)

file(GLOB_RECURSE REFLECTION_HEADER_FILES ${CMAKE_SOURCE_DIR}/include/Serialization/*.h)
file(GLOB_RECURSE REFLECTION_SOURCE_FILES *.cpp)

find_package(nlohmann_json CONFIG)

add_library(${PROJECT_NAME} STATIC ${REFLECTION_SOURCE_FILES} ${REFLECTION_HEADER_FILES})

# add_dependencies(${PROJECT_NAME} INTERFACE reflection)

target_include_directories(${PROJECT_NAME}
	PUBLIC ${CMAKE_SOURCE_DIR}/include
	)
	
target_link_libraries(${PROJECT_NAME}
	PUBLIC nlohmann_json::nlohmann_json
	INTERFACE reflection
	)
	
	
assign_source_group(${REFLECTION_SOURCE_FILES} ${REFLECTION_HEADER_FILES})
