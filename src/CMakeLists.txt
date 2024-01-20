if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(APP_ICON_MACOSX icon.icns)
endif()

file(GLOB SRCS CONFIGURE_DEPENDS *.cpp)

add_executable(
    ${PROJECT_NAME}
        ${SRCS}
        resources.qrc
        ${APP_ICON_MACOSX}
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
)

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
        common
        libgmsh
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Qt6::OpenGLWidgets
        Qt6::Svg
)

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME "Mesher"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE_COPYRIGHT ${MESHER_COPYRIGHT}
        MACOSX_BUNDLE_ICON_FILE icon.icns
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE TRUE
)

set_source_files_properties(
    icon.icns
    PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
)

add_custom_target(make-icon-iconset-dir ALL COMMAND ${CMAKE_COMMAND} -E make_directory icon.iconset)

add_custom_command(
    OUTPUT icon.iconset/icon_512x512.png
    COMMAND sips -z 512 512 ${CMAKE_SOURCE_DIR}/resources/app-icon.png --out icon.iconset/icon_512x512.png
    DEPENDS
        make-icon-iconset-dir
        ${CMAKE_SOURCE_DIR}/resources/app-icon.png
)

add_custom_command(
    OUTPUT icon.icns
    COMMAND iconutil -c icns icon.iconset
    DEPENDS icon.iconset/icon_512x512.png
)

install(
    TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

configure_file(mesherconfig.h.in mesherconfig.h)