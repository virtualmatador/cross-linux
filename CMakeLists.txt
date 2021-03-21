cmake_minimum_required(VERSION 3.13)
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
set(CPACK_PACKAGE_VENDOR "${cross_identifier}")
set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}")
set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_BINARY_DIR}/icon/app.ico")

find_package(PkgConfig)

pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)
link_directories(${GTKMM_LIBRARY_DIRS})
include_directories(${GTKMM_INCLUDE_DIRS})

pkg_check_modules(WEBKIT2GTK REQUIRED webkit2gtk-4.0)
link_directories(${WEBKIT2GTK_LIBRARY_DIRS})
include_directories(${WEBKIT2GTK_INCLUDE_DIRS})

add_compile_definitions(PROJECT_NAME="${CMAKE_PROJECT_NAME}")
add_compile_definitions(APPLICATION_ID="${CPACK_PACKAGE_VENDOR}")

add_executable("${CMAKE_PROJECT_NAME}")
set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY CXX_STANDARD 17)
file(GLOB sources "./*.cpp")
target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${sources})
file(GLOB core_sources "../core/src/*.cpp")
target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${core_sources})
file(GLOB main_sources "../../src/*.cpp")
target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${main_sources})
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_options("${CMAKE_PROJECT_NAME}" BEFORE PRIVATE
        "-Wl,-subsystem,windows")
    target_link_options("${CMAKE_PROJECT_NAME}" BEFORE PRIVATE
        -static-libstdc++ -static-libgcc)
    target_link_libraries("${CMAKE_PROJECT_NAME}" mingw32 win_resource)
endif()

target_link_libraries("${CMAKE_PROJECT_NAME}"
    ${GTKMM_LIBRARIES} ${WEBKIT2GTK_LIBRARIES} soundio pthread stdc++fs)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${CPACK_PACKAGE_VENDOR})
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgtk-3-0,libwebkit2gtk-4.0-37,libsoundio1")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CPACK_GENERATOR "XCODE")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_VENDOR}\\\\${CMAKE_PROJECT_NAME}")
    set(CPACK_NSIS_PACKAGE_NAME "${CMAKE_PROJECT_NAME}")
endif()

include(CPack)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_custom_target("desktop" ALL DEPENDS "${CMAKE_PROJECT_NAME}.desktop")
    add_custom_command(OUTPUT "${CMAKE_PROJECT_NAME}.desktop"
        COMMAND ${CMAKE_COMMAND} -E echo [Desktop Entry] > "${CMAKE_PROJECT_NAME}.desktop"
        COMMAND ${CMAKE_COMMAND} -E echo Name=${cross_target} >> "${CMAKE_PROJECT_NAME}.desktop"
        COMMAND ${CMAKE_COMMAND} -E echo Exec=${CMAKE_PROJECT_NAME} >> "${CMAKE_PROJECT_NAME}.desktop"
        COMMAND ${CMAKE_COMMAND} -E echo Icon=${CMAKE_PROJECT_NAME} >> "${CMAKE_PROJECT_NAME}.desktop"
        COMMAND ${CMAKE_COMMAND} -E echo Type=Application >> "${CMAKE_PROJECT_NAME}.desktop"
        COMMAND ${CMAKE_COMMAND} -E echo Categories=Utility >> "${CMAKE_PROJECT_NAME}.desktop"
        COMMAND ${CMAKE_COMMAND} -E echo Terminal=false >> "${CMAKE_PROJECT_NAME}.desktop"
    )
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.desktop" DESTINATION "share/applications")
endif()

install(TARGETS "${CMAKE_PROJECT_NAME}" DESTINATION "bin")
install(DIRECTORY "../../assets" DESTINATION "share/${CMAKE_PROJECT_NAME}")

add_subdirectory("icon")
