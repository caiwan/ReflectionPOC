include(AssignSourceGroup)

file(GLOB_RECURSE REFLECTION_HEADER_FILES *.h)
file(GLOB_RECURSE REFLECTION_SOURCE_FILES *.cpp)

find_package(nlohmann_json CONFIG)

add_library(Reflection STATIC ${REFLECTION_SOURCE_FILES} ${REFLECTION_HEADER_FILES})

target_include_directories(Reflection
	PUBLIC ./
	)
	
target_link_libraries(Reflection
  PUBLIC
	nlohmann_json::nlohmann_json)
	
	
assign_source_group(${REFLECTION_SOURCE_FILES} ${REFLECTION_HEADER_FILES})
