cmake_minimum_required(VERSION 3.19)
project(mdedit LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS src/ui)

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
    NetworkAuth
    PrintSupport
    Xml
)


add_subdirectory(cmark)
add_subdirectory(utils)

qt_add_executable(mdedit
    src/mainwindow.cpp
    src/mainwindow.h
    src/main.cpp
    src/htmlhighlighter.h
    src/htmlhighlighter.cpp
    src/markdownhighlighter.h
    src/markdownhighlighter.cpp
    src/ui/mainwindow.ui
    resource/mdedit.qrc
    resource/mdedit.rc
    src/responsivetextedit.h
    src/responsivetextedit.cpp
)

target_include_directories(mdedit PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${CMAKE_SOURCE_DIR}/utils

)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(mdedit PROPERTIES
        WIN32_EXECUTABLE ON
        MACOSX_BUNDLE ON
    )
endif()

target_link_libraries(mdedit
    PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::NetworkAuth
    Qt6::PrintSupport
    cmark
    utils
)


include(GNUInstallDirs)

install(TARGETS mdedit
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET mdedit
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
# debug
# message(STATUS "Qt6_DIR: ${Qt6_DIR}")
# message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
# message(STATUS "CMAKE_AUTOMOC: ${CMAKE_AUTOMOC}")
# message(STATUS "CMAKE_AUTORCC: ${CMAKE_AUTORCC}")
# message(STATUS "CMAKE_AUTOUIC: ${CMAKE_AUTOUIC}")

