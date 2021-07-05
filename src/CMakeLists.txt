add_library(reflection INTERFACE)
target_include_directories(reflection
	INTERFACE
		$<INSTALL_INTERFACE:include>
		$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
	)

add_library(Grafkit::reflection ALIAS reflection)

install(TARGETS reflection
    EXPORT ${ROOT_PROJECT}-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES "${CMAKE_SOURCE_DIR}/include/refl.h" DESTINATION "include")

add_subdirectory(Serialization)
# add_subdirectory(LuaBinding)

