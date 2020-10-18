cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
enable_testing()

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/../../manifest.txt" manifest)
foreach(key_value ${manifest})
    string(REGEX MATCH "^[^=]+" key ${key_value})
    string(REPLACE "${key}=" "" value ${key_value})
    set(${key} "${value}")
endforeach()

string(TOLOWER ${cross_target} project_name)
project(${project_name} VERSION ${cross_version}.${cross_release})
set(CPACK_PACKAGE_VENDOR ${cross_identifier})

find_package(PkgConfig)

pkg_check_modules(GTKMM gtkmm-3.0)
link_directories(${GTKMM_LIBRARY_DIRS})
include_directories(${GTKMM_INCLUDE_DIRS})

pkg_check_modules(WEBKIT2GTK webkit2gtk-4.0)
link_directories(${WEBKIT2GTK_LIBRARY_DIRS})
include_directories(${WEBKIT2GTK_INCLUDE_DIRS})

add_compile_definitions(PROJECT_NAME="${CMAKE_PROJECT_NAME}")
add_compile_definitions(APPLICATION_ID="${CPACK_PACKAGE_VENDOR}")
file(GLOB sources "./*.cpp")
add_executable("${CMAKE_PROJECT_NAME}" ${sources})
file(GLOB core_sources "../core/src/*.cpp")
target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${core_sources})
file(GLOB main_sources "../../src/*.cpp")
target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${main_sources})
set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY CXX_STANDARD 17)
target_link_libraries("${CMAKE_PROJECT_NAME}"
    ${GTKMM_LIBRARIES} ${WEBKIT2GTK_LIBRARIES} soundio pthread)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${CPACK_PACKAGE_VENDOR})
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgtk-3-0,libwebkit2gtk-4.0-37,libsoundio1")
endif()

include(CPack)

install(TARGETS "${CMAKE_PROJECT_NAME}" DESTINATION "bin")
install(DIRECTORY "../../assets" DESTINATION "share/${CMAKE_PROJECT_NAME}")
