cmake_minimum_required(VERSION 3.5)
project(Smart_CMS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui ${CMAKE_CURRENT_SOURCE_DIR}/src)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${SFML_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(UI_FILES
    ui/loginwindow.ui
    ui/studentwindow.ui
    ui/teacherwindow.ui
    ui/matchingdialog.ui
    ui/scoreinputdialog.ui
    ui/registerdialog.ui
    ui/adminwindow.ui
)

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
    ui/scoreinputdialog.cpp
    ui/registerdialog.cpp
    ui/adminwindow.cpp
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
    ui/scoreinputdialog.h
    ui/registerdialog.h
    ui/adminwindow.h
)

add_executable(Smart_CMS ${SOURCES} ${HEADERS} ${UI_FILES} resources/styles.qrc)

target_link_libraries(Smart_CMS PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    sfml-graphics
    sfml-window
    sfml-system
    OpenSSL::SSL
    OpenSSL::Crypto
)