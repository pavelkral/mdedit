cmake_minimum_required(VERSION 3.19)
project(wedit LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS src/forms)

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
    NetworkAuth  
    PrintSupport
)
find_package(Qt6 REQUIRED COMPONENTS Core)

qt_standard_project_setup()

add_subdirectory(cmark)

qt_add_executable(wedit
    #WIN32 MACOSX_BUNDLE
    src/mainwindow.cpp
    src/mainwindow.h
    src/main.cpp  
    src/forms/mainwindow.ui
    resource/wedit.qrc
    resource/wedit.rc
    README.md
    .gitignore LICENSE.txt
    src/qstringconvertor.h
    src/qstringconvertor.cpp
)



target_link_libraries(wedit
    PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::NetworkAuth
    Qt6::PrintSupport
    cmark

)
target_link_libraries(wedit PRIVATE Qt6::Core)

include(GNUInstallDirs)

install(TARGETS wedit
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET wedit
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})


# libs debug
# message(STATUS "Qt6_DIR: ${Qt6_DIR}")
# message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")
# message(STATUS "CMAKE_AUTOMOC: ${CMAKE_AUTOMOC}")
# message(STATUS "CMAKE_AUTORCC: ${CMAKE_AUTORCC}")
# message(STATUS "CMAKE_AUTOUIC: ${CMAKE_AUTOUIC}")

