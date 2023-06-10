cmake_minimum_required(VERSION 3.10)
project(qETRC LANGUAGES CXX)
set(CMAKE_PROJECT_VERSION_MAJOR 1)
set(CMAKE_PROJECT_VERSION_MINOR 3)
set(CMAKE_PROJECT_VERSION_PATCH 5)

# Test versoin on SARibbon  1e76ee5

################# Qt basic ################
list(PREPEND CMAKE_PREFIX_PATH $ENV{QTDIR}/lib/cmake)
message(STATUS "set QTDIR $ENV{QTDIR}")
find_package(Qt5 REQUIRED COMPONENTS Widgets Core Gui)
find_package(Qt5 COMPONENTS PrintSupport)

################# External ################



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
    target_link_libraries(qETRC Qt5::PrintSupport)
else()
    message(WARNING "QtPrintSupport not found")
endif()

################# external libraries ##################
if (NOT ANDROID)
    add_subdirectory(external/Qt-Advanced-Docking-System)

    # disable example building in SARibbon, since some of them requires Qt6 ..
    set(BUILD_EXAMPLES OFF)
    add_subdirectory(external/SARibbon)

    target_link_libraries(qETRC SARibbonBar)
    target_link_libraries(qETRC qtadvanceddocking)
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




