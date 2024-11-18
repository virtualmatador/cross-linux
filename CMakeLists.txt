cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
enable_testing()

include("${CMAKE_CURRENT_SOURCE_DIR}/extern/core/manifest.cmake")

string(TOLOWER ${cross_target} project_name)
project(${project_name} VERSION ${cross_version})
set(CPACK_PACKAGE_VENDOR "${cross_identifier}")
set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}")
set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_BINARY_DIR}/icon/app.ico")

find_package(PkgConfig)

pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)
link_directories(${GTKMM_LIBRARY_DIRS})
include_directories(${GTKMM_INCLUDE_DIRS})

pkg_check_modules(WEBKIT2GTK REQUIRED webkit2gtk-4.1)
link_directories(${WEBKIT2GTK_LIBRARY_DIRS})
include_directories(${WEBKIT2GTK_INCLUDE_DIRS})

add_compile_definitions(PROJECT_NAME="${CMAKE_PROJECT_NAME}")
add_compile_definitions(PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")
add_compile_definitions(APPLICATION_ID="${CPACK_PACKAGE_VENDOR}")

add_executable("${CMAKE_PROJECT_NAME}")
set_target_properties("${CMAKE_PROJECT_NAME}" PROPERTIES
    CXX_STANDARD 17
    RUNTIME_OUTPUT_DIRECTORY "bin"
)
target_include_directories("${CMAKE_PROJECT_NAME}" PRIVATE "extern/core/src")
file(GLOB sources "./*.cpp")
target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${sources})
file(GLOB core_sources "extern/core/src/*.cpp")
target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${core_sources})
file(GLOB main_sources "../../src/*.cpp")
target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${main_sources})

target_link_libraries("${CMAKE_PROJECT_NAME}"
    ${GTKMM_LIBRARIES} ${WEBKIT2GTK_LIBRARIES} pthread stdc++fs)
target_link_options("${CMAKE_PROJECT_NAME}" PRIVATE -no-pie)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${CPACK_PACKAGE_VENDOR})
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgtk-3-0,libwebkit2gtk-4.1-0")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CPACK_GENERATOR "XCODE")
endif()

add_custom_target("assets" ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/share/${CMAKE_PROJECT_NAME}/assets")
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/share/${CMAKE_PROJECT_NAME}/assets"
    COMMAND mkdir -p share/${CMAKE_PROJECT_NAME}
    COMMAND ln -sf ../../../assets share/${CMAKE_PROJECT_NAME}/
)
add_dependencies("${CMAKE_PROJECT_NAME}" "assets")

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

get_filename_component(assets_dir "${CMAKE_CURRENT_BINARY_DIR}/share/${CMAKE_PROJECT_NAME}/assets/" REALPATH)
install(DIRECTORY "${assets_dir}" DESTINATION "share/${CMAKE_PROJECT_NAME}/")

add_subdirectory("icon")
