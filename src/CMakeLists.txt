add_library(reflection INTERFACE)
target_include_directories(reflection INTERFACE ${CMAKE_SOURCE_DIR}/include/)

add_subdirectory(Serialization)
# add_subdirectory(LuaBinding)