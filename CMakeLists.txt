cmake_minimum_required(VERSION 3.15)

project(
	ReflectionUtils
	VERSION 0.0.1
	LANGUAGES C CXX
)

set(ROOT_PROJECT ${PROJECT_NAME})

# --- Project and build options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# --- Adjust paths

set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/" ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR} ${CMAKE_PREFIX_PATH})

option(CONAN_PROFILE "Conan profile" "default")

# --- Install conan dependencies
if (CONAN_EXPORTED) # in conan local cache
	# standard conan installation, deps will be defined in conanfile.py
	# and not necessary to call conan again, conan is already running
	include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
	conan_basic_setup()

else ()
	if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
		message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
		file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/develop/conan.cmake"
			"${CMAKE_BINARY_DIR}/conan.cmake")
	endif ()
	include(${CMAKE_BINARY_DIR}/conan.cmake)

	# conan_add_remote(NAME jinncrafters INDEX 1 URL https://api.bintray.com/conan/jinncrafters/conan)

	conan_cmake_run(CONANFILE conanfile.txt
        PROFILE ${CONAN_PROFILE}
		BUILD missing
		BASIC_SETUP
		CMAKE_TARGETS
		CONFIGURATION_TYPES "Release;Debug;RelWithDebInfo;MinSizeRel"
		)
endif ()

# ---
add_subdirectory(src)

# -- Create empty install target for internal meta targets
install(TARGETS project_warnings EXPORT ${ROOT_PROJECT}-targets)
install(TARGETS project_no_warnings EXPORT ${ROOT_PROJECT}-targets)

# --- Install targets
include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/${ROOT_PROJECT})

install(EXPORT ${ROOT_PROJECT}-targets
    FILE
		${ROOT_PROJECT}Targets.cmake
    NAMESPACE
		${ROOT_PROJECT}::
    DESTINATION
        ${INSTALL_CONFIGDIR}
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${ROOT_PROJECT}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/CMakeConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${ROOT_PROJECT}Config.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

install(FILES
	# TODO: This needs nlomann JSON
    # ${CMAKE_CURRENT_LIST_DIR}/cmake/FindRapidJSON.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${ROOT_PROJECT}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${ROOT_PROJECT}ConfigVersion.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)

# TODO: This needs nlomann JSON
# configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/FindRapidJSON.cmake
#     ${CMAKE_CURRENT_BINARY_DIR}/FindRapidJSON.cmake
#     COPYONLY)

export(EXPORT ${ROOT_PROJECT}-targets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/${ROOT_PROJECT}Targets.cmake
    NAMESPACE ${ROOT_PROJECT}::)

export(PACKAGE ${ROOT_PROJECT})

# --- Invoke CPack
set(_fmt TGZ)
if(WIN32)
  set(_fmt ZIP)
endif()

set(CPACK_GENERATOR ${_fmt})
set(CPACK_SOURCE_GENERATOR ${_fmt})
set(CPACK_PACKAGE_VENDOR "Caiwan")
set(CPACK_PACKAGE_CONTACT "Caiwan")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_OUTPUT_FILE_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/package")
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# not .gitignore as its regex syntax is distinct
file(READ ${CMAKE_SOURCE_DIR}/.cpack_ignore _cpack_ignore)
string(REGEX REPLACE "\n" ";" _cpack_ignore ${_cpack_ignore})
set(CPACK_SOURCE_IGNORE_FILES "${_cpack_ignore}")

install(FILES ${CPACK_RESOURCE_FILE_README} ${CPACK_RESOURCE_FILE_LICENSE}
  DESTINATION share/docs/${ROOT_PROJECT})

include(CPack)

# --- Add tests
OPTION(${ROOT_PROJECT}_ENABLE_TESTING "Enable tests to build and execute" ON)
IF (${ROOT_PROJECT}_ENABLE_TESTING)
	add_subdirectory(tests)
ENDIF(${ROOT_PROJECT}_ENABLE_TESTING)
