cmake_minimum_required(VERSION 3.14)

set(qETRC_VERSION_MAJOR 1)
set(qETRC_VERSION_MINOR 3)
set(qETRC_VERSION_PATCH 5)
set(qETRC_RELEASE_CODE 40)

set(qETRC_VERSION "${qETRC_VERSION_MAJOR}.${qETRC_VERSION_MINOR}.${qETRC_VERSION_PATCH}")

message(STATUS "qETRC version ${qETRC_VERSION}")

project(qETRC LANGUAGES CXX VERSION ${qETRC_VERSION})


################# Qt basic ################
list(PREPEND CMAKE_PREFIX_PATH $ENV{QTDIR}/lib/cmake)
message(STATUS "env QTDIR $ENV{QTDIR}")
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} 5.11 COMPONENTS Core Gui Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} 5.11 COMPONENTS PrintSupport)

################## main ##################
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB_RECURSE qETRC_SOURCES "src/*.cpp")

add_executable(qETRC ${qETRC_SOURCES} rsc/resource.qrc)

target_include_directories(qETRC PRIVATE src/)

target_link_libraries(qETRC Qt5::Widgets)

if (Qt5PrintSupport_FOUND)
    message(STATUS "QtPrintSupport is found")
    target_link_libraries(qETRC Qt${QT_VERSION_MAJOR}::PrintSupport)
else()
    message(WARNING "QtPrintSupport not found")
endif()

################# external libraries ##################
include(FetchContent)

if (NOT ANDROID)
    # disable example building in SARibbon, since some of them requires Qt6 ..
    set(BUILD_EXAMPLES OFF)

    add_subdirectory(external/SARibbon)
    
    # list(APPEND CMAKE_PREFIX_PATH "D:/codes/QtProjects/site-packages/Qt-Advanced-Docking-System/out/install/lib/cmake/qt5advanceddocking")
    # find_package(qt${QT_VERSION_MAJOR}advanceddocking CONFIG REQUIRED)

    # try: using FetchContent
    FetchContent_Declare(
        ads
        # GIT_REPOSITORY ${CMAKE_SOURCE_DIR}/external/Qt-Advanced-Docking-System
        GIT_REPOSITORY https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System.git
        GIT_TAG 4.0.3
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/ads
    )
    FetchContent_MakeAvailable(ads)

    target_link_libraries(qETRC SARibbonBar)
    target_link_libraries(qETRC ads::qt${QT_VERSION_MAJOR}advanceddocking)
endif()


#################### miscellaneous #####################

if (ANDROID)
    target_compile_definitions(qETRC PUBLIC QETRC_MOBILE QETRC_MOBILE_2)
else()
    set(RC_ICONS rsc/icons/desktop-icon.ico)
endif()

if (MSVC)
    target_compile_options(qETRC PRIVATE /utf-8 /wd4267 ) 
endif()

install(TARGETS qETRC)
#install(IMPORTED_RUNTIME_ARTIFACTS ads::qt${QT_VERSION_MAJOR}advanceddocking)



