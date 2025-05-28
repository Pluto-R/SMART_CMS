cmake_minimum_required(VERSION 3.5)
project(Smart_CMS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${SFML_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

file(GLOB UI_FILES "ui/*.ui")

set(SOURCES
    src/main.cpp
    src/QSFMLCanvas.cpp
    src/analysis.cpp
    src/matching.cpp
    src/user_manage.cpp
    ui/loginwindow.cpp
    ui/studentwindow.cpp
    ui/teacherwindow.cpp
    ui/matchingdialog.cpp
    ui/scoreanalysisdialog.cpp
    ui/scoreinputdialog.cpp
)

set(HEADERS
    include/QSFMLCanvas.hpp
    include/analysis.hpp
    include/config.hpp
    include/matching.hpp
    include/user_manage.hpp
    ui/loginwindow.h
    ui/studentwindow.h
    ui/teacherwindow.h
    ui/matchingdialog.h
    ui/scoreanalysisdialog.h
    ui/scoreinputdialog.h
)

add_executable(Smart_CMS ${SOURCES} ${HEADERS} ${UI_FILES} resources/styles.qrc)

target_link_libraries(Smart_CMS 
    Qt5::Widgets
    sfml-graphics
    sfml-window
    sfml-system
    OpenSSL::SSL
    OpenSSL::Crypto
)