cmake_minimum_required(VERSION 3.10)
project(qETRC LANGUAGES CXX)

################# Qt basic ################
find_package(Qt5 REQUIRED COMPONENTS Widgets Core Gui)
find_package(Qt5 COMPONENTS PrintSupport)

################# External ################



################## main ##################
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if (MSVC)
    list(APPEND CMAKE_CXX_FLAGS /utf-8)
endif()

file(GLOB_RECURSE qETRC_SOURCES "src/*.cpp")

add_executable(qETRC ${qETRC_SOURCES} rsc/resource.qrc)

target_include_directories(qETRC src/)

target_link_libraries(qETRC Qt5::Widgets)

if (Qt5PrintSupport_FOUND)
    message(STATUS "QtPrintSupport is found")
    target_link_libraries(qETRC Qt5::PrintSupport)
else()
    message(WARNING "QtPrintSupport not found")
endif()

################# external libraries ##################
if (NOT ANDROID)
    add_subdirectory(external/Qt-Advanced-Docking-System)
    add_subdirectory(external/SARibbon)

    target_link_libraries(qETRC PRIVATE SARibbonBar)
    target_link_libraries(qETRC PRIVATE qtadvanceddocking)
endif()


#################### miscellaneous #####################

if (ANDROID)
    target_compile_definitions(qETRC PUBLIC QETRC_MOBILE QETRC_MOBILE_2)
else()
    set(RC_ICONS rsc/icons/desktop-icon.ico)
endif()






